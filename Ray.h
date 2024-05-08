#pragma once
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>

class Ray
{
public:
    glm::ivec3 startPoint; // Using ivec3 for integer-based positions
    glm::vec3 direction;

    Ray(glm::ivec3 startPoint, glm::vec3 direction)
    {
        this->startPoint = startPoint;
        this->direction = glm::normalize(direction); // Normalize direction vector
    }

    // Method to get a point along the ray at a given distance
    glm::ivec3 pointAtDistance(int distance) const
    {
        return glm::ivec3(startPoint + glm::ivec3(direction.x * distance, direction.y * distance, direction.z * distance));
    }

    // Method to check for intersection with a cube
    bool intersectsCube(glm::ivec3 cubePosition, int cubeSize) const
    {
        // Calculate the minimum and maximum points of the cube
        glm::ivec3 minCubePoint = cubePosition;
        glm::ivec3 maxCubePoint = cubePosition + glm::ivec3(cubeSize);

        // Calculate t-values for each component of the ray
        glm::vec3 t1;
        glm::vec3 t2;

        // Calculate t1 for each component
        t1.x = (minCubePoint.x - startPoint.x) / direction.x;
        t1.y = (minCubePoint.y - startPoint.y) / direction.y;
        t1.z = (minCubePoint.z - startPoint.z) / direction.z;

        // Calculate t2 for each component
        t2.x = (maxCubePoint.x - startPoint.x) / direction.x;
        t2.y = (maxCubePoint.y - startPoint.y) / direction.y;
        t2.z = (maxCubePoint.z - startPoint.z) / direction.z;


        // Find the largest t-value
        glm::vec3 tMin = glm::min(t1, t2);
        glm::vec3 tMax = glm::max(t1, t2);

        // Find the largest tMin and the smallest tMax
        float largestTMin = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
        float smallestTMax = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

        // Check if ray intersects with the cube
        return smallestTMax >= largestTMin;
    }
};
