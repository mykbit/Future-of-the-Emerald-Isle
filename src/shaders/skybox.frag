#version 330 core

in vec2 TexCoord;

// TODO: To add the texture sampler
uniform sampler2D ourTexture;

out vec3 finalColor;

void main()
{
	finalColor = texture(ourTexture, TexCoord).rgb;
}
