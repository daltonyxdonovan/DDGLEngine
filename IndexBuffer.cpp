
#include "IndexBuffer.h"
#include "Renderer.h"

IndexBuffer::IndexBuffer(unsigned int data[], unsigned int count) : m_Count(count)
{
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void IndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::UpdateBuffer(unsigned int data[], unsigned int count)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
    m_Count = count;
}

void IndexBuffer::UpdateBuffer(unsigned int data[], unsigned int count, unsigned int offset)
{
    // Bind the element array buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);

    // Check if offset is within valid range
    if (offset + count > m_Count)
    {
        // Handle error: attempting to update beyond buffer size
        // You can throw an exception, log an error message, or clamp the update range.
        return;
    }

    // Update a portion of the buffer using glBufferSubData
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(unsigned int), count * sizeof(unsigned int), data);
}
