#version 330 core

// Inputs from vertex shader
in vec3 color;               
in vec3 worldPosition;       
in vec3 worldNormal;         
in vec4 worldPositionLightSpace;   

out vec4 finalColor;         

// Uniforms
uniform sampler2D shadowMap;
uniform vec3 lightPosition;  
uniform vec3 lightIntensity;

// Shadow calculation function
float ShadowCalculation()
{
    vec3 projCoords = worldPositionLightSpace.xyz / worldPositionLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r; // Closest depth from shadow map
    float currentDepth = projCoords.z; // Current fragment's depth

    float shadow = currentDepth > closestDepth ? 0.2 : 1.0; // Shadow factor
    return shadow;
}

void main()
{
    vec3 lightColor = vec3(1.0, 0.8, 0.4);

    // Diffuse lighting
    vec3 lightDir = normalize(lightPosition - worldPosition);
    vec3 normal = normalize(worldNormal);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * color;

    // Combine lighting with shadow factor
    vec3 lighting = ShadowCalculation() * diffuse * lightColor;
    finalColor = vec4(lighting, 1.0);
}
