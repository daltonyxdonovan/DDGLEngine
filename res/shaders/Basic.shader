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
out float cursor;


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
    cursor = 0;
    if (textureID > 98)
        cursor = 1;
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

uniform vec3 lightPos[256]; 
uniform vec4 lightColor[256];
uniform float lightBrightness[256];
uniform int numLights;
uniform float ambientLight;
in float cursor;


void main() {
    if (invisible != 0.0f)
    {
        discard;
    }
    
    
    vec4 texColor;
    texColor = texture(u_TextureAtlas, v_TexCoord + offset*.1);

    if (texColor.a < 0.1) 
    {
        discard;
    }
    if (cursor < 1)
    {
        vec3 fragPos = vec3(v_VertexX, v_VertexY, v_VertexZ);

        // Check if the fragment position matches any light positions
        for (int i = 0; i < numLights; i++) {
            if (distance(fragPos, lightPos[i]) < 2) { // Adjust the threshold as needed
                color = vec4(1.0, 1.0, 1.0, 1.0); // White color for light positions
                return;
            }
        }

        vec3 normal = normalize(normalVec);
        vec3 ambient = vec3(ambientLight);

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
    }
    color = texColor;
    
}