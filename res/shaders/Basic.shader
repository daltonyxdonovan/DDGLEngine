#shader vertex
#version 430 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float textureID;
layout(location = 3) in vec3 normal;
layout(location = 4) in float invis;

out vec2 v_TexCoord;
out float v_VertexY;
out float v_VertexX;
out float v_VertexZ;
out vec3 normalVec;
out vec2 offset;
out float invisible;


uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * position;
    v_VertexX = position.x;
    v_VertexZ = position.z;
    v_TexCoord = texCoord;
    v_VertexY = position.y;
    normalVec = normal;

    int textureINT = int(textureID);

    int column = int(textureINT%10);
    int row = int(textureINT/10);
    offset = vec2(column,row);
    invisible = invis;
}


#shader fragment
#version 430 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in float v_VertexY;
in float v_VertexX;
in float v_VertexZ;
in vec3 normalVec;
in vec2 offset;
in float invisible;

uniform sampler2D u_TextureAtlas;

uniform vec3 lightPos[256]; // Array of light positions, with a maximum of 100 lights
uniform vec4 lightColor[256]; // Array of light colors, with a maximum of 100 lights
uniform float lightBrightness[256]; // Array of light brightness values, with a maximum of 100 lights
uniform int numLights;     // Number of active lights

void main() {
    if (invisible != 0.0f)
    {
        discard;
    }
    
    
    vec4 texColor;
    texColor = texture(u_TextureAtlas, v_TexCoord + offset*.1);

    if (texColor.a < 0.1) // Optional: Discard almost fully transparent pixels to avoid artifacts
    {
        discard;
    }

    vec3 fragPos = vec3(v_VertexX, v_VertexY, v_VertexZ);
    vec3 normal = normalize(normalVec);
    vec3 ambient = vec3(0.1f); // Ambient lighting


    vec3 finalColor = vec3(0.0);
    
    for (int i = 0; i < numLights; i++) 
    {
        vec3 lightDir = normalize(lightPos[i] - fragPos); // Calculate the direction from the fragment to the light source
        float distance = length(lightPos[i] - fragPos); // Calculate the distance to the light source
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance)); // Attenuation formula

        float diff = max(dot(normal, lightDir), 0.0); // Calculate diffuse intensity
        vec3 diffuse = diff * vec3(lightColor[i]) * lightBrightness[i]; // Use light color and apply brightness
        finalColor += diffuse * attenuation; // Apply attenuation to diffuse lighting
    }

    finalColor = ambient + finalColor; // Add ambient light to the final color
    texColor.rgb *= finalColor; // Apply lighting to texture color
    
    color = texColor;
    
}