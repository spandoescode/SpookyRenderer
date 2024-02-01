/*
Author: Spandan More
Class: ECE6122 A
Last Date Modified: 12/09/2023
Description:

A spooky scene with random motion and collision of 3D objects written in Open GL
*/

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <tuple>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

GLFWwindow *window;
GLuint programID;
GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint Texture, TextureID;
GLuint VertexArrayID;
bool isMotion = false;
bool threadExit = false;

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        isMotion = !isMotion;
    }
}

GLuint loadTexture(const char *filepath)
{
    // Load the floor texture
    GLuint floor = loadBMP_custom(filepath);

    return floor;
}

// Create a custom class to represent a 3D object in the scene
class object3D
{
    // Variables to store the coordinates of the object
    double x, y, z;
    double rotX, rotY, rotZ;

    // Arrays to store the object rendering data
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;

    GLuint vertexbuffer, uvbuffer, normalbuffer, elementbuffer;

public:
    double xMovementSpeed, yMovementSpeed, zMovementSpeed;
    double xRotationSpeed, yRotationSpeed, zRotationSpeed;
    float lightIntensity;

    void setPosition(double _x, double _y, double _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    void setRotation(double _x, double _y, double _z)
    {
        rotX = _x;
        rotY = _y;
        rotZ = _z;
    }

    std::tuple<double, double, double> getPosition()
    {
        return std::make_tuple(x, y, z);
    }

    std::tuple<double, double, double> getRotation()
    {
        return std::make_tuple(rotX, rotY, rotZ);
    }

    void setupVertexArrayObject()
    {
        // Load the .obj file
        bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);
        indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

        glGenBuffers(1, &normalbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

        glGenBuffers(1, &elementbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
    }

    void drawObject(glm::mat4 &ModelMatrix, glm::mat4 &MVP)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);

        GLuint LightID = glGetUniformLocation(programID, "lightValue");
        glUniform1f(LightID, lightIntensity);

        glm::vec3 lightPos = glm::vec3(x, y, z);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,        // attribute
            3,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
        );

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            1,        // attribute
            2,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
        );

        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
            2,        // attribute
            3,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
        );

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles !
        glDrawElements(
            GL_TRIANGLES,      // mode
            indices.size(),    // count
            GL_UNSIGNED_SHORT, // type
            (void *)0          // element array buffer offset
        );

        glBindTexture(GL_TEXTURE_2D, Texture);
    }

    void cleanup()
    {
        glDeleteBuffers(1, &vertexbuffer);
        glDeleteBuffers(1, &uvbuffer);
        glDeleteBuffers(1, &normalbuffer);
        glDeleteBuffers(1, &elementbuffer);
    }
};

class staticObject
{
    // GLfloat *vertices, *normals, *uv, *indices;
    GLuint VAO, VBO, NBO, UVBO, EBO;
    char *textureFile;
    GLuint text;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;

public:
    void loadObject(std::string type)
    {
        if (type == "floor")
        {
            std::string temp = "floor.bmp";

            char *charArray = new char[temp.length() + 1]; // +1 for the null terminator
            std::copy(temp.begin(), temp.end(), charArray);
            charArray[temp.length()] = '\0'; // Add null terminator

            textureFile = charArray;

            // Load the floor texture
            text = loadTexture(textureFile);

            // Vertex data for the rectangle
            GLfloat recVertices[] = {
                -30.0f, -30.0f, 0.0f, // Bottom-left
                30.0f, -30.0f, 0.0f,  // Bottom-right
                30.0f, 30.0f, 0.0f,   // Top-right -- Traingle 1
                -30.0f, 30.0f, 0.0f,  // Top-left
            };

            // Normal data for the rectangle
            GLfloat recNormals[] = {
                0.0f,
                0.0f,
                0.1f,
            };

            // UV map for the rectangle
            GLfloat recUV[] = {
                // UV coordinates (u, v)
                0.0f, 0.0f, // Bottom-left
                1.0f, 0.0f, // Bottom-right
                1.0f, 1.0f, // Top-right
                0.0f, 1.0f  // Top-left
            };

            // Index data for the rectangle
            GLuint recIndices[] = {
                0, 1, 2, // First Triangle
                0, 2, 3  // Second Triangle
            };

            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(recVertices), recVertices, GL_STATIC_DRAW);

            glGenBuffers(1, &NBO);
            glBindBuffer(GL_ARRAY_BUFFER, NBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(recNormals), recNormals, GL_STATIC_DRAW);

            glGenBuffers(1, &UVBO);
            glBindBuffer(GL_ARRAY_BUFFER, UVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(recUV), recUV, GL_STATIC_DRAW);

            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(recIndices), recIndices, GL_STATIC_DRAW);
        }
        else if (type == "back")
        {
            std::string temp = "back.bmp";

            char *charArray = new char[temp.length() + 1]; // +1 for the null terminator
            std::copy(temp.begin(), temp.end(), charArray);
            charArray[temp.length()] = '\0'; // Add null terminator

            textureFile = charArray;

            // Load the floor texture
            text = loadTexture(textureFile);

            // Vertex data for the wall
            GLfloat recVertices[] = {
                -30.0f, 30.0f, 30.0f, // Up Left
                30.0f, 30.0f, 30.0f,  // Up Right
                30.0f, 30.0f, 0.0f,   // Top-right
                -30.0f, 30.0f, 0.0f,  // Top-left
            };

            // Normal data for the rectangle
            GLfloat recNormals[] = {
                0.0f,
                0.1f,
                0.0f,
            };

            // UV map for the rectangle
            GLfloat recUV[] = {
                // UV coordinates (u, v)
                0.0f, 1.0f, // Bottom-left
                1.0f, 1.0f, // Bottom-right
                1.0f, 0.0f, // Top-right
                0.0f, 0.0f  // Top-left
            };

            // Index data for the rectangle
            GLuint recIndices[] = {
                0, 1, 2, // First Triangle
                0, 2, 3  // Second Triangle
            };

            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(recVertices), recVertices, GL_STATIC_DRAW);

            glGenBuffers(1, &NBO);
            glBindBuffer(GL_ARRAY_BUFFER, NBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(recNormals), recNormals, GL_STATIC_DRAW);

            glGenBuffers(1, &UVBO);
            glBindBuffer(GL_ARRAY_BUFFER, UVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(recUV), recUV, GL_STATIC_DRAW);

            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(recIndices), recIndices, GL_STATIC_DRAW);
        }
        else if (type == "skel")
        {
            std::string temp = "black.bmp";

            char *charArray = new char[temp.length() + 1]; // +1 for the null terminator
            std::copy(temp.begin(), temp.end(), charArray);
            charArray[temp.length()] = '\0'; // Add null terminator

            textureFile = charArray;

            // Load the floor texture
            text = loadTexture(textureFile);
            // text = loadDDS("uvmap.DDS");

            bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);
            indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

            glGenBuffers(1, &UVBO);
            glBindBuffer(GL_ARRAY_BUFFER, UVBO);
            glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

            glGenBuffers(1, &NBO);
            glBindBuffer(GL_ARRAY_BUFFER, NBO);
            glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
        }
    }

    void drawObject(glm::mat4 &ModelMatrix, glm::mat4 &MVP, int type)
    {
        if (type == 1)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, text);
            // Set our "myTextureSampler" sampler to use Texture Unit 0
            glUniform1i(TextureID, 0);

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

            // 1rst attribute buffer : vertices
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glVertexAttribPointer(
                0,        // attribute
                3,        // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0,        // stride
                (void *)0 // array buffer offset
            );

            // 2nd attribute buffer : UVs
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, UVBO);
            glVertexAttribPointer(
                1,        // attribute
                2,        // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0,        // stride
                (void *)0 // array buffer offset
            );

            // 3rd attribute buffer : normals
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, NBO);
            glVertexAttribPointer(
                2,        // attribute
                3,        // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0,        // stride
                (void *)0 // array buffer offset
            );

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            // Draw the triangles !
            glDrawElements(
                GL_TRIANGLES,    // mode
                6,               // count
                GL_UNSIGNED_INT, // type
                0                // element array buffer offset
            );

            glBindTexture(GL_TEXTURE_2D, Texture);
        }
        else
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, text);
            // Set our "myTextureSampler" sampler to use Texture Unit 0
            glUniform1i(TextureID, 0);

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

            // 1rst attribute buffer : vertices
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glVertexAttribPointer(
                0,        // attribute
                3,        // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0,        // stride
                (void *)0 // array buffer offset
            );

            // 2nd attribute buffer : UVs
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, UVBO);
            glVertexAttribPointer(
                1,        // attribute
                2,        // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0,        // stride
                (void *)0 // array buffer offset
            );

            // 3rd attribute buffer : normals
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, NBO);
            glVertexAttribPointer(
                2,        // attribute
                3,        // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0,        // stride
                (void *)0 // array buffer offset
            );

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            // Draw the triangles !
            glDrawElements(
                GL_TRIANGLES,      // mode
                indices.size(),    // count
                GL_UNSIGNED_SHORT, // type
                (void *)0          // element array buffer offset
            );

            glBindTexture(GL_TEXTURE_2D, Texture);
        }
    }
};

// Vectors to store the objects in the scene
std::vector<object3D> objects(4);
std::vector<staticObject> statics;

// Thread function to calculate the new positon of the objects
void randomMotion()
{
    // Set the motion parameters of all the objects
    for (int i = 0; i < objects.size(); i++)
    {
        objects[i].xMovementSpeed = ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) - 0.5) / 3;
        objects[i].yMovementSpeed = ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) - 0.5) / 3;
        objects[i].zMovementSpeed = ((static_cast<double>(rand()) / static_cast<double>(2.0 * RAND_MAX))) / 3;

        objects[i].xRotationSpeed = (static_cast<double>((rand()) % 30) - 0.5) / 5;
        objects[i].yRotationSpeed = (static_cast<double>((rand()) % 30) - 0.5) / 5;
        objects[i].zRotationSpeed = (static_cast<double>((rand()) % 30) - 0.5) / 5;

        objects[i].lightIntensity = 0.5f;
    }

    // Set the collision radius between the objects
    double collisionRadius = 1.0;

    while (!threadExit)
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Add your logic for updating object motion here
        if (isMotion)
        {
            for (int i = 0; i < objects.size(); i++)
            {
                // Set the random intensity of light
                float lightChange = ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) - 0.5f);

                // Make sure the object is not too dim
                if (objects[i].lightIntensity + lightChange < 0.3 || objects[i].lightIntensity + lightChange > 1.3)
                {
                    lightChange *= -1;
                }
                objects[i].lightIntensity += lightChange;

                // Fetch the current values
                double x, y, z, rx, ry, rz;
                std::tie(x, y, z) = objects[i].getPosition();
                std::tie(rx, ry, rz) = objects[i].getRotation();

                if (x < -10.0 || x > 10.0)
                {
                    objects[i].xMovementSpeed *= -1;
                }

                if (y < -10.0 || y > 10.0)
                {
                    objects[i].yMovementSpeed *= -1;
                }

                if (z < -8.0 || z > 8.0)
                {
                    objects[i].zMovementSpeed *= -1;
                }

                for (int j = i + 1; j < objects.size(); j++)
                {
                    // Fetch the current values
                    double _x, _y, _z;
                    std::tie(_x, _y, _z) = objects[j].getPosition();

                    // Calculate the distance between two objects
                    double dist = std::sqrt(
                        std::pow(x - _x, 2) +
                        std::pow(y - _y, 2) +
                        std::pow(z - _z, 2));

                    // Check for collision
                    if (dist < collisionRadius)
                    {
                        // Objects i and j collided, reverse their velocities along the collision axis
                        objects[i].xMovementSpeed *= -1;
                        objects[i].yMovementSpeed *= -1;
                        objects[i].zMovementSpeed *= -1;

                        objects[j].xMovementSpeed *= -1;
                        objects[j].yMovementSpeed *= -1;
                        objects[j].zMovementSpeed *= -1;
                    }
                }

                // Calculate the new values
                x += objects[i].xMovementSpeed;
                y += objects[i].yMovementSpeed;
                z += objects[i].zMovementSpeed;

                // Keep the rotation a valid degree value
                if (rx + objects[i].xRotationSpeed < -360.0 || rx + objects[i].xRotationSpeed > 360.0)
                {
                    objects[i].xRotationSpeed *= -1;
                }

                if (ry + objects[i].yRotationSpeed < -360.0 || ry + objects[i].yRotationSpeed > 360.0)
                {
                    objects[i].yRotationSpeed *= -1;
                }

                if (rz + objects[i].zRotationSpeed < -360.0 || rz + objects[i].zRotationSpeed > 360.0)
                {
                    objects[i].zRotationSpeed *= -1;
                }

                rx += objects[i].xRotationSpeed;
                ry += objects[i].yRotationSpeed;
                rz += objects[i].zRotationSpeed;

                // Set the new values
                objects[i].setPosition(x, y, z);
                objects[i].setRotation(rx, ry, rz);
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        // Adjust the sleep duration based on elapsed time to achieve a consistent frame rate
        if (elapsedTime < std::chrono::milliseconds(16))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(16) - elapsedTime);
        }
    }
}

void initializeGLFW()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        exit(-1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void createWindowAndContext()
{
    window = glfwCreateWindow(1024, 768, "Final Project - Spooky Scene", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        exit(-1);
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);
}

void setupOpenGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void setupShadersAndProgram()
{
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
    MatrixID = glGetUniformLocation(programID, "MVP");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ModelMatrixID = glGetUniformLocation(programID, "M");

    Texture = loadDDS("uvmap.DDS");
    TextureID = glGetUniformLocation(programID, "myTextureSampler");
}

void renderLoop()
{
    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    do
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // GLint toggleLocation = glGetUniformLocation(programID, "isSpecular");
        // glUniform1i(toggleLocation, isSpecular);

        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();

        // Use our shader
        glUseProgram(programID);

        glm::vec3 lightPos = glm::vec3(0, 0, 4);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        GLuint LightID1 = glGetUniformLocation(programID, "lightValue");
        glUniform1f(LightID1, 5.0);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);

        // Render the floor and other static objects
        glm::mat4 ModelMatrixF = glm::mat4(1.0);
        ModelMatrixF = glm::translate(ModelMatrixF, glm::vec3(0.0f, 0.0f, -10.0f));
        glm::mat4 MVPF = ProjectionMatrix * ViewMatrix * ModelMatrixF;
        statics[0].drawObject(ModelMatrixF, MVPF, 1);

        glm::mat4 ModelMatrixB = glm::mat4(1.0);
        ModelMatrixB = glm::translate(ModelMatrixB, glm::vec3(0.0f, 0.0f, -10.0f));
        glm::mat4 MVPB = ProjectionMatrix * ViewMatrix * ModelMatrixB;
        statics[1].drawObject(ModelMatrixB, MVPB, 1);

        glm::mat4 ModelMatrixSK1 = glm::mat4(1.0);
        ModelMatrixSK1 = glm::translate(ModelMatrixSK1, glm::vec3(5.0f, 5.0f, -8.0f));
        glm::mat4 MVPSK1 = ProjectionMatrix * ViewMatrix * ModelMatrixSK1;
        statics[2].drawObject(ModelMatrixSK1, MVPSK1, 0);

        glm::mat4 ModelMatrixSK2 = glm::mat4(1.0);
        ModelMatrixSK2 = glm::translate(ModelMatrixSK2, glm::vec3(-5.0f, -3.0f, -8.0f));
        glm::mat4 MVPSK2 = ProjectionMatrix * ViewMatrix * ModelMatrixSK2;
        statics[3].drawObject(ModelMatrixSK2, MVPSK2, 0);

        // Draw all the moving objects
        for (int i = 0; i < objects.size(); i++)
        {
            // Draw the first object
            glm::mat4 ModelMatrix0 = glm::mat4(1.0);

            // Get the new position of the object
            double xNew, yNew, zNew;
            float rxNew, ryNew, rzNew;
            std::tie(xNew, yNew, zNew) = objects[i].getPosition();
            std::tie(rxNew, ryNew, rzNew) = objects[i].getRotation();

            // Set the new positons and rotations in the model matrices
            ModelMatrix0 = glm::translate(ModelMatrix0, glm::vec3(xNew, yNew, zNew));
            ModelMatrix0 = glm::rotate(ModelMatrix0, glm::radians(rxNew), glm::vec3(1, 0, 0));
            ModelMatrix0 = glm::rotate(ModelMatrix0, glm::radians(ryNew), glm::vec3(0, 1, 0));
            ModelMatrix0 = glm::rotate(ModelMatrix0, glm::radians(rzNew), glm::vec3(0, 0, 1));
            glm::mat4 MVP0 = ProjectionMatrix * ViewMatrix * ModelMatrix0;
            objects[i].drawObject(ModelMatrix0, MVP0);
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);
}

int main(void)
{
    // Seed the random number generator

    initializeGLFW();
    createWindowAndContext();
    setupOpenGL();
    setupShadersAndProgram();

    // Create the static objects
    staticObject floor, back, skel1, skel2;
    floor.loadObject("floor");
    statics.push_back(floor);
    back.loadObject("back");
    statics.push_back(back);
    skel1.loadObject("skel");
    statics.push_back(skel1);
    skel2.loadObject("skel");
    statics.push_back(skel2);

    // Create the four moving objects and initialise the models
    for (int i = 0; i < objects.size(); i++)
    {
        objects[i].setupVertexArrayObject();
    }

    // Set the initial positions of the objects
    objects[0].setPosition(0.0f, -2.0f, 0.0f);
    objects[1].setPosition(0.0f, 2.0f, 0.0f);
    objects[2].setPosition(2.0f, 0.0f, 0.0f);
    objects[3].setPosition(-2.0f, 0.0f, 0.0f);

    objects[0].setRotation(90.0f, 0.0f, 0.0f);
    objects[1].setRotation(90.0f, 180.0f, 0.0f);
    objects[2].setRotation(90.0f, 90.0f, 0.0f);
    objects[3].setRotation(90.0f, 270.0f, 0.0f);

    // Start the random motion thread
    std::thread motionThread(randomMotion);

    // Main Rendering Loop
    renderLoop();

    // Exit the thread
    threadExit = true;
    motionThread.join();

    // For all the objects
    for (int i = 0; i < objects.size(); i++)
    {
        objects[i].cleanup();
    }

    // Once the program has exited cleanup the buffers
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
