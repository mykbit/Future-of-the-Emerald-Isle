#include "glm/detail/type_mat.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "static_model.h"

StaticModel::StaticModel(const char* modelPath, glm::mat4* modelMatrices, int amount) {
    // Load the model
	if (!loadModel(modelPath)) {
		return;
	}
	this->modelMatrices = modelMatrices;
	this->amount = amount;

	// Prepare buffers for rendering
	bindModel(model);
}

bool StaticModel::loadModel(const char *filename) {
	tinygltf::TinyGLTF loader;
	bool res;
	string err;
	string warn;
	// check if filename has .glb extension
	if (strstr(filename, ".glb") != NULL) {
		res = loader.LoadBinaryFromFile(&this->model, &err, &warn, filename);
	} else {
		res = loader.LoadASCIIFromFile(&this->model, &err, &warn, filename);
	}
	
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
	// Bind transform data
	glGenBuffers(1, &primitive.transformVBO);
	glBindBuffer(GL_ARRAY_BUFFER, primitive.transformVBO);
	glBufferData(GL_ARRAY_BUFFER, this->amount * sizeof(glm::mat4), &this->modelMatrices[0], GL_STATIC_DRAW);
	// Bind texture
	if (model.textures.size() > 0) {
      	// fixme: Use material's baseColor
	  	tinygltf::Material &material = model.materials[prim_gltf.material];
      	tinygltf::Parameter parameter = material.values["baseColorTexture"];
      	tinygltf::Texture &tex = model.textures[parameter.TextureIndex()];
      	if (tex.source > -1) {
      	  	glGenTextures(1, &primitive.texID);
      	  	tinygltf::Image &image = model.images[tex.source];
      	  	glBindTexture(GL_TEXTURE_2D, primitive.texID);
      	  	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      	  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
      		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
      	  	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, format, type, &image.image.at(0));
			glGenerateMipmap(GL_TEXTURE_2D);
      	}
    } else if (model.materials[prim_gltf.material].values.find("baseColorFactor") != model.materials[prim_gltf.material].values.end()) {
        // Fallback: Use baseColorFactor as texture
        tinygltf::Parameter parameter = model.materials[prim_gltf.material].values["baseColorFactor"];
        glm::vec4 baseColorFactor = glm::make_vec4(parameter.ColorFactor().data());
        GLubyte data[4] = {
            static_cast<GLubyte>(baseColorFactor.r * 255),
            static_cast<GLubyte>(baseColorFactor.g * 255),
            static_cast<GLubyte>(baseColorFactor.b * 255),
            static_cast<GLubyte>(baseColorFactor.a * 255)};
        
        glGenTextures(1, &primitive.texID);
        glBindTexture(GL_TEXTURE_2D, primitive.texID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else {
        // Handle case where neither texture nor baseColorFactor is provided
        glGenTextures(1, &primitive.texID);
        glBindTexture(GL_TEXTURE_2D, primitive.texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
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
		// Bind transform data
		glBindBuffer(GL_ARRAY_BUFFER, primitives[i].transformVBO);
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
		// Bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, primitives[i].texID);
		// Draw the primitive
		glDrawElementsInstanced(GL_TRIANGLES, indexAccessor.count, indexAccessor.componentType, BUFFER_OFFSET(indexAccessor.byteOffset), this->amount);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);
		glDisableVertexAttribArray(6);
		glBindVertexArray(0);
	}
}

void StaticModel::drawModelNodes(tinygltf::Model &model, tinygltf::Node &node, glm::mat4 vp, glm::mat4 parentTransform, Shader& shader) {
	glm::mat4 globalTransform = parentTransform * getNodeTransform(node);
	if (node.mesh >= 0 && node.mesh < model.meshes.size()) {
		shader.setMat4("model", globalTransform);
		drawPrimitives(this->primitiveObjects[node.mesh], model, model.meshes[node.mesh]);
	}
	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(model, model.nodes[node.children[i]], vp, globalTransform, shader);
	}
}

void StaticModel::render(glm::mat4 vp, Shader& shader, glm::mat4 transform) {
	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); i++) {
		tinygltf::Node &node = model.nodes[scene.nodes[i]];
		drawModelNodes(model, node, vp, transform, shader);
	}
}