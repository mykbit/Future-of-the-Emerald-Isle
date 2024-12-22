#version 330 core

in vec3 worldPosition;
in vec3 worldNormal; 
in vec2 texCoord;

out vec3 finalColor;

uniform sampler2D tex;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;

void main()
{
	finalColor = texture(tex, texCoord).rgb;
}
