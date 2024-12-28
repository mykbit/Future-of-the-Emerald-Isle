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
static int shadowWidth = 16384;
static int shadowHeight = 16384;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
static void processInput(GLFWwindow *window);
static void configureDepthMapFBO();
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void setupCarModelMatrices(glm::mat4* modelMatrices, int amount);
static void setupBuildingModelMatrices(glm::mat4* modelMatrices, int amount);
static void setupTreeModelMatrices(glm::mat4* modelMatrices, int amount);
static void setupRoadBlockModelMatrices(glm::mat4* modelMatrices, int amount);
static void setupGrassModelMatrices(glm::mat4* modelMatrices, int amount);
static void setupAirplaneModelMatrices(glm::mat4* modelMatrices, int amount);

// Camera
static float cameraSpeed = 1000.0f;
static glm::vec3 eye_center(0.0f, 150.0f, -800.0f);
static glm::vec3 lookat(0.0f, 0.0f, -1.0f);
static glm::vec3 up(0.0f, 1.0f, 0.0f);
static float yaw = -90.0f;
static float pitch = 0.0f;
static float lastX = windowWidth / 2.0f;
static float lastY = windowHeight / 2.0f;
static bool firstMouse = true;
static float sensitivity = 0.1f;
static float deltaTime = 0.0f;
static float lastTime = 0.0f;
static float FoV = 45.0f;
static float zNear = 0.1f;
static float zFar = 20000.0f; 

// Lighting  
static glm::vec3 lightIntensity(2.0f, 2.0f, 2.0f);
static glm::vec3 lightPosition(-7700.0f, 1400.0f, 10000.0f);
// static glm::vec3 lightPosition(-275.0f, 500.0f, -275.0f);
static glm::vec3 lightLookat(0.0f, -1.0f, 0.0f);
static glm::vec3 lightUp(0.0f, 0.0f, 1.0f);
static float depthFoV = 75.0f; // Maybe fix 
static float depthNear = 0.1f;
static float depthFar = 20000.0f;

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

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

	// Skybox
	Skybox skybox = Skybox(glm::vec3(0, 0, 0), glm::vec3(-10000, -10000, -10000), skyboxShader);

	// Surface
	int amount = 1;
	glm::mat4* surfaceModelMatrices = new glm::mat4[amount];
	glm::mat4 surfaceModel = glm::mat4(1.0f);
	surfaceModel = glm::translate(surfaceModel, glm::vec3(0, 0, 0));
	surfaceModel = glm::scale(surfaceModel, glm::vec3(10000, 1, 10000));
	surfaceModelMatrices[0] = surfaceModel;
	Surface surface = Surface(surfaceModelMatrices, amount);

	// Debug light cube
	amount = 1;
	glm::mat4* lightCubeModelMatrices = new glm::mat4[amount];
	glm::mat4 lightCubeModel = glm::mat4(1.0f);
	lightCubeModel = glm::translate(lightCubeModel, lightPosition);
	lightCubeModel = glm::scale(lightCubeModel, glm::vec3(100, 100, 100));
	lightCubeModelMatrices[0] = lightCubeModel;
	StaticModel lightCube = StaticModel("../src/assets/cube/Cube.gltf", lightCubeModelMatrices, amount);

	// Cars
	amount = 28;
	glm::mat4* carModelMatrices = new glm::mat4[amount];
	setupCarModelMatrices(carModelMatrices, amount);
	StaticModel car = StaticModel("../src/assets/covered_car/covered_car_1k.gltf", carModelMatrices, amount);

	// Buildings
	amount = 36;
	glm::mat4* buildingModelMatrices = new glm::mat4[amount];
	setupBuildingModelMatrices(buildingModelMatrices, amount);
	Building building = Building(buildingModelMatrices, amount);

	// Trees
	amount = 700;
	glm::mat4* treeModelMatrices = new glm::mat4[amount];
	setupTreeModelMatrices(treeModelMatrices, amount);
	StaticModel tree = StaticModel("../src/assets/quiver_tree/quiver_tree_02_1k.gltf", treeModelMatrices, amount);

	// Road blocks
	amount = 50;
	glm::mat4* roadBlockModelMatrices = new glm::mat4[amount];
	setupRoadBlockModelMatrices(roadBlockModelMatrices, amount);
	StaticModel roadBlock = StaticModel("../src/assets/concrete_road_barrier/concrete_road_barrier_1k.gltf", roadBlockModelMatrices, amount);

	// Grass
	// amount = 5000;
	// glm::mat4* grassModelMatrices = new glm::mat4[amount];
	// setupGrassModelMatrices(grassModelMatrices, amount);
	// StaticModel grass = StaticModel("../src/assets/grass/grass_medium_01_1k.gltf", grassModelMatrices, amount);

	// Airplane
	amount = 1;
	glm::mat4* airplaneModelMatrices = new glm::mat4[amount];
	setupAirplaneModelMatrices(airplaneModelMatrices, amount);
	glm::mat4 airplaneMovementMatrix = glm::mat4(1.0f);
	glm::vec3 flightRestrictions = glm::vec3(3000, 3000, 3000);
	int transCount = 0;
	StaticModel airplane = StaticModel("../src/assets/airplane/airplane.glb", airplaneModelMatrices, amount);

	// Camera setup
  	glm::mat4 viewMatrix, projectionMatrix, vp;
	projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);
	viewMatrix = glm::lookAt(eye_center, eye_center + lookat, up);
	vp = projectionMatrix * viewMatrix;

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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
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
	car.render(vp, depthShader);
	building.render(vp, depthShader);
	tree.render(vp, depthShader);
	roadBlock.render(vp, depthShader);
	airplane.render(vp, depthShader);
	// grass.render(vp, depthShader);
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Time and frame rate tracking
	lastTime = glfwGetTime();
	float time = 0.0f;			// Animation time 
	float fTime = 0.0f;			// Time for measuring fps
	unsigned long frames = 0;

	// Main loop
	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update states for animation
        double currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;

		viewMatrix = glm::lookAt(eye_center, eye_center + lookat, up);
		vp = projectionMatrix * viewMatrix;
		
		// 2. render scene as normal using the generated depth/shadow map
		// --------------------------------------------------------------
		glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightingShader.use();
		lightingShader.setMat4("VP", vp);
		lightingShader.setVec3("viewPos", eye_center);
		lightingShader.setVec3("lightPos", lightPosition);
		lightingShader.setVec3("lightIntensity", lightIntensity);
		lightingShader.setInt("shadows", 1);
		lightingShader.setFloat("far_plane", depthFar);
		lightingShader.setInt("reverse_normals", 0);
		lightingShader.setInt("diffuseTexture", 0);
		lightingShader.setInt("depthMap", 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		surface.render(vp, lightingShader);
		car.render(vp, lightingShader);
		building.render(vp, lightingShader);
		tree.render(vp, lightingShader);
		roadBlock.render(vp, lightingShader);

		airplaneMovementMatrix = glm::translate(airplaneMovementMatrix, glm::vec3(0, 0, 1));
		airplane.render(vp, lightingShader, airplaneMovementMatrix);
		transCount++;
		if ((transCount >= flightRestrictions.x) || (transCount >= flightRestrictions.y) || (transCount >= flightRestrictions.z)) {
			airplaneMovementMatrix = glm::mat4(1.0f);
			transCount = 0;
		} 
		// grass.render(vp, lightingShader);

		// 3. Render the skybox separately from the rest of the scene
		// --------------------------------------------------------------
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
		processInput(window);
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

static void setupCarModelMatrices(glm::mat4* modelMatrices, int amount) {
	int count = 0;
	glm::mat4 identity = glm::mat4(1.0f);
    glm::mat4 model = glm::translate(identity, glm::vec3(80, 1, 0));\
	model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
    modelMatrices[count++] = model;

	model = glm::translate(identity, glm::vec3(300, 1, -80));
	model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	modelMatrices[count++] = model;
	
	model = glm::translate(identity, glm::vec3(520, 1, 580));
	model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
	modelMatrices[count++] = model;

	int car_spacing = 0;
	for (count = count; count < amount; count++) {
		// Place cars in the backyard
		model = glm::translate(identity, glm::vec3(750 - car_spacing, 1, 800));
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0, 1, 0));
		modelMatrices[count] = model;
		car_spacing += 50;
	}
}

static void setupBuildingModelMatrices(glm::mat4* modelMatrices, int amount) {
	float building_spacing = 300.0f;
	int min_x = 60;
	int max_x = 70;
	int min_y = 160;
	int max_y = 200;
	std::random_device rd;
    std::mt19937 engine(rd());
	vector<glm::mat4> buildings;
	for (int i = -amount / 12; i < amount / 12; i++) {
		for (int j = -amount / 12; j < amount / 12; j++) {
			int x = std::uniform_int_distribution<int>(min_x, max_x)(engine);
			int y = std::uniform_int_distribution<int>(min_y, max_y)(engine);
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(i * building_spacing, y, j * building_spacing));
			model = glm::scale(model, glm::vec3(x, y, 48));
			buildings.push_back(model);
		}
	}

	for (int i = 0; i < amount; i++) {
		modelMatrices[i] = buildings[i];
	}
}

static void setupTreeModelMatrices(glm::mat4* modelMatrices, int amount) {
	srand(glfwGetTime()); // initialize random seed	
	float radius = 3000.0;
	float offset = 1500.0f;
	for (int i = 0; i < amount; i++)
	{
	    glm::mat4 model = glm::mat4(1.0f);
	    // 1. translation: displace along circle with 'radius' in range [-offset, offset]
	    float angle = (float)i / (float)amount * 360.0f;
	    float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
	    float x = sin(angle) * radius + displacement;
	    displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
	    float z = cos(angle) * radius + displacement;
	    model = glm::translate(model, glm::vec3(x, 0, z));

	    // 2. scale: scale between 5 and 10
	    // float scale = (rand() % 5 + 5) / 10.0f;
		model = glm::scale(model, glm::vec3(150, 150, 150));

	    // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
	    float rotAngle = (rand() % 360);
	    model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 1.0f, 0.0f));

	    // 4. now add to list of matrices
	    modelMatrices[i] = model;
	}  
}

static void setupRoadBlockModelMatrices(glm::mat4* modelMatrices, int amount) {
    float cityWidth = 2000.0f;   // Width of the city
    float cityHeight = 2000.0f;  // Height of the city
    float spacing = 200.0f;      // Spacing between roadblocks
    float roadBlockHeight = 10.0f;  // Height of the roadblocks

 	int count = 0;
    glm::mat4 model;
    
    // Place roadblocks along the top edge
    for (float x = -cityWidth / 2; x <= cityWidth / 2; x += spacing) {
        if (count >= amount) break;
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, roadBlockHeight, -cityHeight / 2));
        model = glm::scale(model, glm::vec3(100, 100, 100)); // Adjust scale if needed
        modelMatrices[count++] = model;
    }
 
    // Place roadblocks along the bottom edge
    for (float x = -cityWidth / 2; x <= cityWidth / 2; x += spacing) {
        if (count >= amount) break;
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, roadBlockHeight, cityHeight / 2));
        model = glm::scale(model, glm::vec3(100, 100, 100)); // Adjust scale if needed
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrices[count++] = model;
    }

    // Place roadblocks along the left edge
    for (float z = -cityHeight / 2; z <= cityHeight / 2; z += spacing) {
        if (count >= amount) break;
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-cityWidth / 2, roadBlockHeight, z));
        model = glm::scale(model, glm::vec3(100, 100, 100)); // Adjust scale if needed
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrices[count++] = model;
    }

    // Place roadblocks along the right edge
    for (float z = -cityHeight / 2; z <= cityHeight / 2; z += spacing) {
        if (count >= amount) break;
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cityWidth / 2, roadBlockHeight, z));
        model = glm::scale(model, glm::vec3(100, 100, 100)); // Adjust scale if needed
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrices[count++] = model;
    }
}

static void setupGrassModelMatrices(glm::mat4* modelMatrices, int amount) {
	float fieldWidth = 2000;
	float fieldHeight = 2500;
	float spacing = 35;

	int count = 0;
	glm::mat4 model;
	
	for (int x = 0; x < fieldWidth; x += spacing) {
		for (int z = 0; z < fieldHeight; z += spacing) {
			if (count >= amount) break;
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3((-fieldWidth) / 2 + x, 1, (-fieldHeight) / 2 + z));
			model = glm::scale(model, glm::vec3(50, 50, 50));
			modelMatrices[count++] = model;
		}
	}
}

static void setupAirplaneModelMatrices(glm::mat4* modelMatrices, int amount) {
	for (int i = 0; i < amount; i++) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(200, 500, -10000));
		model = glm::scale(model, glm::vec3(10, 10, 10));
		modelMatrices[i] = model;
	}
}
 
static void configureDepthMapFBO() {
    glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < 6; i++) {
	    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	    std::cerr << "Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    lookat = glm::normalize(front);
}

static void processInput(GLFWwindow* window) {
    float velocity = cameraSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        eye_center += velocity * lookat;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        eye_center -= velocity * lookat;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        eye_center -= glm::normalize(glm::cross(lookat, up)) * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        eye_center += glm::normalize(glm::cross(lookat, up)) * velocity;
}
