#ifndef CUBECOLLIDER_H
#define CUBECOLLIDER_H

#include <glm/glm.hpp>
#include <vector>

class CubeCollider
{
  private:
  public:
    glm::vec3 position; // Position of the cube collider
    glm::vec3 size;     // Size of the cube collider

    std::vector<glm::vec3> topFace;
    std::vector<glm::vec3> bottomFace;
    std::vector<glm::vec3> leftFace;
    std::vector<glm::vec3> rightFace;
    std::vector<glm::vec3> frontFace;
    std::vector<glm::vec3> backFace;

    CubeCollider();

    // Check collision with another cube collider
    bool CheckCollision(const CubeCollider &other) const;
    glm::vec3 ResolveCollision(const CubeCollider &other);
    glm::vec3 CalculateCollisionNormal(const CubeCollider &other) const;
    void UpdateScale(glm::vec3 &size);

    // Getters and setters
    const glm::vec3 &getPosition() const;
    void setPosition(const glm::vec3 &newPosition);

    const glm::vec3 &getSize() const;
    void setSize(const glm::vec3 &newSize);

    const float &getOverlapX(const CubeCollider &other) const;
    const float &getOverlapY(const CubeCollider &other) const;
    const float &getOverlapZ(const CubeCollider &other) const;
};

#endif // CUBECOLLIDER_H
