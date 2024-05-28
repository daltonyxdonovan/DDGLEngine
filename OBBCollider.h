#ifndef OBBCOLLIDER_H
#define OBBCOLLIDER_H

#define GLM_ENABLE_EXPERIMENTAL

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>
#include <limits>
#include <vector>

class OBBCollider
{
  public:
    glm::vec3 position;
    glm::vec3 size; // Half-sizes
    glm::mat3 orientation;

    OBBCollider() : position(0.0f), size(1.0f), orientation(1.0f)
    {
    }

    OBBCollider(const glm::vec3 &pos, const glm::vec3 &halfSize, const glm::mat3 &orient)
        : position(pos), size(halfSize), orientation(orient)
    {
    }

    bool OverlapOnAxis(const OBBCollider &other, const glm::vec3 &axis) const
    {
        float aProj = glm::dot(size, glm::abs(orientation * axis));
        float bProj = glm::dot(other.size, glm::abs(other.orientation * axis));
        float distance = glm::abs(glm::dot(position - other.position, axis));
        return distance <= aProj + bProj;
    }

    float PenetrationDepthOnAxis(const OBBCollider &other, const glm::vec3 &axis) const
    {
        float aProj = glm::dot(size, glm::abs(orientation * axis));
        float bProj = glm::dot(other.size, glm::abs(other.orientation * axis));
        float distance = glm::abs(glm::dot(position - other.position, axis));
        return (aProj + bProj) - distance;
    }

    glm::vec3 ResolveCollision(const OBBCollider &other) const
    {
        std::array<glm::vec3, 15> axes;

        // Generate all potential separating axes
        for (int i = 0; i < 3; ++i)
        {
            axes[i] = orientation[i];
            axes[i + 3] = other.orientation[i];
        }

        int index = 6;
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                axes[index] = glm::cross(orientation[i], other.orientation[j]);
                if (glm::length2(axes[index]) > std::numeric_limits<float>::epsilon())
                {
                    axes[index] = glm::normalize(axes[index]);
                }
                index++;
            }
        }

        float minPenetrationDepth = std::numeric_limits<float>::max();
        glm::vec3 mtvAxis;

        for (const auto &axis : axes)
        {
            if (glm::length2(axis) < std::numeric_limits<float>::epsilon())
                continue;

            if (!OverlapOnAxis(other, axis))
            {
                return glm::vec3(0.0f); // No collision
            }
            else
            {
                float depth = PenetrationDepthOnAxis(other, axis);
                if (depth < minPenetrationDepth)
                {
                    minPenetrationDepth = depth;
                    mtvAxis = axis;
                }
            }
        }

        glm::vec3 mtv = glm::normalize(mtvAxis) * minPenetrationDepth;

        // Ensure MTV points in the correct direction
        if (glm::dot(mtv, other.position - position) < 0.0f)
        {
            mtv = -mtv;
        }

        return mtv;
    }

    void setSize(const glm::vec3 &newSize)
    {
        size = newSize;
    }

    void setPosition(const glm::vec3 &newPosition)
    {
        position = newPosition;
    }

    bool CheckCollision(const OBBCollider &other) const
    {
        glm::vec3 axes[15];
        glm::mat3 rotation = orientation;
        glm::mat3 otherRotation = other.orientation;

        // Current OBB's local axes
        axes[0] = rotation[0];
        axes[1] = rotation[1];
        axes[2] = rotation[2];

        // Other OBB's local axes
        axes[3] = otherRotation[0];
        axes[4] = otherRotation[1];
        axes[5] = otherRotation[2];

        // Cross products of each pair of axes
        int index = 6;
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                axes[index] = glm::cross(axes[i], axes[3 + j]);
                index++;
            }
        }

        for (int i = 0; i < 15; ++i)
        {
            if (!OverlapOnAxis(other, axes[i]))
            {
                return false; // Found a separating axis, no collision
            }
        }

        return true; // No separating axis found, collision detected
    }
};

#endif // OBBCOLLIDER_H
