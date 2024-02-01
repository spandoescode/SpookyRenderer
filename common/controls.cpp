/*
Author: Spandan More
Class: ECE6122
Last Date Modified: 10/28/2023
Description:

Homework 3

Code file that handles user keyboard inputs for the OpenGL application
*/

// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow *window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix()
{
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix()
{
	return ProjectionMatrix;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3(0, 0, 5);

// Initial distance of the camera from the origin
float radius = 20.0f;
// Initial horizontal angle : toward -Z
float phi = glm::radians(90.0f);
// Initial vertical angle : none
float theta = glm::radians(0.0f);
// Initial Field of View
float initialFoV = 45.0f;

void computeMatricesFromInputs()
{
	// Move towards the origin
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		radius -= 0.05f;
	}
	// Move away from the origin
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		radius += 0.05f;
	}
	// Rotate Right
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		phi += glm::radians(0.5f);
	}
	// Rotate Left
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		phi -= glm::radians(0.5f);
	}
	// Rotate Up
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		theta -= glm::radians(0.5f);
	}
	// Rotate Down
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		theta += glm::radians(0.5f);
	}

	// Limit the camera radius to avoid going too close or too far
	radius = glm::clamp(radius, 0.0f, 30.0f);
	theta = glm::clamp(theta, 0.1f, 3.13f);
	// phi = glm::clamp(phi, 0.1f, 6.27f);

	position.x = radius * sin(theta) * cos(phi);
	position.y = radius * sin(theta) * sin(phi);
	position.z = radius * cos(theta);

	float FoV = initialFoV; // - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,					 // Camera is here
		glm::vec3(0.0f, 0.0f, 0.0f), // and looks here : at the same position, plus "direction"
		glm::vec3(0.0f, 0.0f, 1.0f)	 // Head is up (set to 0,-1,0 to look upside-down)
	);
}