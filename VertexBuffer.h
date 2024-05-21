#pragma once
#include <glm/glm.hpp>
#include <vector>

class VertexBuffer
{
  private:
    unsigned int m_RendererID;
    unsigned int m_DataSize;
    std::vector<float> m_BufferData;

  public:
    VertexBuffer(float data[], unsigned int size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
    void UpdateBuffer(float data[], unsigned int size);
    void UpdateBuffer(float data[], unsigned int size, unsigned int offset);
    void UpdateChosen(int index, float chosen, int stride);
    void UpdateScale(int index, glm::vec3 positionOfCube, float xScale, float yScale, float zScale, int stride);
    void SetPosition(int index, float x, float y, float z, int stride);
    void AddVoxel(int index, int totalVoxelsAmount, int stride, float *cornerPositions);
    void UpdateTexture(int index, float textureID, int stride);
    void AddData(float data[], unsigned int size);
};
