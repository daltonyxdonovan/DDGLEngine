#pragma once
#include <vector>

class IndexBuffer
{
  private:
    unsigned int m_Count;

  public:
    unsigned int m_RendererID;

    IndexBuffer(unsigned int data[], unsigned int count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;
    void UpdateBuffer(unsigned int data[], unsigned int count);
    void UpdateBuffer(unsigned int data[], unsigned int count, unsigned int offset);

    inline unsigned int GetCount() const
    {
        return m_Count;
    }
};
