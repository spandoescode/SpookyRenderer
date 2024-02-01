/*
Author: Spandan More
Class: ECE6122
Last Date Modified: 10/28/2023
Description:

Homework 3

Main file for the OpenGL application which includes four Suzannes and a rectangle in the z=0 plane.
The user can move the camera using WASD and the up,down arrow keys. You can also toggle diffuse and specular
lighting components using the L key.
*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

// Boolean to control the specular and diffuse lighting components
bool isSpecular = true;

// Callback function for handling keyboard input
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		isSpecular = !isSpecular; // Toggle the applySpecular variable when "L" key is pressed
	}
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Set the key callback function for lighting toggle
	glfwSetKeyCallback(window, keyCallback);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	// glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Load the texture
	GLuint Texture = loadDDS("uvmap.DDS");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Vertex data for the rectangle
	GLfloat recVertices[] = {
		-3.5f, -3.5f, 0.0f, // Bottom-left
		3.5f, -3.5f, 0.0f,	// Bottom-right
		3.5f, 3.5f, 0.0f,	// Top-right -- Traingle 1
		-3.5f, 3.5f, 0.0f,	// Top-left
							// 3.5f, 3.5f, 0.0f,	// Top-right
							// -3.5f, -3.5f, 0.0f, // Bottom-left -- Traingle 2
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
		0.0f, 1.0f	// Top-left
	};

	// Index data for the rectangle
	GLuint recIndices[] = {
		0, 1, 2, // First Triangle
		0, 2, 3	 // Second Triangle
	};

	GLuint recVAO;
	glGenVertexArrays(1, &recVAO);
	glBindVertexArray(recVAO);

	// Load the rectangle into a VBO
	GLuint recVBO;

	glGenBuffers(1, &recVBO);
	glBindBuffer(GL_ARRAY_BUFFER, recVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(recVertices), recVertices, GL_STATIC_DRAW);

	GLuint recNBO;

	glGenBuffers(1, &recNBO);
	glBindBuffer(GL_ARRAY_BUFFER, recNBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(recNormals), recNormals, GL_STATIC_DRAW);

	GLuint recUVBO;

	glGenBuffers(1, &recUVBO);
	glBindBuffer(GL_ARRAY_BUFFER, recUVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(recUV), recUV, GL_STATIC_DRAW);

	GLuint recEBO;

	glGenBuffers(1, &recEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, recEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(recIndices), recIndices, GL_STATIC_DRAW);

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	// glm::mat4 ViewMatrix;

	// ViewMatrix = glm::lookAt(
	// 	glm::vec3(0.0f, 0.0f, 10.0f), // Camera is here
	// 	glm::vec3(0.0f, 0.0f, 0.0f),  // and looks here : at the same position, plus "direction"
	// 	glm::vec3(0.0f, 1.0f, 0.0f)	  // Head is up (set to 0,-1,0 to look upside-down)
	// );

	do
	{

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0)
		{ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set the boolean uniform in the fragment shader
		GLint toggleLocation = glGetUniformLocation(programID, "isSpecular");
		glUniform1i(toggleLocation, isSpecular);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		////// Start of the rendering of the first object //////

		// Use our shader
		glUseProgram(programID);

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		////// Start of rendering of the Rectangle //////

		glm::mat4 ModelMatrix0 = glm::mat4(1.0);
		ModelMatrix0 = glm::translate(ModelMatrix0, glm::vec3(0.0f, 0.0f, -1.0f));
		glm::mat4 MVP0 = ProjectionMatrix * ViewMatrix * ModelMatrix0;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP0[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix0[0][0]);

		glBindVertexArray(recVAO);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, recVBO);
		glVertexAttribPointer(
			0,		  // attribute
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void *)0 // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, recUVBO);
		glVertexAttribPointer(
			1,		  // attribute
			2,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void *)0 // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, recNBO);
		glVertexAttribPointer(
			2,		  // attribute
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void *)0 // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, recEBO);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,	 // mode
			6,				 // count
			GL_UNSIGNED_INT, // type
			0				 // element array buffer offset
		);

		////// End of rendering of the Rectangle //////

		glm::mat4 ModelMatrix1 = glm::mat4(1.0);
		ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(0.0f, -2.0f, 0.0f));
		ModelMatrix1 = glm::rotate(ModelMatrix1, glm::radians(90.0f), glm::vec3(1, 0, 0));
		glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void *)0 // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,		  // attribute
			2,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void *)0 // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,		  // attribute
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void *)0 // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,	   // mode
			indices.size(),	   // count
			GL_UNSIGNED_SHORT, // type
			(void *)0		   // element array buffer offset
		);

		////// End of rendering of the first object //////
		////// Start of the rendering of the second object //////

		// In our very specific case, the 2 objects use the same shader.
		// So it's useless to re-bind the "programID" shader, since it's already the current one.
		// glUseProgram(programID);

		// Similarly : don't re-set the light position and camera matrix in programID,
		// it's still valid !
		// *** You would have to do it if you used another shader ! ***
		// glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		// glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		// Again : this is already done, but this only works because we use the same shader.
		//// Bind our texture in Texture Unit 0
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, Texture);
		//// Set our "myTextureSampler" sampler to use Texture Unit 0
		// glUniform1i(TextureID, 0);

		// BUT the Model matrix is different (and the MVP too)
		glm::mat4 ModelMatrix2 = glm::mat4(1.0);
		ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(0.0f, 2.0f, 0.0f));
		ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(90.0f), glm::vec3(1, 0, 0));
		ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(180.0f), glm::vec3(0, 1, 0));
		glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);

		// The rest is exactly the same as the first object

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void *)0);

		////// End of rendering of the second object //////

		////// Start of rendering of the third object //////

		// BUT the Model matrix is different (and the MVP too)
		glm::mat4 ModelMatrix3 = glm::mat4(1.0);
		ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(2.0f, 0.0f, 0.0f));
		ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(90.0f), glm::vec3(1, 0, 0));
		ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(90.0f), glm::vec3(0, 1, 0));
		glm::mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrix3;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix3[0][0]);

		// The rest is exactly the same as the first object

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void *)0);

		////// End of rendering of the third object //////

		////// Start of rendering of the fourth object //////

		// BUT the Model matrix is different (and the MVP too)
		glm::mat4 ModelMatrix4 = glm::mat4(1.0);
		ModelMatrix4 = glm::translate(ModelMatrix4, glm::vec3(-2.0f, 0.0f, 0.0f));
		ModelMatrix4 = glm::rotate(ModelMatrix4, glm::radians(90.0f), glm::vec3(1, 0, 0));
		ModelMatrix4 = glm::rotate(ModelMatrix4, glm::radians(270.0f), glm::vec3(0, 1, 0));
		glm::mat4 MVP4 = ProjectionMatrix * ViewMatrix * ModelMatrix4;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP4[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix4[0][0]);

		// The rest is exactly the same as the first object

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void *)0);

		////// End of rendering of the fourth object //////

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}