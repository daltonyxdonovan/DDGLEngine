#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float textureID;
layout(location = 3) in vec3 normal;
layout(location = 4) in float chosen;

out vec2 v_TexCoord;
out float v_VertexY;
out float v_VertexX;
out float v_VertexZ;
out float textureIDint;
out vec3 normalVec;
out float chosen2;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * position;
    v_VertexX = position.x;
    v_VertexZ = position.z;
    v_TexCoord = texCoord;
    v_VertexY = position.y;

    textureIDint = textureID;
    normalVec = normal;
    chosen2 = chosen;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in float v_VertexY;
in float v_VertexX;
in float v_VertexZ;
in float textureIDint;
in vec3 normalVec;
in float chosen2;

uniform sampler2D u_TextureBrick1;
uniform sampler2D u_TextureBrick2;
uniform sampler2D u_TextureBrick3;
uniform sampler2D u_TextureBrick4;

uniform vec3 lightPos[10]; // Array of light positions, with a maximum of 10 lights
uniform vec4 lightColor[10]; // Array of light colors, with a maximum of 10 lights
uniform int numLights;     // Number of active lights

void main()
{
    vec4 texColor;

    if (textureIDint <= 0.0f)
        texColor = texture(u_TextureBrick1, v_TexCoord);
    else if (textureIDint <= 1.0f)
        texColor = texture(u_TextureBrick2, v_TexCoord);
    else if (textureIDint <= 2.0f)
        texColor = texture(u_TextureBrick3, v_TexCoord);
    else if (textureIDint <= 3.0f)
        texColor = texture(u_TextureBrick4, v_TexCoord);

    vec3 fragPos = vec3(v_VertexX, v_VertexY, v_VertexZ);
    vec3 normal = normalize(normalVec);
    vec3 ambient = vec3(0.1); // Ambient lighting

    vec3 finalColor = vec3(0.0);

    for (int i = 0; i < numLights; i++) {
        vec3 lightDir = normalize(lightPos[i] - fragPos); // Calculate the direction from the fragment to the light source
        float distance = length(lightPos[i] - fragPos); // Calculate the distance to the light source
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance)); // Attenuation formula

        float diff = max(dot(normal, lightDir), 0.0); // Calculate diffuse intensity
        vec3 diffuse = diff * vec3(lightColor[i]); // Use light color
        finalColor += diffuse * attenuation; // Apply attenuation to diffuse lighting
    }

    finalColor = ambient + finalColor; // Add ambient light to the final color
    texColor.rgb *= finalColor; // Apply lighting to texture color

    color = texColor;
}
