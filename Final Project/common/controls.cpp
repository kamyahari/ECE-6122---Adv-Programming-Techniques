/*
Author: Kamya Hari
Class: ECE 6122 A
Last date Modified: 12/04/2024


Objective:
Added additional camera/light/light power control methods - updates according to the user input command. 

*/

// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

#include <sstream>
#include <string>
// Add to controls.cpp
#include <thread>
#include <atomic>

std::atomic<bool> shouldExit(false);
//glm::vec3 lightPos(0, 0, 15);

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;
bool toggleSDColor=true;
int dbnceCnt = 20;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

bool getLightSwitch()
{
	return toggleSDColor;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 0, 5);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;

// Lab3 key movement coordinates (spherical)
float cRadius = 50.0f;
float cPhi = -90.0f;
float cTheta = 1.0f;

// Speed
float speedLab3 = 10.0f; // 3 units / second

// Debounce limit
const int DEB_LIMIT = 40;


void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024/2 - xpos );
	verticalAngle   += mouseSpeed * float( 768/2 - ypos );

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

// A custom function for Lab3
// Creates the view and Projection matrix based on following custom key definitions
// keyboard inputs definitions
//1) ‘w’ key moves the camera radially closer to the origin.
//2) ‘s’ key moves the camera radially farther from the origin.
//3) ‘a’ key rotates the camera to the left maintaining the radial distance from the origin.
//4) ‘d’ key rotates to camera to the right maintaining the radial distance from the origin.
//5) The up arrow key radially rotates the camera up.
//6) The down arrow radially rotates the camera down.
//7) The ‘L’ key toggles the specular and diffuse components of the light on and off but leaves the ambient component unchanged.
//8) Pressing the escape key closes the window and exits the program

void computeMatricesFromInputsLab3()
{
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Keep rolling debounce count
	// if not at Debounce limit
	if (dbnceCnt <= DEB_LIMIT)
	{
		dbnceCnt++;
	}

	// Create origin
	glm::vec3 origin = glm::vec3(0, 0, 0);
	// Up vector (look in the z direction)
	glm::vec3 up = glm::vec3(0, 0, 1);

	// ‘w’ key moves the camera radially closer to the origin
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
	{
		// Last cRadius to avoids camera jerks
		// at maximum zoom (Stas stable)
		float lastcRadius = cRadius;
		cRadius -= deltaTime * speedLab3;
		if (cRadius < 0)
		{ // Avoid zooming into -ve zone
			cRadius = lastcRadius;
		}
	}
	// ‘s’ key moves the camera radially farther from the origin.
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
	{
		cRadius += deltaTime * speedLab3;
	}
	// ‘a’ key rotates the camera to the left maintaining the radial distance from the origin.
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
	{
		cPhi += deltaTime * speedLab3;
		// Check for wrap around condition
		if (cPhi > 360.0f)
		{ // Wrap around to zero
			cPhi = cPhi - 360.f;
		}
	}
	// ‘d’ key rotates to camera to the right maintaining the radial distance from the origin
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) 
	{
		cPhi -= deltaTime * speedLab3;
		// Check for wrap around condition
		if (cPhi < 0.f)
		{ // Wrap around to zero
			cPhi = 360.f + cPhi;
		}
	}
	// The up arrow key radially rotates the camera up
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) 
	{
		cTheta += deltaTime * speedLab3;
		// Check for wrap around condition
		if (cTheta > 360.0f)
		{ // Wrap around to zero
			cTheta = cTheta - 360.f;
		}
	}
	// The down arrow key radially rotates the camera down
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) 
	{
		cTheta -= deltaTime * speedLab3;
		// Check for wrap around condition
		if (cTheta < 0.0f)
		{ // Wrap around to zero
			cTheta = 360.f + cTheta;
		}
	}

	// The ‘L’ key toggles the specular and diffuse components of the light on and off 
	// but leaves the ambient component unchanged.
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		if (dbnceCnt >= DEB_LIMIT)
		{ // Toggle light switch and restart the Debounce count
		    toggleSDColor = !toggleSDColor;
			dbnceCnt = 0;
		}
	}

	// Convert to Cartesian co-ordinate system
	float posX = cRadius * sin(glm::radians(cTheta)) * cos(glm::radians(cPhi));
	float posY = cRadius * sin(glm::radians(cTheta)) * sin(glm::radians(cPhi));
	float posZ = cRadius * cos(glm::radians(cTheta));

	// Create new camera position
	glm::vec3 position = glm::vec3(posX, posY, posZ);
	float FoV = initialFoV; // - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);

	// Adjust the UP (ESSENTIAL for Continuus vertical Rotation WITHOUT a FLIP)!
	if (cTheta > 180.0f)
	{ // Flip the UP (-Z)
		up = glm::vec3(0, 0, -1);
	}

	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		origin,             // and looks here : at the same position, plus "direction"
		up                  // Look in the z-direction (set to 0,0,1 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
//glm::vec3 lightPos = glm::vec3(0, 0, 15);  // Initial light position

glm::vec3 lightPos(0, 0, 15);
float lightPower = 400.0f;

bool validateSphericalCoords(float theta, float phi, float radius) {
	return theta >= 10.0f && theta <= 80.0f &&
		phi >= 0.0f && phi <= 360.0f &&
		radius > 0.0f;
}

/*void processCommand(const std::string& input) {
	std::stringstream ss(input);
	std::string cmd;
	ss >> cmd;

	if (cmd == "camera" || cmd == "light") {
		float theta, phi, radius;
		if (ss >> theta >> phi >> radius && validateSphericalCoords(theta, phi, radius)) {
			if (cmd == "camera") {
				cTheta = cTheta + theta;
				cPhi = cPhi + phi;
				cRadius = cRadius + radius;
			}
			else {
				float x = radius * sin(glm::radians(theta)) * cos(glm::radians(phi));
				float y = radius * sin(glm::radians(theta)) * sin(glm::radians(phi));
				float z = radius * cos(glm::radians(theta));
				lightPos = glm::vec3(x, y, z);
			}
		}
		else {
			std::cout << "Invalid command" << std::endl;
		}
	}
	else if (cmd == "power") {
		float power;
		if (ss >> power && power > 0.0f) {
			lightPower = power;
			std::cout << "Light power updated to: " << lightPower << std::endl;  // Debug output
		}
		else {
			std::cout << "Invalid power value" << std::endl;
		}
	}
	else {
		std::cout << "Invalid command" << std::endl;
	}
}*/
void processCommand(const std::string& input) {
	std::stringstream ss(input);
	std::string cmd;
	ss >> cmd;

	if (cmd == "camera" || cmd == "light") {
		float theta, phi, radius;
		std::cout << "\nEnter values for theta (10-80), phi (0-360), and radius (>0): ";
		if (ss >> theta >> phi >> radius && validateSphericalCoords(theta, phi, radius)) {
			if (cmd == "camera") {
				cTheta = cTheta+theta;
				cPhi = cPhi+phi;
				cRadius = cRadius+radius;
				std::cout << "Camera position updated successfully.\n";
			}
			else {
				float x = radius * sin(glm::radians(theta)) * cos(glm::radians(phi));
				float y = radius * sin(glm::radians(theta)) * sin(glm::radians(phi));
				float z = radius * cos(glm::radians(theta));
				lightPos = glm::vec3(x, y, z);
				std::cout << "Light position updated successfully.\n";
			}
		}
		else {
			std::cout << "Invalid parameters. Please use format: " << cmd << " theta phi radius\n";
			std::cout << "Where: theta is between 10 and 80\n";
			std::cout << "       phi is between 0 and 360\n";
			std::cout << "       radius is greater than 0\n";
		}
	}
	else if (cmd == "power") {
		float power;
		std::cout << "\nEnter power value (>0): ";
		if (ss >> power && power > 0.0f) {
			lightPower = power;
			std::cout << "Light power updated to: " << lightPower << "\n";
		}
		else {
			std::cout << "Invalid power value. Please enter a number greater than 0.\n";
		}
	}
	else if (cmd == "move") {
		std::cout << "\nEnter chess move (e.g., e2e4): ";
	}
	else if (cmd == "help") {
		std::cout << "\nAvailable commands:\n";
		std::cout << "  camera theta phi radius - Update camera position\n";
		std::cout << "  light theta phi radius  - Update light position\n";
		std::cout << "  power value            - Update light power\n";
		std::cout << "  move e2e4              - Make a chess move\n";
		std::cout << "  quit                   - Exit the program\n";
	}
	else {
		std::cout << "Invalid command. Type 'help' for available commands.\n";
	}
}