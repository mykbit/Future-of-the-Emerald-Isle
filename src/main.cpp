#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <render/shader.h>

#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void cursor_callback(GLFWwindow *window, double xpos, double ypos);

// OpenGL camera view parameters
static glm::vec3 eye_center(-278.0f, 273.0f, 800.0f);
static glm::vec3 lookat(-278.0f, 273.0f, 0.0f);
static glm::vec3 up(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 600.0f;
static float zFar = 1500.0f; // Maybe fix

// Lighting control 
const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);
static glm::vec3 lightIntensity = 5.0f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);
static glm::vec3 lightPosition(-275.0f, 500.0f, -275.0f);
static glm::vec3 lightLookat(-275.0f, 0.0f, -275.0f);
static glm::vec3 lightUp(0.0f, 0.0f, 1.0f);

// Shadow mapping
static int shadowMapWidth = windowWidth;
static int shadowMapHeight = windowHeight;

// TODO: set these parameters 
static float depthFoV = 75.0f; // Maybe fix 
static float depthNear = 1000.0f;
static float depthFar = depthNear * 2.0f; 

// Helper flag and function to save depth maps for debugging
static bool saveDepth = false;

struct CornellBox {

	// Refer to original Cornell Box data 
	// from https://www.graphics.cornell.edu/online/box/data.html

	GLfloat cornell_box_vertex_buffer_data[60] = {
		// Floor 
		-552.8, 0.0, 0.0,   
		0.0, 0.0,   0.0,
		0.0, 0.0, -559.2,
		-549.6, 0.0, -559.2,

		// Ceiling
		-556.0, 548.8, 0.0,   
		-556.0, 548.8, -559.2,
		0.0, 548.8, -559.2,
		0.0, 548.8,   0.0,

		// Left wall 
		-552.8,   0.0,   0.0, 
		-549.6,   0.0, -559.2,
		-556.0, 548.8, -559.2,
		-556.0, 548.8,   0.0,

		// Right wall 
		0.0,   0.0, -559.2,   
		0.0,   0.0,   0.0,
		0.0, 548.8,   0.0,
		0.0, 548.8, -559.2,

		// Back wall 
		-549.6,   0.0, -559.2, 
		0.0,   0.0, -559.2,
		0.0, 548.8, -559.2,
		-556.0, 548.8, -559.2,
	};

	// TODO: set vertex normals properly
	GLfloat cornell_box_normal_buffer_data[60] = {
    	// Floor 
    	0.0, 1.0, 0.0,   
    	0.0, 1.0, 0.0,
    	0.0, 1.0, 0.0,
    	0.0, 1.0, 0.0,
	
    	// Ceiling
    	0.0, -1.0, 0.0,   
    	0.0, -1.0, 0.0,
    	0.0, -1.0, 0.0,
    	0.0, -1.0, 0.0,
	
    	// Left wall 
    	1.0, 0.0, 0.0,   
    	1.0, 0.0, 0.0,
    	1.0, 0.0, 0.0,
    	1.0, 0.0, 0.0,
	
    	// Right wall 
    	-1.0, 0.0, 0.0,   
    	-1.0, 0.0, 0.0,
    	-1.0, 0.0, 0.0,
    	-1.0, 0.0, 0.0,
	
    	// Back wall 
    	0.0, 0.0, 1.0,   
    	0.0, 0.0, 1.0,
    	0.0, 0.0, 1.0,
    	0.0, 0.0, 1.0,
	};


	GLfloat cornell_box_color_buffer_data[60] = {
		// Floor
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Ceiling
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Left wall
		1.0f, 0.0f, 0.0f, 
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Right wall
		0.0f, 1.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 

		// Back wall
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f,
	};

	GLuint cornell_box_index_buffer_data[30] = {
    	// Floor
    	0, 1, 2, 
    	0, 2, 3,

    	// Ceiling
    	4, 5, 6, 
    	4, 6, 7,

    	// Left wall
    	8, 9, 10, 
    	8, 10, 11,

    	// Right wall
    	12, 13, 14, 
    	12, 14, 15,

    	// Back wall
    	16, 17, 18, 
    	16, 18, 19,
	};

	GLfloat short_box_vertex_buffer_data[60] = {
		// Short Box
		// Top face
		-130.0, 165.0,  -65.0, 
		-82.0, 165.0, -225.0,
		-240.0, 165.0, -272.0,
		-290.0, 165.0, -114.0,

		// Front face
		-290.0,   0.0, -114.0,
		-290.0, 165.0, -114.0,
		-240.0, 165.0, -272.0,
		-240.0,   0.0, -272.0,

		// Back face
		-130.0,   0.0,  -65.0,
		-130.0, 165.0,  -65.0,
		-290.0, 165.0, -114.0,
		-290.0,   0.0, -114.0,

		// Left face
		-82.0,   0.0, -225.0,
		-82.0, 165.0, -225.0,
		-130.0, 165.0,  -65.0,
		-130.0,   0.0,  -65.0,
	};

	GLfloat short_box_normal_buffer_data[60] = {
		// Short Box
    	// Top face (pointing up)
    	0.0, 1.0, 0.0,  
		0.0, 1.0, 0.0,  
		0.0, 1.0, 0.0,  
		0.0, 1.0, 0.0,
    	// Front face (pointing forward)
    	-1.0, 0.0, 0.0,  
		-1.0, 0.0, 0.0,  
		-1.0, 0.0, 0.0,  
		-1.0, 0.0, 0.0,
    	// Back face (pointing backward)
    	0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,  
		0.0, 0.0, 1.0,  
		0.0, 0.0, 1.0,
    	// Left face (pointing left)
    	1.0, 0.0, 0.0,  
		1.0, 0.0, 0.0,  
		1.0, 0.0, 0.0,  
		1.0, 0.0, 0.0,
    	// Right face (pointing right)
    	0.0, 0.0, -1.0,  
		0.0, 0.0, -1.0,  
		0.0, 0.0, -1.0,  
		0.0, 0.0, -1.0,
	};

	GLfloat short_box_color_buffer_data[60] = {
		// Short Box
		// Top face (white)
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,
		// Left face (white)
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,
		// Front face (white)
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,
		// Right face (white)
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,
		// Back face (white)
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f,
	};

	GLuint short_box_index_buffer_data[30] = {
		// Short Box
		// Top face
		0, 1, 2, 
		0, 2, 3,
		// Front face
		4, 5, 6, 
		4, 6, 7,
		// Back face
		8, 9, 10, 
		8, 10, 11,
		// Left face
		12, 13, 14, 
		12, 14, 15,
	};

	GLfloat tall_box_vertex_buffer_data[60] = {
		// Tall Box
		// Top face
		-423.0, 330.0, -247.0,
		-265.0, 330.0, -296.0,
		-314.0, 330.0, -456.0,
		-472.0, 330.0, -406.0,

		// Front face
		-423.0,   0.0, -247.0,
		-423.0, 330.0, -247.0,
		-472.0, 330.0, -406.0,
		-472.0,   0.0, -406.0,

		// Back face
		-472.0,   0.0, -406.0,
		-472.0, 330.0, -406.0,
		-314.0, 330.0, -456.0,
		-314.0,   0.0, -456.0,

		// Left face
		-314.0,   0.0, -456.0,
		-314.0, 330.0, -456.0,
		-265.0, 330.0, -296.0,
		-265.0,   0.0, -296.0,

		// Right face
		-265.0,   0.0, -296.0,
		-265.0, 330.0, -296.0,
		-423.0, 330.0, -247.0,
		-423.0,   0.0, -247.0,
	};

	GLfloat tall_box_normal_buffer_data[60] = {
		// Tall Box
		// Top face
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,

		// Front face
		-1.0, 0.0, 0.0,
		-1.0, 0.0, 0.0,
		-1.0, 0.0, 0.0,
		-1.0, 0.0, 0.0,

		// Back face
		0.0, 0.0, -1.0,
		0.0, 0.0, -1.0,
		0.0, 0.0, -1.0,
		0.0, 0.0, -1.0,

		// Left face
		1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,

		// Right face
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,
	};
	
	GLfloat tall_box_color_buffer_data[60] = {
		// Tall Box
		// Top face (white)
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		// Left face (white)
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		// Front face (white)
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		// Right face (white)
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		// Back face (white)
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
	};

	GLuint tall_box_index_buffer_data[30] = {
		// Tall Box
		// Top face
		0, 1, 2, 
		0, 2, 3,
		// Front face
		4, 5, 6, 
		4, 6, 7,
		// Back face
		8, 9, 10, 
		8, 10, 11,
		// Left face
		12, 13, 14, 
		12, 14, 15,
		// Right face
		16, 17, 18,
		16, 18, 19,
	};

	// OpenGL buffers
	GLuint cornellBoxVertexArrayID;
	GLuint cornellBoxVertexBufferID; 
	GLuint cornellBoxIndexBufferID;
	GLuint cornellBoxColorBufferID;
	GLuint cornellBoxNormalBufferID;

	GLuint shortBoxVertexArrayID;
	GLuint shortBoxVertexBufferID;
	GLuint shortBoxIndexBufferID;
	GLuint shortBoxColorBufferID;
	GLuint shortBoxNormalBufferID;

	GLuint tallBoxVertexArrayID;
	GLuint tallBoxVertexBufferID;
	GLuint tallBoxIndexBufferID;
	GLuint tallBoxColorBufferID;
	GLuint tallBoxNormalBufferID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint programID;
	GLuint simpleDepthID;
	GLuint lightSpaceMatrixID_box;
	GLuint lightSpaceMatrixID_shadow;
	GLuint shadowMapID;

	// Depth map
	GLuint depthMapFBO;
	GLuint depthMapTexture;

	void initialize() {
		// Cornell Box
		// Create a vertex array object
		glGenVertexArrays(1, &cornellBoxVertexArrayID);
		glBindVertexArray(cornellBoxVertexArrayID);

		// Create a vertex buffer object to store the vertex data		
		glGenBuffers(1, &cornellBoxVertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, cornellBoxVertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cornell_box_vertex_buffer_data), cornell_box_vertex_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
		glGenBuffers(1, &cornellBoxColorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, cornellBoxColorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cornell_box_color_buffer_data), cornell_box_color_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the vertex normals		
		glGenBuffers(1, &cornellBoxNormalBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, cornellBoxNormalBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cornell_box_normal_buffer_data), cornell_box_normal_buffer_data, GL_STATIC_DRAW);

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &cornellBoxIndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cornellBoxIndexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cornell_box_index_buffer_data), cornell_box_index_buffer_data, GL_STATIC_DRAW);

		// Short box
		glGenVertexArrays(1, &shortBoxVertexArrayID);
		glBindVertexArray(shortBoxVertexArrayID);
		glGenBuffers(1, &shortBoxVertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, shortBoxVertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(short_box_vertex_buffer_data), short_box_vertex_buffer_data, GL_STATIC_DRAW);
		glGenBuffers(1, &shortBoxColorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, shortBoxColorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(short_box_color_buffer_data), short_box_color_buffer_data, GL_STATIC_DRAW);
		glGenBuffers(1, &shortBoxNormalBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, shortBoxNormalBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(short_box_normal_buffer_data), short_box_normal_buffer_data, GL_STATIC_DRAW);
		glGenBuffers(1, &shortBoxIndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shortBoxIndexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short_box_index_buffer_data), short_box_index_buffer_data, GL_STATIC_DRAW);

		// Tall box
		glGenVertexArrays(1, &tallBoxVertexArrayID);
		glBindVertexArray(tallBoxVertexArrayID);
		glGenBuffers(1, &tallBoxVertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, tallBoxVertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tall_box_vertex_buffer_data), tall_box_vertex_buffer_data, GL_STATIC_DRAW);
		glGenBuffers(1, &tallBoxColorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, tallBoxColorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tall_box_color_buffer_data), tall_box_color_buffer_data, GL_STATIC_DRAW);
		glGenBuffers(1, &tallBoxNormalBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, tallBoxNormalBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tall_box_normal_buffer_data), tall_box_normal_buffer_data, GL_STATIC_DRAW);
		glGenBuffers(1, &tallBoxIndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tallBoxIndexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tall_box_index_buffer_data), tall_box_index_buffer_data, GL_STATIC_DRAW);

		// Create a depth map FBO
		glGenFramebuffers(1, &depthMapFBO);
		glGenTextures(1, &depthMapTexture);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    	// attach depth texture as FBO's depth buffer
    	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    	glDrawBuffer(GL_NONE);
    	glReadBuffer(GL_NONE);
    	glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../src/box.vert", "../src/box.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}
		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
		lightPositionID = glGetUniformLocation(programID, "lightPosition");
		lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
		lightSpaceMatrixID_box = glGetUniformLocation(programID, "lightSpaceMatrix");
		shadowMapID = glGetUniformLocation(programID, "shadowMap");

		// Create and compile our GLSL program for simple depth rendering
		simpleDepthID = LoadShadersFromFile("../src/simpledepth.vert", "../src/simpledepth.frag");
		if (simpleDepthID == 0)
		{
			std::cerr << "Failed to load simple depth shaders." << std::endl;
		}
		lightSpaceMatrixID_shadow = glGetUniformLocation(simpleDepthID, "lightSpaceMatrix");
	}

	void renderLightingPass(glm::mat4 cameraMatrix, glm::mat4 lightSpaceMatrix) {
		glUseProgram(programID);

		// Cornell Box
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(shadowMapID, 0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, cornellBoxVertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, cornellBoxColorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, cornellBoxNormalBufferID);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cornellBoxIndexBufferID);
		// Set model-view-projection matrix
		glm::mat4 mvp = cameraMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
		// Set light data 
		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &cameraMatrix[0][0]);
		glUniformMatrix4fv(lightSpaceMatrixID_box, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
		// Draw the Cornell box
		glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Short box
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(shadowMapID, 0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, shortBoxVertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shortBoxIndexBufferID);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, shortBoxColorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, shortBoxNormalBufferID);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &cameraMatrix[0][0]);
		glUniformMatrix4fv(lightSpaceMatrixID_box, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Tall box
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(shadowMapID, 0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, tallBoxVertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tallBoxIndexBufferID);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, tallBoxColorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, tallBoxNormalBufferID);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &cameraMatrix[0][0]);
		glUniformMatrix4fv(lightSpaceMatrixID_box, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}

	void renderShadowPass(glm::mat4 viewProjectionMatrix) {
		glViewport(0, 0, shadowMapWidth, shadowMapHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		// Short box
    	glUseProgram(simpleDepthID);
    	glEnableVertexAttribArray(0);
    	glBindBuffer(GL_ARRAY_BUFFER, shortBoxVertexBufferID);
    	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shortBoxIndexBufferID);
    	glUniformMatrix4fv(lightSpaceMatrixID_shadow, 1, GL_FALSE, &viewProjectionMatrix[0][0]);
    	glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
		glDisableVertexAttribArray(0);

    	// Tall box
		glUseProgram(simpleDepthID);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, tallBoxVertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tallBoxIndexBufferID);
		glUniformMatrix4fv(lightSpaceMatrixID_shadow, 1, GL_FALSE, &viewProjectionMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
		glDisableVertexAttribArray(0);

		glCullFace(GL_BACK);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// This function retrieves and stores the depth map of the default frame buffer 
	// or a particular frame buffer (indicated by FBO ID) to a PNG image.
	void saveDepthTexture(std::string filename) {
	    int width = shadowMapWidth;
	    int height = shadowMapHeight;
		if (shadowMapWidth == 0 || shadowMapHeight == 0) {
			width = windowWidth;
			height = windowHeight;
		}
	    int channels = 3;
	
	    std::vector<float> depth(width * height);
	    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	    glReadBuffer(GL_DEPTH_COMPONENT);
	    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
	    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	    std::vector<unsigned char> img(width * height * 3);
	    for (int i = 0; i < width * height; ++i) img[3*i] = img[3*i+1] = img[3*i+2] = depth[i] * 255;

	    stbi_write_png(filename.c_str(), width, height, channels, img.data(), width * channels);
	}

	void cleanup() {
		glDeleteBuffers(1, &cornellBoxVertexBufferID);
		glDeleteBuffers(1, &cornellBoxColorBufferID);
		glDeleteBuffers(1, &cornellBoxIndexBufferID);
		glDeleteBuffers(1, &cornellBoxNormalBufferID);
		glDeleteVertexArrays(1, &cornellBoxVertexArrayID);
		glDeleteBuffers(1, &shortBoxVertexBufferID);
		glDeleteBuffers(1, &shortBoxColorBufferID);
		glDeleteBuffers(1, &shortBoxIndexBufferID);
		glDeleteBuffers(1, &shortBoxNormalBufferID);
		glDeleteVertexArrays(1, &shortBoxVertexArrayID);
		glDeleteBuffers(1, &tallBoxVertexBufferID);
		glDeleteBuffers(1, &tallBoxColorBufferID);
		glDeleteBuffers(1, &tallBoxIndexBufferID);
		glDeleteBuffers(1, &tallBoxNormalBufferID);
		glDeleteVertexArrays(1, &tallBoxVertexArrayID);
		glDeleteFramebuffers(1, &depthMapFBO);
		glDeleteTextures(1, &depthMapTexture);
		glDeleteProgram(simpleDepthID);
		glDeleteProgram(programID);
	}
}; 

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(windowWidth, windowHeight, "Lab 3", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);

	glfwSetCursorPosCallback(window, cursor_callback);

	// Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}

	// Background
	glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Prepare shadow map size for shadow mapping. Usually this is the size of the window itself, but on some platforms like Mac this can be 2x the size of the window. Use glfwGetFramebufferSize to get the shadow map size properly. 
    glfwGetFramebufferSize(window, &shadowMapWidth, &shadowMapHeight);

    // Create the classical Cornell Box
	CornellBox b;
	b.initialize();

    glm::mat4 lightProjectionMatrix = glm::perspective(glm::radians(depthFoV), (float)shadowMapWidth / shadowMapHeight, depthNear, depthFar);
	// glm::mat4 lightProjectionMatrix = glm::ortho(-250.0f, 250.0f, -250.0f, 250.0f, depthNear, depthFar);

	// Camera setup
    glm::mat4 viewMatrix, projectionMatrix;
	projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 direction = lightPosition - lightLookat;
    	glm::vec3 normalizedDirection = glm::normalize(direction);
    	glm::vec3 offset = normalizedDirection * depthNear;
    	glm::vec3 depthPosition = lightPosition + offset;
    	glm::mat4 lightViewMatrix = glm::lookAt(depthPosition, lightLookat, lightUp);
    	glm::mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		
		b.renderShadowPass(lightSpaceMatrix);
		b.renderLightingPass(vp, lightSpaceMatrix);

		if (saveDepth) {
            std::string filename = "depth_camera.png";
            b.saveDepthTexture(filename);
            std::cout << "Depth texture saved to " << filename << std::endl;
            saveDepth = false;
        }

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	b.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
  if (key == GLFW_KEY_R && action == GLFW_PRESS) {
    eye_center = glm::vec3(-278.0f, 273.0f, 800.0f);
    lightPosition = glm::vec3(-275.0f, 500.0f, -275.0f);
  }

  if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    eye_center.y += 20.0f;
  }

  if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    eye_center.y -= 20.0f;
  }

  if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    eye_center.x -= 20.0f;
  }

  if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    eye_center.x += 20.0f;
  }

  if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    lightPosition.z -= 20.0f;
  }

  if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    lightPosition.z += 20.0f;
  }

  if (key == GLFW_KEY_SPACE && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    saveDepth = true;
  }

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

static void cursor_callback(GLFWwindow *window, double xpos, double ypos) {
  if (xpos < 0 || xpos >= windowWidth || ypos < 0 || ypos > windowHeight)
    return;

  // Normalize to [0, 1]
  float x = xpos / windowWidth;
  float y = ypos / windowHeight;

  // To [-1, 1] and flip y up
  x = x * 2.0f - 1.0f;
  y = 1.0f - y * 2.0f;

  const float scale = 250.0f;
  lightPosition.x = x * scale - 278;
  lightPosition.y = y * scale + 278;

  // std::cout << lightPosition.x << " " << lightPosition.y << " " << lightPosition.z << std::endl;
}