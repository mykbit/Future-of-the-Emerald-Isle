#ifndef STATIC_MODEL_CLASS_H
#define STATIC_MODEL_CLASS_H

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "tiny_gltf.h"
#include <render/shader.h>

using namespace std;

class StaticModel {
    public:
        // Shader variable IDs
        glm::mat4* modelMatrices;
        int amount;

        // Model data
        tinygltf::Model model;

        // Lighting
        glm::vec3 lightPosition;
        glm::vec3 lightIntensity;

        struct Primitive {
            GLuint vao;
            GLuint positionVBO;
            GLuint normalVBO;
            GLuint indexVBO;
            GLuint texcoordVBO;
            GLuint texID;
            GLuint transformVBO;
        };
        vector<vector<Primitive>> primitiveObjects;

        StaticModel(const char* modelPath, glm::mat4* modelMatrices, int amount);
        bool loadModel(const char *filename);
        glm::mat4 getNodeTransform(const tinygltf::Node& node);
        void bindPrimitive(tinygltf::Model &model, Primitive &primitive, tinygltf::Primitive &prim_gltf);
        void bindMesh(tinygltf::Model &model, tinygltf::Mesh &mesh, vector<Primitive> &primitives);
        void bindModel(tinygltf::Model &model);
        void drawPrimitives(vector<Primitive> &primitives, tinygltf::Model &model, tinygltf::Mesh &mesh);
        void drawDepthPrimitives(vector<Primitive> &primitives, tinygltf::Model &model, tinygltf::Mesh &mesh);
        void drawModelNodes(tinygltf::Model &model, tinygltf::Node &node, glm::mat4 vp, glm::mat4 parentTransform, Shader& shader);
        void render(glm::mat4 cameraMatrix, Shader& shader, glm::mat4 transform = glm::mat4(1.0f));
        void cleanup();
};

#endif