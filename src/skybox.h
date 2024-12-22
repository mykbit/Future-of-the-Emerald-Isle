#ifndef SKYBOX_CLASS_H
#define SKYBOX_CLASS_H

#include "glm/detail/type_mat.hpp"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glad/gl.h>
#include <iostream>
#include "glm/gtx/transform.hpp"

#include "render/shader.h"

class Skybox {
    public:
    glm::vec3 position;
    glm::vec3 scale;
    // OpenGL buffers
	GLuint vertexArrayID; 
	GLuint vertexBufferID; 
	GLuint indexBufferID; 
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint textureID;
	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint textureSamplerID;
	GLuint shaderID;

    // Buffers
    GLfloat vertex_buffer_data[72] = {
    	// Front face
    	-1.0f, -1.0f, 1.0f, 
    	1.0f, -1.0f, 1.0f, 
    	1.0f, 1.0f, 1.0f, 
    	-1.0f, 1.0f, 1.0f, 
    
    	// Back face 
    	1.0f, -1.0f, -1.0f, 
    	-1.0f, -1.0f, -1.0f, 
    	-1.0f, 1.0f, -1.0f, 
    	1.0f, 1.0f, -1.0f,
    
    	// Left face
    	-1.0f, -1.0f, -1.0f, 
    	-1.0f, -1.0f, 1.0f, 
    	-1.0f, 1.0f, 1.0f, 
    	-1.0f, 1.0f, -1.0f, 
    	// Right face 
    	1.0f, -1.0f, 1.0f, 
    	1.0f, -1.0f, -1.0f, 
    	1.0f, 1.0f, -1.0f, 
    	1.0f, 1.0f, 1.0f,
    	// Top face
    	-1.0f, 1.0f, 1.0f, 
    	1.0f, 1.0f, 1.0f, 
    	1.0f, 1.0f, -1.0f, 
    	-1.0f, 1.0f, -1.0f, 
    	// Bottom face
    	-1.0f, -1.0f, -1.0f, 
    	1.0f, -1.0f, -1.0f, 
    	1.0f, -1.0f, 1.0f, 
    	-1.0f, -1.0f, 1.0f, 
    };

    GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
    	0, 1, 2, 	
    	0, 2, 3, 
    
    	4, 5, 6, 
    	4, 6, 7, 
    	8, 9, 10, 
    	8, 10, 11, 
    	12, 13, 14, 
    	12, 14, 15, 
    	16, 17, 18, 
    	16, 18, 19, 
    	20, 21, 22, 
    	20, 22, 23, 
    };

    // TODO: Define UV buffer data
    GLfloat uv_buffer_data[48] = {
      	// Front (Back without reversal)
      	1.0f / 4 * 4, 1.0f / 3 * 1, // (1,0)
      	1.0f / 4 * 3, 1.0f / 3 * 1, // (0,0)
      	1.0f / 4 * 3, 1.0f / 3 * 2, // (0,1)
      	1.0f / 4 * 4, 1.0f / 3 * 2, // (1,1)
      	// Back (Front without reversal)
      	1.0f / 4 * 2, 1.0f / 3 * 1,
      	1.0f / 4 * 1, 1.0f / 3 * 1,
      	1.0f / 4 * 1, 1.0f / 3 * 2,
      	1.0f / 4 * 2, 1.0f / 3 * 2,
      	// Left (Right without reversal)
      	1.0f / 4 * 1, 1.0f / 3 * 1, // 
      	1.0f / 4 * 0, 1.0f / 3 * 1,
      	1.0f / 4 * 0, 1.0f / 3 * 2,
      	1.0f / 4 * 1, 1.0f / 3 * 2,
      	// Right (Left without reversal)
      	1.0f / 4 * 3, 1.0f / 3 * 1,
      	1.0f / 4 * 2, 1.0f / 3 * 1,
      	1.0f / 4 * 2, 1.0f / 3 * 2,
      	1.0f / 4 * 3, 1.0f / 3 * 2,
      	// Top (Bottom without reversal)
      	1.0f / 4 * 1, 1.0f / 3 * 3,
      	1.0f / 4 * 2, 1.0f / 3 * 3,
      	1.0f / 4 * 2, 1.0f / 3 * 2,
      	1.0f / 4 * 1, 1.0f / 3 * 2,
      	// Bottom (Top without reversal)
      	1.0f / 4 * 1, 1.0f / 3 * 1,
      	1.0f / 4 * 2, 1.0f / 3 * 1,
      	1.0f / 4 * 2, 1.0f / 3 * 0,
      	1.0f / 4 * 1, 1.0f / 3 * 0,
    };

    Skybox(glm::vec3 position, glm::vec3 scale);
    void render(glm::mat4 cameraMatrix);
    void cleanup();

    private:
    GLuint LoadSkyboxTexture(const char *texture_file_path);
};

#endif