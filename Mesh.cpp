#include "Mesh.h"

void Mesh::SetVertices(const std::vector<Vertex>& vertices) {
    this->vertices = vertices;
}

void Mesh::SetIndices(const std::vector<unsigned int>& indices) {
    this->indices = indices;
}

const std::vector<Vertex>& Mesh::GetVertices() const {
    return vertices;
}

const std::vector<unsigned int>& Mesh::GetIndices() const {
    return indices;
}

void Mesh::Clear() {
    vertices.clear();
    indices.clear();
}

void Mesh::SetRotation(const glm::vec3& rotation) {
    // Construct quaternions from the rotation angles
    glm::quat quaternionX = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat quaternionY = glm::angleAxis(glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat quaternionZ = glm::angleAxis(glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Combine the quaternions to get the final rotation quaternion
    glm::quat rotationQuaternion = quaternionX * quaternionY * quaternionZ;

    // Rotate each vertex using the rotation quaternion
    for (Vertex& vertex : vertices) {
        vertex.position = rotationQuaternion * vertex.position;
    }
}

bool Mesh::IsValidMesh() const {
    return !vertices.empty() && !indices.empty();
}

Mesh Mesh::CreateCube() {
    std::vector<Vertex> vertices = {
        // Front face
        Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f)),
        Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 0.0f)),
        // Back face
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f)),
        Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 1.0f)),
        Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 1.0f))
        // Left face
        
    };

    std::vector<unsigned int> indices = {
        // Front face
        0, 1, 2,
        2, 3, 0,
        // Back face
        4, 5, 6,
        6, 7, 4,
        // Left face
        0, 4, 7,
        7, 3, 0,
        // Right face
        1, 5, 6,
        6, 2, 1,
        // Top face
        3, 2, 6,
        6, 7, 3,
        // Bottom face
        0, 1, 5,
        5, 4, 0
    };

    //make each face a different color
    for (int i = 0; i < 8; i++) {
        vertices[i].r = rand()%155+100;
        vertices[i].g = rand()%155+100;
        vertices[i].b = rand()%155+100;
    }

    return Mesh(vertices, indices);
}

void Mesh::SetScale(const glm::vec3& scale) {
    for (Vertex& vertex : vertices) {
        vertex.position *= scale;
    }
}
