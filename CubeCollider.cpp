#include "CubeCollider.h"
#include <algorithm>

CubeCollider::CubeCollider() : position(0, 3, 0), size(1, 1, 1) 
{
    topFace = 
    {
        glm::vec3{position.x+size.x,position.y+size.y,position.z+size.z},
        glm::vec3{position.x+size.x,position.y+size.y,position.z-size.z},
        glm::vec3{position.x-size.x,position.y+size.y,position.z+size.z},
        glm::vec3{position.x-size.x,position.y+size.y,position.z-size.z}
    };

    bottomFace = 
    {
        glm::vec3{position.x+size.x,position.y-size.y,position.z+size.z},
        glm::vec3{position.x+size.x,position.y-size.y,position.z-size.z},
        glm::vec3{position.x-size.x,position.y-size.y,position.z+size.z},
        glm::vec3{position.x-size.x,position.y-size.y,position.z-size.z}
    };

    leftFace = 
    {
        glm::vec3{position.x-size.x,position.y+size.y,position.z+size.z},
        glm::vec3{position.x-size.x,position.y+size.y,position.z-size.z},
        glm::vec3{position.x-size.x,position.y-size.y,position.z+size.z},
        glm::vec3{position.x-size.x,position.y-size.y,position.z-size.z}
    };

    rightFace = 
    {
        glm::vec3{position.x+size.x,position.y+size.y,position.z+size.z},
        glm::vec3{position.x+size.x,position.y+size.y,position.z-size.z},
        glm::vec3{position.x+size.x,position.y-size.y,position.z+size.z},
        glm::vec3{position.x+size.x,position.y-size.y,position.z-size.z}
    };

    frontFace = 
    {
        glm::vec3{position.x+size.x,position.y+size.y,position.z+size.z},
        glm::vec3{position.x+size.x,position.y-size.y,position.z+size.z},
        glm::vec3{position.x-size.x,position.y+size.y,position.z+size.z},
        glm::vec3{position.x-size.x,position.y-size.y,position.z+size.z}
    };

    backFace = 
    {
        glm::vec3{position.x+size.x,position.y+size.y,position.z-size.z},
        glm::vec3{position.x+size.x,position.y-size.y,position.z-size.z},
        glm::vec3{position.x-size.x,position.y+size.y,position.z-size.z},
        glm::vec3{position.x-size.x,position.y-size.y,position.z-size.z}
    };
    



};

bool CubeCollider::CheckCollision(const CubeCollider& other) const {
    // Calculate min and max points for both colliders
    glm::vec3 minA = position - size / 2.0f;
    glm::vec3 maxA = position + size / 2.0f;
    glm::vec3 minB = other.position - other.size / 2.0f;
    glm::vec3 maxB = other.position + other.size / 2.0f;

    // Check for overlap on all axes
    if (maxA.x < minB.x || minA.x > maxB.x) return false;
    if (maxA.y < minB.y || minA.y > maxB.y) return false;
    if (maxA.z < minB.z || minA.z > maxB.z) return false;

    // If no axis has overlap, then there is a collision
    return true;
}

glm::vec3 CubeCollider::ResolveCollision(const CubeCollider& other) {
    glm::vec3 mtv = glm::vec3(0.0f);
    glm::vec3 overlap = glm::vec3(0.0f);

    glm::vec3 minA = position - size / 2.0f;
    glm::vec3 maxA = position + size / 2.0f;
    glm::vec3 minB = other.position - other.size / 2.0f;
    glm::vec3 maxB = other.position + other.size / 2.0f;

    // Calculate overlap on each axis
    overlap.x = std::min(maxA.x - minB.x, maxB.x - minA.x);
    overlap.y = std::min(maxA.y - minB.y, maxB.y - minA.y);
    overlap.z = std::min(maxA.z - minB.z, maxB.z - minA.z);

    // Find the axis with the smallest overlap
    if (overlap.x < overlap.y && overlap.x < overlap.z) {
        mtv.x = overlap.x * ((position.x < other.position.x) ? -1.0f : 1.0f);
    } else if (overlap.y < overlap.x && overlap.y < overlap.z) {
        mtv.y = overlap.y * ((position.y < other.position.y) ? -1.0f : 1.0f);
    } else {
        mtv.z = overlap.z * ((position.z < other.position.z) ? -1.0f : 1.0f);
    }

    // Move the collider out of collision
    return mtv;
}

const float& CubeCollider::getOverlapX(const CubeCollider& other) const {
    glm::vec3 minA = position - size / 2.0f;
    glm::vec3 maxA = position + size / 2.0f;
    glm::vec3 minB = other.position - other.size / 2.0f;
    glm::vec3 maxB = other.position + other.size / 2.0f;

    return std::min(maxA.x - minB.x, maxB.x - minA.x);
}

const float& CubeCollider::getOverlapY(const CubeCollider& other) const {
    glm::vec3 minA = position - size / 2.0f;
    glm::vec3 maxA = position + size / 2.0f;
    glm::vec3 minB = other.position - other.size / 2.0f;
    glm::vec3 maxB = other.position + other.size / 2.0f;

    return std::min(maxA.y - minB.y, maxB.y - minA.y);
}

const float& CubeCollider::getOverlapZ(const CubeCollider& other) const {
    glm::vec3 minA = position - size / 2.0f;
    glm::vec3 maxA = position + size / 2.0f;
    glm::vec3 minB = other.position - other.size / 2.0f;
    glm::vec3 maxB = other.position + other.size / 2.0f;

    return std::min(maxA.z - minB.z, maxB.z - minA.z);
}

const glm::vec3& CubeCollider::getPosition() const {
    return position;
}

void CubeCollider::setPosition(const glm::vec3& newPosition) {
    position = newPosition;
}

const glm::vec3& CubeCollider::getSize() const {
    return size;
}

void CubeCollider::setSize(const glm::vec3& newSize) {
    size = newSize;
}
