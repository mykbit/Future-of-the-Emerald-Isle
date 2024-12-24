#ifndef BUILDING_CLASS_H
#define BUILDING_CLASS_H

#include "glm/detail/type_mat.hpp"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glad/gl.h>
#include <iostream>
#include "glm/detail/type_vec.hpp"
#include "glm/gtx/transform.hpp"

#include "render/shader.h"

class Building {
	public:
    GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
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

	GLfloat normal_buffer_data[72] = {
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

		// Bottom face
		0.0, -1.0, 0.0,
		0.0, -1.0, 0.0,
		0.0, -1.0, 0.0,
		0.0, -1.0, 0.0,
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
    	// Front
    	0.0f, 1.0f,
    	1.0f, 1.0f,
    	1.0f, 0.0f,
    	0.0f, 0.0f,
    	// Back
    	0.0f, 1.0f,
    	1.0f, 1.0f,
    	1.0f, 0.0f,
    	0.0f, 0.0f,
    	// Left
    	0.0f, 1.0f,
    	1.0f, 1.0f,
    	1.0f, 0.0f,
    	0.0f, 0.0f,
    	// Right
    	0.0f, 1.0f,
    	1.0f, 1.0f,
    	1.0f, 0.0f,
    	0.0f, 0.0f,
    	// Top - We do not want texture the top
    	0.0f, 0.0f,
    	0.0f, 0.0f,
    	0.0f, 0.0f,
    	0.0f, 0.0f,
    	// Bottom - We do not want texture the bottom
    	0.0f, 0.0f,
    	0.0f, 0.0f,
    	0.0f, 0.0f,
    	0.0f, 0.0f,
    };

    // OpenGL buffers
    GLuint vertexArrayID; 
    GLuint vertexBufferID; 
    GLuint indexBufferID; 
    GLuint colorBufferID;
	GLuint normalBufferID;
    GLuint uvBufferID;
    GLuint textureID;
    // Shader variable IDs
    GLuint modelMatrixID;
	GLuint vpMatrixID;
    GLuint textureSamplerID;
    GLuint shaderID;
	GLuint lightPositionID;
	GLuint lightIntensityID;

    Building(GLuint& shaderID);
    void render(glm::mat4 cameraMatrix, glm::mat4 transform);
    void cleanup();

    private:
    GLuint LoadTextureTileBox(const char *texture_file_path);
};
#endif