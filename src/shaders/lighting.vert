#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMat;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 VP;
uniform mat4 model;

uniform bool reverse_normals;

void main()
{
    FragPos = vec3((aInstanceMat * model) * vec4(aPos, 1.0));

    mat3 normalMatrix = transpose(inverse(mat3(aInstanceMat * model)));
    Normal = reverse_normals ? normalMatrix * (-aNormal) : normalMatrix * aNormal;


    TexCoords = aTexCoords;
    gl_Position = (VP * (aInstanceMat * model)) * vec4(aPos, 1.0);
}
