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
uniform mat4 u_Model; // Model matrix to transform normal and position
uniform mat4 u_View;  // View matrix to get the view position

out vec3 fragPos;    // Fragment position in world space
out vec3 viewPos;    // View position in world space

out float cursor;

void main()
{
    gl_Position = u_MVP * position;
    vec4 worldPos = u_Model * position;
    fragPos = worldPos.xyz;

    v_VertexX = worldPos.x;
    v_VertexZ = worldPos.z;
    v_VertexY = worldPos.y;
    v_TexCoord = texCoord;
    normalVec = mat3(transpose(inverse(u_Model))) * normal; // Correct normal transformation

    viewPos = vec3(inverse(u_View) * vec4(0.0, 0.0, 0.0, 1.0)); // Get view position from the inverse of view matrix

    int textureINT = int(textureID);
    int column = int(textureINT % 10);
    int row = int(textureINT / 10);
    offset = vec2(column, row);
    invisible = invis;
    cursor = (textureID > 98) ? 1 : 0;
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

in vec3 fragPos; // Fragment position from vertex shader
in vec3 viewPos; // View position from vertex shader

void main() {
    if (invisible != 0.0f) {
        discard;
    }
    
    vec4 texColor;
    texColor = texture(u_TextureAtlas, v_TexCoord + offset * 0.1);

    if (texColor.a < 0.1) {
        discard;
    }

    if (cursor < 1) {
        vec3 normal = normalize(normalVec);
        vec3 ambient = vec3(ambientLight);
        vec3 viewDir = normalize(viewPos - fragPos);

        vec3 finalColor = vec3(0.0);

        for (int i = 0; i < numLights; i++) {
            vec3 lightDir = normalize(lightPos[i] - fragPos); // Direction from the fragment to the light source
            float distance = length(lightPos[i] - fragPos); // Distance to the light source
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance)); // Attenuation formula

            // Diffuse component
            float diff = max(dot(normal, lightDir), 0.0);
            vec3 diffuse = diff * vec3(lightColor[i]) * lightBrightness[i];

            // Specular component
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(normal, halfwayDir), 0.0), 256.0); // Shininess factor of 32
            vec3 specular = spec * vec3(lightColor[i]) * lightBrightness[i];

            finalColor += (diffuse + specular) * attenuation; // Apply attenuation to both diffuse and specular
        }

        finalColor = ambient + finalColor; // Add ambient light to the final color
        texColor.rgb *= finalColor; // Apply lighting to texture color
    }
    color = texColor;
}
