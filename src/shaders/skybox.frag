#version 330 core

in vec3 color;

// TODO: To add UV input to this fragment shader 
in vec2 TexCoord;

// TODO: To add the texture sampler
uniform sampler2D ourTexture;

out vec3 finalColor;

void main()
{
	// TODO: texture lookup. 
	finalColor = texture(ourTexture, TexCoord).rgb;
}
