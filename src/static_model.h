#ifndef STATIC_MODEL_CLASS_H
#define STATIC_MODEL_CLASS_H

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#include "glm/detail/type_mat.hpp"
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
        GLuint mvpMatrixID;
        GLuint lightPositionID;
        GLuint lightIntensityID;
        GLuint shaderID;

        // Model data
        tinygltf::Model model;
        glm::mat4 modelMatrix;
        glm::mat4 modelRot;
        float radians;
        glm::vec3 axisRot;
        glm::mat4 modelPos;
        glm::mat4 modelScale;

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
        };
        vector<vector<Primitive>> primitiveObjects;

        StaticModel(const char* modelPath, const char* vertexShader, const char* fragShader, glm::vec3 modelPos, glm::vec3 modelScale, float radians, glm::vec3 axisRot, glm::vec3 lightPosition, glm::vec3 lightIntensity);
        bool loadModel(const char *filename);
        glm::mat4 getNodeTransform(const tinygltf::Node& node);
        void bindPrimitive(tinygltf::Model &model, Primitive &primitive, tinygltf::Primitive &prim_gltf);
        void bindMesh(tinygltf::Model &model, tinygltf::Mesh &mesh, vector<Primitive> &primitives);
        void bindModel(tinygltf::Model &model);
        void drawPrimitives(vector<Primitive> &primitives, tinygltf::Model &model, tinygltf::Mesh &mesh);
        void drawModelNodes(tinygltf::Model &model, tinygltf::Node &node, glm::mat4 vp, glm::mat4 parentTransform);
        void render(glm::mat4 cameraMatrix);
        void cleanup();
};

#endif