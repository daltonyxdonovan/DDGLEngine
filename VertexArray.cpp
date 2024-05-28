#include "VertexArray.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"
#include <vector>

#include <iostream>

VertexArray::VertexArray(int VAO)
{
    glGenVertexArrays(VAO, &m_RendererID);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(1);
}

void VertexArray::AddBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout)
{
    Bind();
    vb.Bind();
    const auto &elements = layout.GetElements();
    GLintptr offset = 0;
    for (unsigned int i = 0; i < elements.size(); i++)
    {
        const auto &element = elements[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(),
                              reinterpret_cast<const void *>(offset));
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
    }
}

void VertexArray::UpdateBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout)
{
    Bind();
    vb.Bind();
    const auto &elements = layout.GetElements();
    GLintptr offset = 0;
    for (unsigned int i = 0; i < elements.size(); i++)
    {
        const auto &element = elements[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(),
                              reinterpret_cast<const void *>(offset));
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
    }
}
