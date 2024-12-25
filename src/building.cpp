#include "building.h"
#include "glm/detail/type_mat.hpp"
#include "stb_image.h"

Building::Building(glm::mat4* modelMatrices, int amount) {
    this->modelMatrices = modelMatrices;
    this->amount = amount;
    // Create a vertex array object
    glGenVertexArrays(1, &this->vertexArrayID);
    glBindVertexArray(this->vertexArrayID);
    // Create a vertex buffer object to store the vertex data		
    glGenBuffers(1, &this->vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertex_buffer_data), this->vertex_buffer_data, GL_STATIC_DRAW);
    // Create an index buffer object to store the index data that defines triangle faces
    glGenBuffers(1, &this->indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->index_buffer_data), this->index_buffer_data, GL_STATIC_DRAW);
    // Create a UV buffer object to store the UV data
    for (int i = 0; i < 24; ++i) this->uv_buffer_data[2*i+1] *= 5;
    glGenBuffers(1, &this->uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, this->uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(this->uv_buffer_data), this->uv_buffer_data, GL_STATIC_DRAW);
    // Create a normal buffer object to store the normal data
    glGenBuffers(1, &this->normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, this->normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(this->normal_buffer_data), this->normal_buffer_data, GL_STATIC_DRAW);
    // Create a transform buffer object to store the model matrices
    glGenBuffers(1, &this->transformBufferID);  
    glBindBuffer(GL_ARRAY_BUFFER, this->transformBufferID);
    glBufferData(GL_ARRAY_BUFFER, this->amount * sizeof(glm::mat4), &this->modelMatrices[0], GL_STATIC_DRAW);
    // Get a handle for our "MVP" uniform
    this->textureID = LoadTextureTileBox("../src/assets/textures/building.jpg");
}

GLuint Building::LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;
    glGenTextures(1, &texture);  
    glBindTexture(GL_TEXTURE_2D, texture);  

    // To tile textures on a box, we set wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (img) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }
    stbi_image_free(img);

    return texture;
}

void Building::render(glm::mat4 cameraMatrix, Shader& shader) {
    glBindVertexArray(this->vertexArrayID);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBufferID);
    // Enable normal buffer
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, this->normalBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// TODO: Enable UV buffer and texture sampler
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->uvBufferID);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // Bind transform data
    glBindBuffer(GL_ARRAY_BUFFER, this->transformBufferID);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
	// Set textureSampler to use texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->textureID);
	shader.setMat4("model", glm::mat4(1.0f));
	// Draw the box
	glDrawElementsInstanced(
		GL_TRIANGLES,      // mode
		36,    			   // number of indices
		GL_UNSIGNED_INT,   // type
		(void*)0,           // element array buffer offset
        this->amount
	);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glDisableVertexAttribArray(6);
    glBindVertexArray(0);
}

void Building::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteTextures(1, &textureID);
    // glDeleteProgram(shaderID);
}