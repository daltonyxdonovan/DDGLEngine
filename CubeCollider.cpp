#include "CubeCollider.h"
#include <algorithm>
#include <iostream>

CubeCollider::CubeCollider() : position(0, 0, 0), size(1, 1, 1)
{
    topFace = {glm::vec3{position.x + size.x, position.y + size.y, position.z + size.z},
               glm::vec3{position.x + size.x, position.y + size.y, position.z - size.z},
               glm::vec3{position.x - size.x, position.y + size.y, position.z + size.z},
               glm::vec3{position.x - size.x, position.y + size.y, position.z - size.z}};

    bottomFace = {glm::vec3{position.x + size.x, position.y - size.y, position.z + size.z},
                  glm::vec3{position.x + size.x, position.y - size.y, position.z - size.z},
                  glm::vec3{position.x - size.x, position.y - size.y, position.z + size.z},
                  glm::vec3{position.x - size.x, position.y - size.y, position.z - size.z}};

    leftFace = {glm::vec3{position.x - size.x, position.y + size.y, position.z + size.z},
                glm::vec3{position.x - size.x, position.y + size.y, position.z - size.z},
                glm::vec3{position.x - size.x, position.y - size.y, position.z + size.z},
                glm::vec3{position.x - size.x, position.y - size.y, position.z - size.z}};

    rightFace = {glm::vec3{position.x + size.x, position.y + size.y, position.z + size.z},
                 glm::vec3{position.x + size.x, position.y + size.y, position.z - size.z},
                 glm::vec3{position.x + size.x, position.y - size.y, position.z + size.z},
                 glm::vec3{position.x + size.x, position.y - size.y, position.z - size.z}};

    frontFace = {glm::vec3{position.x + size.x, position.y + size.y, position.z + size.z},
                 glm::vec3{position.x + size.x, position.y - size.y, position.z + size.z},
                 glm::vec3{position.x - size.x, position.y + size.y, position.z + size.z},
                 glm::vec3{position.x - size.x, position.y - size.y, position.z + size.z}};

    backFace = {glm::vec3{position.x + size.x, position.y + size.y, position.z - size.z},
                glm::vec3{position.x + size.x, position.y - size.y, position.z - size.z},
                glm::vec3{position.x - size.x, position.y + size.y, position.z - size.z},
                glm::vec3{position.x - size.x, position.y - size.y, position.z - size.z}};
};

bool CubeCollider::CheckCollision(const CubeCollider &other) const
{
    // Calculate min and max points for both colliders
    glm::vec3 minA = position - size;
    glm::vec3 maxA = position + size;
    glm::vec3 minB = other.position - other.size;
    glm::vec3 maxB = other.position + other.size;

    // Check for overlap on all axes
    if (maxA.x < minB.x || minA.x > maxB.x)
        return false;
    if (maxA.y < minB.y || minA.y > maxB.y)
        return false;
    if (maxA.z < minB.z || minA.z > maxB.z)
        return false;

    // If no axis has overlap, then there is a collision
    return true;
}

void CubeCollider::UpdateScale(glm::vec3 &sizes)
{
    setSize(sizes);

    topFace = {glm::vec3{position.x + size.x, position.y + size.y, position.z + size.z},
               glm::vec3{position.x + size.x, position.y + size.y, position.z - size.z},
               glm::vec3{position.x - size.x, position.y + size.y, position.z + size.z},
               glm::vec3{position.x - size.x, position.y + size.y, position.z - size.z}};

    std::cout << "x: " << position.x + size.x << "\ny: " << position.y + size.y << "\nz: " << position.z + size.z
              << std::endl
              << std::endl;

    bottomFace = {glm::vec3{position.x + size.x, position.y - size.y, position.z + size.z},
                  glm::vec3{position.x + size.x, position.y - size.y, position.z - size.z},
                  glm::vec3{position.x - size.x, position.y - size.y, position.z + size.z},
                  glm::vec3{position.x - size.x, position.y - size.y, position.z - size.z}};

    leftFace = {glm::vec3{position.x - size.x, position.y + size.y, position.z + size.z},
                glm::vec3{position.x - size.x, position.y + size.y, position.z - size.z},
                glm::vec3{position.x - size.x, position.y - size.y, position.z + size.z},
                glm::vec3{position.x - size.x, position.y - size.y, position.z - size.z}};

    rightFace = {glm::vec3{position.x + size.x, position.y + size.y, position.z + size.z},
                 glm::vec3{position.x + size.x, position.y + size.y, position.z - size.z},
                 glm::vec3{position.x + size.x, position.y - size.y, position.z + size.z},
                 glm::vec3{position.x + size.x, position.y - size.y, position.z - size.z}};

    frontFace = {glm::vec3{position.x + size.x, position.y + size.y, position.z + size.z},
                 glm::vec3{position.x + size.x, position.y - size.y, position.z + size.z},
                 glm::vec3{position.x - size.x, position.y + size.y, position.z + size.z},
                 glm::vec3{position.x - size.x, position.y - size.y, position.z + size.z}};

    backFace = {glm::vec3{position.x + size.x, position.y + size.y, position.z - size.z},
                glm::vec3{position.x + size.x, position.y - size.y, position.z - size.z},
                glm::vec3{position.x - size.x, position.y + size.y, position.z - size.z},
                glm::vec3{position.x - size.x, position.y - size.y, position.z - size.z}};
}

glm::vec3 CubeCollider::ResolveCollision(const CubeCollider &other)
{

    glm::vec3 minA = position - size;
    glm::vec3 maxA = position + size;
    glm::vec3 minB = other.position - other.size;
    glm::vec3 maxB = other.position + other.size;

    glm::vec3 mtv = glm::vec3(0.0f);
    glm::vec3 overlap = glm::vec3(0.0f);

    // Calculate overlap on each axis
    overlap.x = std::max(0.0f, std::min(maxA.x, maxB.x) - std::max(minA.x, minB.x));
    overlap.y = std::max(0.0f, std::min(maxA.y, maxB.y) - std::max(minA.y, minB.y));
    overlap.z = std::max(0.0f, std::min(maxA.z, maxB.z) - std::max(minA.z, minB.z));

    // Find the axis with the smallest overlap
    int axis;
    if (overlap.x <= overlap.y && overlap.x <= overlap.z)
    {
        axis = 0;
    }
    else if (overlap.y <= overlap.x && overlap.y <= overlap.z)
    {
        axis = 1;
    }
    else
    {
        axis = 2;
    }

    // Snap player position to grid on the colliding axis
    float gridSize = 1.0f;

    // float newPosition = glm::floor((position[axis] + (axis == 0 ? -overlap.x : overlap.x)) / gridSize) * gridSize;

    float newPosition;
    if (axis == 0) // x-axis
    {
        if (position.x > other.position.x)
            newPosition = glm::floor((position.x + overlap.x) / gridSize) * gridSize;
        else
            newPosition = glm::ceil((position.x - overlap.x) / gridSize) * gridSize;
    }
    else if (axis == 1) // y-axis
    {
        if (position.y > other.position.y)
            newPosition = glm::floor((position.y + overlap.y) / gridSize) * gridSize;
        else
            newPosition = glm::ceil((position.y - overlap.y) / gridSize) * gridSize;
    }
    else if (axis == 2) // z-axis
    {
        if (position.z > other.position.z)
            newPosition = glm::floor((position.z + overlap.z) / gridSize) * gridSize;
        else
            newPosition = glm::ceil((position.z - overlap.z) / gridSize) * gridSize;
    }

    mtv[axis] = newPosition - position[axis];
    if (std::abs(mtv[axis]) > FLT_EPSILON)
    {
        position[axis] += mtv[axis];
    }

    // Move the collider out of collision (considering early termination)
    if (std::abs(mtv[axis]) > FLT_EPSILON)
    { // Epsilon check for tiny movements
        position[axis] += mtv[axis];
    }

    return mtv;
}

const float &CubeCollider::getOverlapX(const CubeCollider &other) const
{
    glm::vec3 minA = position - size;
    glm::vec3 maxA = position + size;
    glm::vec3 minB = other.position - other.size;
    glm::vec3 maxB = other.position + other.size;

    return std::min(maxA.x - minB.x, maxB.x - minA.x);
}

const float &CubeCollider::getOverlapY(const CubeCollider &other) const
{
    glm::vec3 minA = position - size;
    glm::vec3 maxA = position + size;
    glm::vec3 minB = other.position - other.size;
    glm::vec3 maxB = other.position + other.size;

    return std::min(maxA.y - minB.y, maxB.y - minA.y);
}

const float &CubeCollider::getOverlapZ(const CubeCollider &other) const
{
    glm::vec3 minA = position - size;
    glm::vec3 maxA = position + size;
    glm::vec3 minB = other.position - other.size;
    glm::vec3 maxB = other.position + other.size;

    return std::min(maxA.z - minB.z, maxB.z - minA.z);
}

const glm::vec3 &CubeCollider::getPosition() const
{
    return position;
}

void CubeCollider::setPosition(const glm::vec3 &newPosition)
{
    position = newPosition;
}

const glm::vec3 &CubeCollider::getSize() const
{
    return size;
}

void CubeCollider::setSize(const glm::vec3 &newSize)
{
    size = newSize;
}
