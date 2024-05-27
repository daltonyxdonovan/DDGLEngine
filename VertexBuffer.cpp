
#include "VertexBuffer.h"
#include "Renderer.h"

VertexBuffer::VertexBuffer(float data[], unsigned int size)
{
    // Check the size parameter
    if (size <= 0)
    {
        std::cout << "ERROR: VERTEX BUFFER IS SMALLER THAN 1" << std::endl;
    }
    m_BufferData.insert(m_BufferData.end(), data, data + size);
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    // Set m_DataSize based on the provided size
    m_DataSize = size;
}

void VertexBuffer::UpdateBuffer(float data[], unsigned int size, unsigned int offset)
{
    // Bind the vertex array buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    // Update a portion of the buffer using glBufferSubData
    glBufferSubData(GL_ARRAY_BUFFER, offset * 4, size * 4, data); // 4 IS THE SIZE OF GL_FLOAT (idk caps lock lol)
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::UpdateBuffer(float data[], unsigned int size)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void VertexBuffer::UpdateChosen(int index, float chosen, int stride)
{
    // Calculate base offset for the cube data within the 'CornerPositions' array
    unsigned int baseOffset = index * 360; // Each cube has 240 floats
    float updatedData[1];

    updatedData[0] = 1;
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1));
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 10);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 20);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 30);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 40);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 50);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 60);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 70);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 80);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 90);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 100);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 110);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 120);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 130);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 140);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 150);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 160);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 170);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 180);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 190);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 200);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 210);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 220);
    UpdateBuffer(updatedData, 1, baseOffset + (stride - 1) + 230);
}

void VertexBuffer::UpdateTexture(int index, float textureID, int stride)
{
    // Calculate base offset for the cube data within the 'CornerPositions' array
    unsigned int baseOffset = index * 360; // Each cube has 240 floats
    float updatedData[1];

    updatedData[0] = textureID;
    UpdateBuffer(updatedData, 1, baseOffset + 5);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 10);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 20);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 30);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 40);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 50);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 60);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 70);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 80);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 90);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 100);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 110);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 120);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 130);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 140);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 150);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 160);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 170);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 180);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 190);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 200);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 210);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 220);
    UpdateBuffer(updatedData, 1, baseOffset + 5 + 230);
}

void VertexBuffer::AddVoxel(int index, int totalVoxelsAmount, int stride, float *cornerPositions)
{

    const int strideTotal = stride * 24;
    unsigned int baseOffset = totalVoxelsAmount * 360;
    float updatedData[240];

    for (int i = 0; i < 240; i++)
        updatedData[i] = cornerPositions[i];

    AddData(updatedData, 240);
}

void VertexBuffer::AddData(float data[], unsigned int size)
{
    unsigned int currentSize = m_DataSize * 240;
    unsigned int newSize = currentSize + size;

    // Check if we need to reallocate
    if (newSize * sizeof(float) > m_DataSize)
    {
        m_DataSize = newSize * sizeof(float);

        // Create a new buffer with the new size
        unsigned int newBufferID;
        glGenBuffers(1, &newBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, newBufferID);
        glBufferData(GL_ARRAY_BUFFER, m_DataSize, nullptr, GL_DYNAMIC_DRAW);

        // Copy existing data to the new buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, currentSize * sizeof(float), m_BufferData.data());

        // Delete the old buffer and update the Renderer ID
        glDeleteBuffers(1, &m_RendererID);
        m_RendererID = newBufferID;
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    // Add the new data to the buffer
    glBufferSubData(GL_ARRAY_BUFFER, currentSize * sizeof(float), size * sizeof(float), data);
}

void VertexBuffer::UpdateScale(int index, glm::vec3 positionOfCube, float xScale, float yScale, float zScale,
                               int stride)
{
    unsigned int baseOffset = index * 360; // Each cube has 240 floats
    float updatedData[3];

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 2);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 3);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 4);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 5);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 6);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 7);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 8);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 9);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 10);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 11);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 12);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 13);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 14);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 15);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 16);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 17);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 18);

    updatedData[0] = 1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 19);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 20);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = -1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 21);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = 1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 22);

    updatedData[0] = -1 * xScale + positionOfCube.x;
    updatedData[1] = -1 * yScale + positionOfCube.y;
    updatedData[2] = 1 * zScale + positionOfCube.z;
    UpdateBuffer(updatedData, 3, baseOffset + stride * 23);
}

void SetPosition(int index, float x, float y, float z, int stride)
{
}
