#ifndef SURFACE_CLASS_H
#define SURFACE_CLASS_H

#include "glm/detail/type_mat.hpp"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glad/gl.h>
#include <iostream>
#include "glm/gtx/transform.hpp"

#include "render/shader.h"

class Surface {
    public:
    glm::vec3 position;
    glm::vec3 scale;
    // OpenGL buffers
	GLuint vertexArrayID; 
	GLuint vertexBufferID; 
	GLuint indexBufferID; 
	GLuint uvBufferID;
	GLuint textureID;
    GLuint normalBufferID;

    GLfloat vertex_buffer_data[12] = {
        -1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, -1.0f, 
		-1.0f, 1.0f, -1.0f, 
    };

    // UV coordinates for the texture
    GLfloat uv_buffer_data[8] = {
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f 
    };

    GLfloat normal_buffer_data[12] = {
        // Top face
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
    };

    // Index data for two triangles forming the quad
    GLuint index_buffer_data[6] = {
        0, 1, 2,
        0, 2, 3 
    };

    Surface(glm::vec3 position, glm::vec3 scale);
    void render(glm::mat4 cameraMatrix, Shader& shader);
    void cleanup();

    private:
    GLuint LoadTextureTileBox(const char *texture_file_path);
};

#endif