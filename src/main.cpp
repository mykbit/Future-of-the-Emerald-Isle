#include "glm/detail/type_mat.hpp"
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
#include "stb_image_write.h"

using namespace std;

static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;
static int shadowWidth = 1024;
static int shadowHeight = 1024;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void configureDepthMapFBO();
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Camera
static float cameraSpeed = 10.0f;
static glm::vec3 eye_center(0.0f, 40.0f, -150.0f);
static glm::vec3 lookat(0.0f, 0.0f, -1.0f);
static glm::vec3 up(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 0.1f;
static float zFar = 20000.0f; 

// Lighting  
static glm::vec3 lightIntensity(5e1f, 5e1f, 5e1f);
static glm::vec3 lightPosition(-800.0f, 300.0f, 800.0f);
// static glm::vec3 lightPosition(-275.0f, 500.0f, -275.0f);
static glm::vec3 lightLookat(0.0f, 0.0f, 0.0f);
static glm::vec3 lightUp(0.0f, 0.0f, 1.0f);
static float depthFoV = 75.0f; // Maybe fix 
static float depthNear = 0.1f;
static float depthFar = 5000.0f; 

GLuint lightPositionID;
GLuint lightIntensityID;
GLuint shadowShaderID;
GLuint lightingShaderID;
GLuint depthMapFBO;
GLuint depthCubemap;
bool shadows = true;

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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

	Shader depthShader = Shader("../src/shaders/depth.vert", "../src/shaders/depth.frag", "../src/shaders/depth.geom");
	Shader lightingShader = Shader("../src/shaders/lighting.vert", "../src/shaders/lighting.frag");
	Shader skyboxShader = Shader("../src/shaders/skybox.vert", "../src/shaders/skybox.frag");

	configureDepthMapFBO();
	lightingShader.use();
	lightingShader.setInt("diffuseTexture", 0);
	lightingShader.setInt("depthMap", 1);

	// Our 3D character
	StaticModel car = StaticModel("../src/assets/covered_car/covered_car_1k.gltf");
	glm::mat4 modelTransform = glm::mat4(1.0f);
	modelTransform = glm::translate(modelTransform, glm::vec3(0, 1, 0));
	modelTransform = glm::scale(modelTransform, glm::vec3(15, 15, 15));
	modelTransform = glm::rotate(modelTransform, glm::radians(45.0f), glm::vec3(0, 1, 0));


	Skybox skybox = Skybox(glm::vec3(0, 0, 0), glm::vec3(-10000, -10000, -10000), skyboxShader);

	Surface surface = Surface(glm::vec3(0, 0, 0), glm::vec3(10000, 1, 10000));

	// Building building = Building(shaderID);

	std::vector<glm::mat4> buildingTransforms;
	float building_spacing = 200.0f;
	int min_x = 60;
	int max_x = 70;
	int min_y = 160;
	int max_y = 200;
	std::random_device rd;
    std::mt19937 engine(rd());

	for (int i = -2; i < 2; i++) {
		for (int j = -2; j < 2; j++) {
			int x = std::uniform_int_distribution<int>(min_x, max_x)(engine);
			int y = std::uniform_int_distribution<int>(min_y, max_y)(engine);

			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::translate(transform, glm::vec3(i * building_spacing, y, j * building_spacing));
			transform = glm::scale(transform, glm::vec3(x, y, 60));
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

		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		// 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)shadowWidth / (float)shadowHeight, depthNear, depthFar);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPosition, lightPosition + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

		// 1. render scene to depth cubemap
        // --------------------------------
		glViewport(0, 0, shadowWidth, shadowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		depthShader.use();
		for (unsigned int i = 0; i < 6; ++i) {
			depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		}
		depthShader.setFloat("far_plane", depthFar);
		depthShader.setVec3("lightPos", lightPosition);
		surface.render(vp, depthShader);
		car.render(vp, modelTransform, depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glCullFace(GL_BACK);

		// 2. render scene as normal using the generated depth/shadow map
		// --------------------------------------------------------------
		glViewport(0, 0, windowWidth*2, windowHeight*2);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightingShader.use();
		lightingShader.setMat4("VP", vp);
		lightingShader.setVec3("viewPos", eye_center);
		lightingShader.setVec3("lightPos", lightPosition);
		// lightingShader.setVec3("lightIntensity", lightIntensity);
		lightingShader.setInt("shadows", shadows);
		lightingShader.setFloat("far_plane", depthFar);
		lightingShader.setInt("reverse_normals", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		surface.render(vp, lightingShader);
		car.render(vp, modelTransform, lightingShader);

		skyboxShader.use();
		skybox.render(vp);
		
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
	// model.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

static void configureDepthMapFBO() {
    glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; i++) {
	    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, 
	                 shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	    std::cerr << "Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}
