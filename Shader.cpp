#include <GL/glew.h>
#include "Shader.h"
#include "Renderer.h"
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>




Shader::Shader(const std::string& filepath)
    : m_FilePath(filepath), m_RendererID(0)
{
    // Create a shader program
    ShaderProgramSource source = ParseShader(filepath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
    unsigned int fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

    // Attach the shaders to the program
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    // Link the program
    glLinkProgram(program);

    // Validate the program
    glValidateProgram(program);

    // Delete the shaders
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

ShaderProgramSource Shader::ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        //std::cout << line << std::endl;
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                std::cout << "Found vertex shader!" << std::endl;

                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                std::cout << "Found fragment shader!" << std::endl;

                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    int location = GetUniformLocation(name);
    GLCall(glUniform1f(location, value));
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    int location = GetUniformLocation(name);
    GLCall(glUniform1i(location, value));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    int location = GetUniformLocation(name);
    GLCall(glUniform3f(location, v0, v1, v2));
}


unsigned int Shader::CompileShader(const std::string& source, unsigned int type)
{
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    int location = GetUniformLocation(name);
    GLCall(glUniform4f(location, v0, v1, v2, v3));
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];
    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1)
    {
        std::cout << "Warning: uniform '" << name << "' doesn't exist! (this normally means you haven't used the var inside the shader!)" << std::endl;
    }
        m_UniformLocationCache[name] = location;
    return location;
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    int location = GetUniformLocation(name);
    GLCall(glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]));
}