#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "static_model.h"

StaticModel::StaticModel(const char* modelPath, const char* vertexShader, const char* fragShader, glm::vec3 modelPos, glm::vec3 modelScale, float radians, glm::vec3 axisRot, glm::vec3 lightPosition, glm::vec3 lightIntensity) {
    // Load the model
	if (!loadModel(modelPath)) {
		return;
	}
	// Prepare buffers for rendering
	bindModel(model);
	this->radians = radians;
	this->axisRot = axisRot;
	this->modelPos = glm::translate(glm::mat4(1.0f), modelPos);
	this->modelRot = glm::rotate(glm::mat4(1.0f), glm::radians(radians), axisRot);
	this->modelScale = glm::scale(glm::mat4(1.0f), modelScale);
	this->modelMatrix = this->modelPos * this->modelRot * this->modelScale;
    this->lightPosition = lightPosition;
    this->lightIntensity = lightIntensity;
	// Load shaders
	shaderID = LoadShadersFromFile(vertexShader, fragShader);
	if (shaderID == 0) {
		cerr << "Failed to load shaders." << endl;
	}
	// Get handles for GLSL variables
	mvpMatrixID = glGetUniformLocation(shaderID, "MVP");
	lightPositionID = glGetUniformLocation(shaderID, "lightPosition");
	lightIntensityID = glGetUniformLocation(shaderID, "lightIntensity");
}

bool StaticModel::loadModel(const char *filename) {
	tinygltf::TinyGLTF loader;
	string err;
	string warn;
	bool res = loader.LoadASCIIFromFile(&this->model, &err, &warn, filename);
	if (!warn.empty()) {
		cout << "WARN: " << warn << endl;
	}
	if (!err.empty()) {
		cout << "ERR: " << err << endl;
	}
	if (!res)
		cout << "Failed to load glTF: " << filename << endl;
	else
		cout << "Loaded glTF: " << filename << endl;
	return res;
}

glm::mat4 StaticModel::getNodeTransform(const tinygltf::Node& node) {
	glm::mat4 transform(1.0f); 
	if (node.matrix.size() == 16) {
		transform = glm::make_mat4(node.matrix.data());
	} else {
		if (node.translation.size() == 3) {
			transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
		}
		if (node.rotation.size() == 4) {
			glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			transform *= glm::mat4_cast(q);
		}
		if (node.scale.size() == 3) {
			transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
		}
	}
	return transform;
}

void StaticModel::bindPrimitive(tinygltf::Model &model, StaticModel::Primitive &primitive, tinygltf::Primitive &prim_gltf) {
	// Create VAO
	glGenVertexArrays(1, &primitive.vao);
	glBindVertexArray(primitive.vao);
	// Bind vertex data
	tinygltf::Accessor positionAccessor = model.accessors[prim_gltf.attributes["POSITION"]];
	tinygltf::BufferView positionBufferView = model.bufferViews[positionAccessor.bufferView];
	glGenBuffers(1, &primitive.positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, primitive.positionVBO);
	glBufferData(GL_ARRAY_BUFFER, positionBufferView.byteLength, &model.buffers[positionBufferView.buffer].data.at(0) +positionBufferView.byteOffset, GL_STATIC_DRAW);
	// Bind normal data
	tinygltf::Accessor normalAccessor = model.accessors[prim_gltf.attributes["NORMAL"]];
	tinygltf::BufferView normalBufferView = model.bufferViews[normalAccessor.bufferView];
	glGenBuffers(1, &primitive.normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, primitive.normalVBO);
	glBufferData(GL_ARRAY_BUFFER, normalBufferView.byteLength, &model.buffers[normalBufferView.buffer].data.at(0) +normalBufferView.byteOffset, GL_STATIC_DRAW);
	// Bind texture coordinate data
	tinygltf::Accessor texCoordAccessor = model.accessors[prim_gltf.attributes["TEXCOORD_0"]];
	tinygltf::BufferView texCoordBufferView = model.bufferViews[texCoordAccessor.bufferView];
	glGenBuffers(1, &primitive.texcoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, primitive.texcoordVBO);
	glBufferData(GL_ARRAY_BUFFER, texCoordBufferView.byteLength, &model.buffers[texCoordBufferView.buffer].data.at(0) +texCoordBufferView.byteOffset, GL_STATIC_DRAW);
	// Bind index data
	tinygltf::Accessor indexAccessor = model.accessors[prim_gltf.indices];
	tinygltf::BufferView indexBufferView = model.bufferViews[indexAccessor.bufferView];
	glGenBuffers(1, &primitive.indexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.indexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferView.byteLength, &model.buffers[indexBufferView.buffer].data.at(0) +indexBufferView.byteOffset, GL_STATIC_DRAW);
	// Bind texture
	if (model.textures.size() > 0) {
      // fixme: Use material's baseColor
      tinygltf::Texture &tex = model.textures[1];
      if (tex.source > -1) {
        GLuint texid;
        glGenTextures(1, &texid);
        tinygltf::Image &image = model.images[tex.source];
        glBindTexture(GL_TEXTURE_2D, texid);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        GLenum format = GL_RGBA;
        if (image.component == 1) {
          format = GL_RED;
        } else if (image.component == 2) {
          format = GL_RG;
        } else if (image.component == 3) {
          format = GL_RGB;
        } else {
          // ???
        }
        GLenum type = GL_UNSIGNED_BYTE;
        if (image.bits == 8) {
          // ok
        } else if (image.bits == 16) {
          type = GL_UNSIGNED_SHORT;
        } else {
          // ???
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0,
                     format, type, &image.image.at(0));
      }
    }
}
void StaticModel::bindMesh(tinygltf::Model &model, tinygltf::Mesh &mesh, vector<StaticModel::Primitive> &primitives) {
	for (size_t i = 0; i < mesh.primitives.size(); i++) {
		StaticModel::Primitive primitive;
		tinygltf::Primitive prim_gltf = mesh.primitives[i];
		bindPrimitive(model, primitive, prim_gltf);
		primitives.push_back(primitive);
	}
}
void StaticModel::bindModel(tinygltf::Model &model) {
	for (size_t i = 0; i < model.meshes.size(); i++) {
		vector<StaticModel::Primitive> primitives;
		tinygltf::Mesh mesh = model.meshes[i];
		bindMesh(model, mesh, primitives);
		this->primitiveObjects.push_back(primitives);
	}
}
void StaticModel::drawPrimitives(vector<StaticModel::Primitive> &primitives, tinygltf::Model &model, tinygltf::Mesh &mesh) {
	for (size_t i = 0; i < mesh.primitives.size(); i++) 
	{
		glBindVertexArray(primitives[i].vao);
		
		// Bind vertex data
		tinygltf::Accessor positionAccessor = model.accessors[mesh.primitives[i].attributes["POSITION"]];
		tinygltf::BufferView positionBufferView = model.bufferViews[positionAccessor.bufferView];
		glEnableVertexAttribArray(0);
		glBindBuffer(positionBufferView.target, primitives[i].positionVBO);
		glVertexAttribPointer(0, positionAccessor.type, positionAccessor.componentType, positionAccessor.normalized ? GL_TRUE :GL_FALSE, positionBufferView.byteStride, BUFFER_OFFSET(positionAccessor.byteOffset));
		
		// Bind normal data
		tinygltf::Accessor normalAccessor = model.accessors[mesh.primitives[i].attributes["NORMAL"]];
		tinygltf::BufferView normalBufferView = model.bufferViews[normalAccessor.bufferView];
		glEnableVertexAttribArray(1);
		glBindBuffer(normalBufferView.target, primitives[i].normalVBO);
		glVertexAttribPointer(1, normalAccessor.type, normalAccessor.componentType, normalAccessor.normalized ? GL_TRUE :GL_FALSE, normalBufferView.byteStride, BUFFER_OFFSET(normalAccessor.byteOffset));
		// Bind texture coordinate data	
		tinygltf::Accessor texCoordAccessor = model.accessors[mesh.primitives[i].attributes["TEXCOORD_0"]];
		tinygltf::BufferView texCoordBufferView = model.bufferViews[texCoordAccessor.bufferView];
		glEnableVertexAttribArray(2);
		glBindBuffer(texCoordBufferView.target, primitives[i].texcoordVBO);
		glVertexAttribPointer(2, texCoordAccessor.type, texCoordAccessor.componentType, texCoordAccessor.normalized ? GL_TRUE :GL_FALSE, texCoordBufferView.byteStride, BUFFER_OFFSET(texCoordAccessor.byteOffset));
		// Bind index data
		tinygltf::Accessor indexAccessor = model.accessors[mesh.primitives[i].indices];
		tinygltf::BufferView indexBufferView = model.bufferViews[indexAccessor.bufferView];
		glBindBuffer(indexBufferView.target, primitives[i].indexVBO);
		// Bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, primitives[i].texID);
		// Draw the primitive
		glDrawElements(GL_TRIANGLES, indexAccessor.count, indexAccessor.componentType, BUFFER_OFFSET(indexAccessor.byteOffset));
		glBindVertexArray(0);
	}
}
void StaticModel::drawModelNodes(tinygltf::Model &model, tinygltf::Node &node, glm::mat4 vp, glm::mat4 parentTransform) {
	glm::mat4 globalTransform = parentTransform * getNodeTransform(node);
	if (node.mesh >= 0 && node.mesh < model.meshes.size()) {
		glUseProgram(shaderID);
		glm::mat4 mvp = vp * this->modelMatrix * globalTransform;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
		drawPrimitives(this->primitiveObjects[node.mesh], model, model.meshes[node.mesh]);
	}
	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(model, model.nodes[node.children[i]], vp, globalTransform);
	}
}
void StaticModel::render(glm::mat4 vp) {
	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); i++) {
		tinygltf::Node &node = model.nodes[scene.nodes[i]];
		drawModelNodes(model, node, vp, glm::mat4(1.0f));
	}
}
void StaticModel::cleanup() {
	glDeleteProgram(shaderID);
}