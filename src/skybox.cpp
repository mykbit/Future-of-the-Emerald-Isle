#include "skybox.h"
#include "stb_image.h"

Skybox::Skybox(glm::vec3 position, glm::vec3 scale) {
	// Define scale of the building geometry
	this->position = position;
	this->scale = scale;
	// Create a vertex array object
	glGenVertexArrays(1, &this->vertexArrayID);
	glBindVertexArray(this->vertexArrayID);
	// Create a vertex buffer object to store the vertex data		
	glGenBuffers(1, &this->vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertex_buffer_data), this->vertex_buffer_data, GL_STATIC_DRAW);
	// TODO: Create a vertex buffer object to store the UV data
	glGenBuffers(1, &this->uvBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->uvBufferID);	
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->uv_buffer_data), this->uv_buffer_data, GL_STATIC_DRAW);
	// Create an index buffer object to store the index data that defines triangle faces
	glGenBuffers(1, &this->indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->index_buffer_data), this->index_buffer_data, GL_STATIC_DRAW);
	// Create and compile our GLSL program from the shaders
	this->shaderID = LoadShadersFromFile("../src/shaders/skybox.vert", "../src/shaders/skybox.frag");
	if (this->shaderID == 0)
	{
		std::cerr << "Failed to load shaders." << std::endl;
	}
	// Get a handle for our "MVP" uniform
	mvpMatrixID = glGetUniformLocation(shaderID, "MVP");
    textureID = LoadSkyboxTexture("../src/assets/textures/sky.png");
    textureSamplerID = glGetUniformLocation(shaderID,"textureSampler");
}

GLuint Skybox::LoadSkyboxTexture(const char *texture_file_path) {
	int w, h, channels;
	uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

void Skybox::render(glm::mat4 cameraMatrix) {
	glUseProgram(this->shaderID);

    glBindVertexArray(this->vertexArrayID);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBufferID);
	// TODO: Model transform 
	// -----------------------
    glm::mat4 modelMatrix = glm::mat4();    
    // Scale the box along each axis to make it look like a building
	modelMatrix = glm::scale(modelMatrix, scale);
	modelMatrix = glm::translate(modelMatrix, position);
    // -----------------------
	// Set model-view-projection matrix
	glm::mat4 mvp = cameraMatrix * modelMatrix;
	glUniformMatrix4fv(this->mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
	// TODO: Enable UV buffer and texture sampler
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->uvBufferID);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	// Set textureSampler to use texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->textureID);
	glUniform1i(this->textureSamplerID, 0); 
	// Draw the box
	glDrawElements(
		GL_TRIANGLES,      // mode
		36,    			   // number of indices
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Skybox::cleanup() {
	glDeleteBuffers(1, &this->vertexBufferID);
	glDeleteBuffers(1, &this->indexBufferID);
	glDeleteVertexArrays(1, &this->vertexArrayID);
	glDeleteBuffers(1, &this->uvBufferID);
	glDeleteTextures(1, &this->textureID);
	glDeleteProgram(this->shaderID);
}