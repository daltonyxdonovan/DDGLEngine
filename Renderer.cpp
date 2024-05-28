#include "Renderer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include <iostream>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

bool GLLogCall(const char *function, const char *file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ")" << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

void Renderer::Clear() const
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Draw(const VertexArray &va, const IndexBuffer &ib, const Shader &shader) const
{
    shader.Bind();
    va.Bind();
    ib.Bind();

    glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::DrawLine(const glm::vec2 &start, const glm::vec2 &end, const glm::vec4 &color) const
{
    // Define the vertices for the line
    float vertices[4] = {start.x, start.y, end.x, end.y};

    // Create a vertex buffer with the line vertices
    VertexBuffer vb(vertices, 2 * sizeof(glm::vec2));

    // Define the layout of the vertex buffer
    VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 2); // position

    // Create a vertex array and bind the vertex buffer to it
    VertexArray va(1);
    va.AddBuffer(vb, layout);

    // Set the line color using OpenGL's immediate mode
    glColor4f(color.r, color.g, color.b, color.a);

    // Draw the line using immediate mode
    glBegin(GL_LINES);
    glVertex2f(start.x, start.y);
    glVertex2f(end.x, end.y);
    glEnd();
}
