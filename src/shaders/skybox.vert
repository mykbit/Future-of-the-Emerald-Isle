#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 aTexCoord;

// TODO: To add UV to this vertex shader 
out vec2 TexCoord;

// Matrix for vertex transformation
uniform mat4 VP;
uniform mat4 model;

void main() {
    // Transform vertex
    gl_Position = VP * model * vec4(vertexPosition, 1);

    // TODO: Pass UV to the fragment shader    
    TexCoord = aTexCoord;
}
