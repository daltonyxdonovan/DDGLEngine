#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float textureID;

out vec2 v_TexCoord;
out float v_VertexY;
out float textureIDint;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * position;
    v_TexCoord = texCoord;
    v_VertexY = position.y;

    textureIDint = textureID;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in float v_VertexY;
in float textureIDint;

uniform sampler2D u_TextureBrick1;
uniform sampler2D u_TextureBrick2;
uniform sampler2D u_TextureBrick3;
uniform sampler2D u_TextureBrick4;

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

    color = texColor;
}