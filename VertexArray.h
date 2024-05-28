#pragma once
#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray
{

  private:
    unsigned int m_RendererID;

  public:
    VertexArray(int VAO);
    ~VertexArray();

    void AddBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout);
    void Bind() const;
    void Unbind() const;
    void UpdateBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout);
};
