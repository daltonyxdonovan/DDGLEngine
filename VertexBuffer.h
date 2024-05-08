#pragma once
#include <vector>

class VertexBuffer
{
private:
    unsigned int m_RendererID;

public:
    VertexBuffer(float data[], unsigned int size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
    void UpdateBuffer(float data[], unsigned int size);

};

