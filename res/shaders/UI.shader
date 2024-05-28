#shader vertex
#version 430 core

layout(location = 0) in vec4 position; // 2D position in the form of vec4 (x, y, 0, 1)
layout(location = 1) in vec2 texCoord; // Texture coordinates
layout(location = 2) in float textureID; // ID for the texture (assuming it's still relevant)

out vec2 v_TexCoord;
out vec2 offset;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * position; // Position transformation for 2D space
    v_TexCoord = texCoord;

    int textureINT = int(textureID);

    int column = int(textureINT % 10);
    int row = int(textureINT / 10);
    offset = vec2(column, row);
}


#shader fragment
#version 430 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in float v_VertexY;
in float v_VertexX;
in float v_VertexZ;
in vec2 offset;

uniform sampler2D u_Heart0;


void main() {
    
    
    vec4 texColor;
    texColor = texture(u_Heart0, v_TexCoord + offset*.1);

    color = texColor;
    
}