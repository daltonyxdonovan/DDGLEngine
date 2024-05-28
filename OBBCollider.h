#ifndef OBBCOLLIDER_H
#define OBBCOLLIDER_H

#define GLM_ENABLE_EXPERIMENTAL

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>
#include <limits>
#include <vector>

class OBBCollider
{
  public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 size; // Half-sizes
    glm::quat orientation;

    OBBCollider() : position(0.0f), size(1.0f), orientation(0.0f,0.0f,0.0f,1.0f)
    {
    }

    OBBCollider(const glm::vec3 &pos, const glm::vec3 &halfSize, const glm::mat3 &orient)
        : position(pos), size(halfSize), orientation(orient)
    {

    }

    // Set rotation using a quaternion (assuming normalized)
  void setRotation(const glm::quat &newOrientation)
  {
    orientation = newOrientation;
  }

        void SetOBBColliderFromCorners(OBBCollider& collider, const std::vector<glm::vec3>& corners) {
  // Validate input size
  if (corners.size() != 24) {
    return; // Error: Unexpected number of corner positions
  }

  // Find minimum and maximum extents of the OBB
  glm::vec3 minExtents = corners[0];
  glm::vec3 maxExtents = corners[0];
  for (int i = 1; i < corners.size(); i++) {
    minExtents.x = std::min(minExtents.x, corners[i].x);
    minExtents.y = std::min(minExtents.y, corners[i].y);
    minExtents.z = std::min(minExtents.z, corners[i].z);

    maxExtents.x = std::max(maxExtents.x, corners[i].x);
    maxExtents.y = std::max(maxExtents.y, corners[i].y);
    maxExtents.z = std::max(maxExtents.z, corners[i].z);
  }

  // Calculate center of the OBB
  glm::vec3 center = (minExtents + maxExtents) / 2.0f;

  // Calculate half dimensions of the OBB
  glm::vec3 halfExtents = (maxExtents - minExtents) / 2.0f;

  // Basis vectors of the OBB (assuming principal axes)
  glm::vec3 basisX = glm::normalize(corners[1] - corners[0]); // Assuming opposite corners on X-axis
  glm::vec3 basisY = glm::normalize(corners[4] - corners[0]); // Assuming opposite corners on Y-axis
  glm::vec3 basisZ = glm::normalize(corners[16] - corners[0]); // Assuming opposite corners on Z-axis

  // Check for degenerate basis vectors (colinear points)
  if (glm::abs(glm::dot(basisX, basisY)) > 0.99f ||
      glm::abs(glm::dot(basisX, basisZ)) > 0.99f ||
      glm::abs(glm::dot(basisY, basisZ)) > 0.99f) {
    // Error: Degenerate OBB (colinear points) - handle appropriately (e.g., use AABB)
    return;
  }

  // Create final rotation matrix from basis vectors
  glm::mat3 rotationMatrix(basisX, basisY, basisZ);

  // Convert rotation matrix to quaternion
  collider.orientation = glm::quat_cast(rotationMatrix);

  // Set center and half extents of the OBB collider
  collider.position = center;
  collider.size = halfExtents;
}


  bool OverlapOnAxis(const OBBCollider &other, const glm::vec3 &axis) const
{
  float aProj = glm::dot(size, glm::abs(glm::normalize(glm::rotate(orientation, axis)))); // Normalize after rotation
  float bProj = glm::dot(other.size, glm::abs(glm::normalize(glm::rotate(other.orientation, axis))));
  float distance = glm::abs(glm::dot(position - other.position, axis));
  return distance <= aProj + bProj;
}

  float PenetrationDepthOnAxis(const OBBCollider &other, const glm::vec3 &axis) const
  {
    float aProj = glm::dot(size, glm::abs(glm::rotate(orientation, axis)));
    float bProj = glm::dot(other.size, glm::abs(glm::rotate(other.orientation, axis)));
    float distance = glm::abs(glm::dot(position - other.position, axis));
    return (aProj + bProj) - distance;
  }


    glm::vec3 ResolveCollision(const OBBCollider &other) const
    {
        std::vector<glm::vec3> axes;

  // Generate axes from rotated basis vectors of both OBBs using quaternions
  for (int i = 0; i < 3; ++i)
  {
      axes.push_back(glm::normalize(glm::rotate(orientation, glm::vec3(1.0f, 0.0f, 0.0f))));
      axes.push_back(glm::normalize(glm::rotate(other.orientation, glm::vec3(1.0f, 0.0f, 0.0f))));
      axes.push_back(glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 1.0f, 0.0f))));
      axes.push_back(glm::normalize(glm::rotate(other.orientation, glm::vec3(0.0f, 1.0f, 0.0f))));
      axes.push_back(glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 0.0f, 1.0f))));
      axes.push_back(glm::normalize(glm::rotate(other.orientation, glm::vec3(0.0f, 0.0f, 1.0f))));
  }


    // Normalize and perform remaining checks as before
    for (int i = 0; i < 15; ++i)
    {
      if (glm::length2(axes[i]) > std::numeric_limits<float>::epsilon())
      {
        axes[i] = glm::normalize(axes[i]);
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

    bool IsPointInside(const glm::vec3 &point) const
    {
    // Transform the point to the OBB's local coordinate system using inverse rotation
    glm::vec3 localPoint = glm::inverse(orientation) * (point - position);

    // Check if the local point is within the OBB's bounds
    return (std::abs(localPoint.x) <= size.x && std::abs(localPoint.y) <= size.y &&
            std::abs(localPoint.z) <= size.z);
    }

    bool CheckCollision(const OBBCollider &other) const
    {
        

        // Cross products of each pair of axes
        std::vector<glm::vec3> axes;

  // Generate axes from rotated basis vectors of both OBBs using quaternions
        for (int i = 0; i < 3; ++i)
        {
            axes.push_back(glm::normalize(glm::rotate(orientation, glm::vec3(1.0f, 0.0f, 0.0f))));
            axes.push_back(glm::normalize(glm::rotate(other.orientation, glm::vec3(1.0f, 0.0f, 0.0f))));
            axes.push_back(glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 1.0f, 0.0f))));
            axes.push_back(glm::normalize(glm::rotate(other.orientation, glm::vec3(0.0f, 1.0f, 0.0f))));
            axes.push_back(glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 0.0f, 1.0f))));
            axes.push_back(glm::normalize(glm::rotate(other.orientation, glm::vec3(0.0f, 0.0f, 1.0f))));
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
