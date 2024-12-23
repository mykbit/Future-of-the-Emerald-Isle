#include "glm/detail/type_mat.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "static_model.h"
#include "skybox.h"
#include "surface.h"
#include "building.h"

#include <iomanip>
#include <random>
#include <vector>
#include <iostream>
#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Camera
static float cameraSpeed = 10.0f;
static glm::vec3 eye_center(0.0f, 40.0f, -150.0f);
static glm::vec3 lookat(0.0f, 0.0f, -1.0f);
static glm::vec3 up(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 0.1f;
static float zFar = 15000.0f; 

// Lighting  
static glm::vec3 lightIntensity(5e6f, 5e6f, 5e6f);
static glm::vec3 lightPosition(-275.0f, 500.0f, 800.0f);

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		cerr << "Failed to initialize GLFW." << endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(windowWidth, windowHeight, "Emerald Isle", NULL, NULL);
	if (window == NULL)
	{
		cerr << "Failed to open a GLFW window." << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);

	// Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		cerr << "Failed to initialize OpenGL context." << endl;
		return -1;
	}

	// Background
	glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Our 3D character
	StaticModel model = StaticModel("../src/assets/covered_car/covered_car_1k.gltf", "../src/shaders/simple.vert", "../src/shaders/simple.frag", glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 90, glm::vec3(0.0f, 1.0f, 0.0f), lightPosition, lightIntensity);

	Skybox skybox = Skybox(glm::vec3(0, 0, 0), glm::vec3(-10000, -10000, -10000));

	Surface surface = Surface(glm::vec3(0, 0, 0), glm::vec3(10000, 1, 10000));

	Building building = Building();

	std::vector<glm::mat4> buildingTransforms;
	float building_spacing = 100.0f;
	int min_x = 20;
	int max_x = 25;
	int min_y = 40;
	int max_y = 100;
	std::random_device rd;
    std::mt19937 engine(rd());

	for (int i = -2; i < 2; i++) {
		for (int j = -2; j < 2; j++) {
			int x = std::uniform_int_distribution<int>(min_x, max_x)(engine);
			int y = std::uniform_int_distribution<int>(min_y, max_y)(engine);

			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::translate(transform, glm::vec3(i * building_spacing, y, j * building_spacing));
			transform = glm::scale(transform, glm::vec3(x, y, 16));
			buildingTransforms.push_back(transform);
		}
	}

	// Camera setup
  	glm::mat4 viewMatrix, projectionMatrix;
	projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

	// Time and frame rate tracking
	static double lastTime = glfwGetTime();
	float time = 0.0f;			// Animation time 
	float fTime = 0.0f;			// Time for measuring fps
	unsigned long frames = 0;

	// Main loop
	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update states for animation
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;

		// Rendering
		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;
		
		skybox.render(vp);
		surface.render(vp);
		model.render(vp);
		for (glm::mat4 &t : buildingTransforms)
		{
			building.render(vp, t);
		}
		// FPS tracking 
		// Count number of frames over a few seconds and take average
		frames++;
		fTime += deltaTime;
		if (fTime > 2.0f) {		
			float fps = frames / fTime;
			frames = 0;
			fTime = 0;
			
			stringstream stream;
			stream << fixed << setprecision(2) << "Emerald Isle | " << fps << " FPS";
			glfwSetWindowTitle(window, stream.str().c_str());
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	model.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {	
	glm::vec3 forward = glm::normalize(lookat - eye_center);

    // Calculate right (perpendicular direction to both 'up' and 'forward')
    glm::vec3 right = glm::normalize(glm::cross(up, forward));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        // Move forward (along the 'lookat' direction)
        eye_center += cameraSpeed * forward;
        lookat += cameraSpeed * forward;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        // Move left (perpendicular to the 'lookat' direction)
		eye_center += cameraSpeed * right;
        lookat += cameraSpeed * right;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        // Move backward (opposite of the 'lookat' direction)
        eye_center -= cameraSpeed * forward;
        lookat -= cameraSpeed * forward;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        // Move right (perpendicular to the 'lookat' direction)
        eye_center -= cameraSpeed * right;
        lookat -= cameraSpeed * right;
    }
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  // Left arrow key
	{
		// Rotate the camera around the Y-axis (counterclockwise)
		float angle = 0.05f; // Set the rotation speed (radians)
		
		// Calculate new position for the camera
		float x = eye_center.x * cos(angle) - eye_center.z * sin(angle);
		float z = eye_center.x * sin(angle) + eye_center.z * cos(angle);
		
		// Update eye_center (camera position)
		eye_center.x = x;
		eye_center.z = z;

		// Update the lookat direction to always point to the center of the object
		lookat = glm::normalize(glm::vec3(-eye_center.x, 0.0f, -eye_center.z));
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)  // Right arrow key
	{
		// Rotate the camera around the Y-axis (clockwise)
		float angle = -0.05f; // Negative for clockwise
		
		// Calculate new position for the camera
		float x = eye_center.x * cos(angle) - eye_center.z * sin(angle);
		float z = eye_center.x * sin(angle) + eye_center.z * cos(angle);
		
		// Update eye_center (camera position)
		eye_center.x = x;
		eye_center.z = z;

		// Update the lookat direction to always point to the center of the object
		lookat = glm::normalize(glm::vec3(-eye_center.x, 0.0f, -eye_center.z));
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		eye_center.y += 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		eye_center.y -= 1.0f;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
