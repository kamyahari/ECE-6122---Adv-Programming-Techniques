// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
    return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
    return ProjectionMatrix;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3(0, 0, 5);

float radius = 50.0f;
// Initial horizontal angle : toward -Z
float phi = 0.0f;
// Initial vertical angle : none
float theta = glm::radians(85.0f);
// Initial Field of View
float initialFoV = 45.0f;

float delta_theta = 0.01f;
float delta_phi = 0.01f;


void computeMatricesFromInputs() {

    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        radius -= .001f;
        if (radius <= 0.0f)
            radius = .001f;

    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        radius += .001f;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        phi += glm::radians(.010f);
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        phi -= glm::radians(0.010f);
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        theta -= glm::radians(.010f);
        if (theta > 90.0f)
            theta = 90.0f;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        theta += glm::radians(delta_theta);
        if (theta < delta_theta)
            theta = delta_theta;
    }

    position.x = radius * sin(theta) * cos(phi);
    position.y = radius * sin(theta) * sin(phi);
    position.z = radius * cos(theta);

    float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    ViewMatrix = glm::lookAt(
        position,           // Camera is here
        glm::vec3(0, 0, 0), // and looks here : at the same position, plus "direction"
        glm::vec3(0, 0, 1)  // Head is up (set to 0, 0, 1 to look upside-down)
    );
}