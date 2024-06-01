
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// Do NOT connect these two
#include "Camera.h"
#include "CubeCollider.h"
#include "FastNoiseLite.h"
#include "IndexBuffer.h"
#include "Light.h"
#include "OBBCollider.h"
#include "Ray.h"
#include "Renderer.h"
#include "Shader.h"
#include "SoundManager.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "src/Texture.h"
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

bool CheckGLErrors(const char *function, const char *file, int line)
{
    bool errorOccurred = false;
    while (GLenum error = glGetError())
    {
        std::cerr << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
        errorOccurred = true;
    }
    return errorOccurred;
}

#define GLCall(x)                                                                                                      \
    ClearGLErrors();                                                                                                   \
    x;                                                                                                                 \
    CheckGLErrors(#x, __FILE__, __LINE__);

void ClearGLErrors()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

Camera camera;
int width = 1920;
int height = 1080;
bool mouseControl = false;
std::vector<Light *> lights;
bool picking = false;
int chosenTextureID = 0;
glm::vec3 sizeToSave = glm::vec3(1, 1, 1);
float ambientLightLevel = 0.1f;
int currentLevel = 0;
int maxLevelAllowed = 9;
bool showDebugInfo = true;
int showDebugCooldown = 0;
float stepHeight = 3.f;

std::vector<std::string> inventory = {
    "Red Key",
};

#pragma region CLASSES/METHODS

class Cube
{
  public:
    glm::vec3 position;
    glm::vec3 positionLowest;
    glm::vec3 positionHighest;
    glm::vec3 rotation;
    glm::vec3 scale;
    std::vector<glm::vec3> vertices;
    int textureIndex = 0;
    std::vector<glm::vec3> verticesAfterTransformation = {};
    OBBCollider collider;
    bool needsSoundPlayed = false;
    bool playedSoundAlready = false;
    int index = 0;
    bool invisible = false;
    std::vector<float> cornerPositions;
    std::vector<float> cornerPositionsOriginal;
    Cube *parent = nullptr;
    std::vector<Cube *> children;
    bool door = false;
    bool requiresRedKey = false;
    bool requiresGreenKey = false;
    bool requiresBlueKey = false;
    bool liftingPlatform = false;
    bool movingPlatform = false;
    int direction = 0;

    std::vector<unsigned int> indices = {
        0,  1,  2,  2,  3,  0,  // front face
        4,  5,  6,  6,  7,  4,  // back face
        8,  9,  10, 10, 11, 8,  // top face
        12, 13, 14, 14, 15, 12, // bottom face
        16, 17, 18, 18, 19, 16, // right face
        20, 21, 22, 22, 23, 20  // left face
    };

    Cube()
    {
        float textureID = 0;
        float chosen = 0;
        this->position = glm::vec3(0, 0, 0);
        this->positionLowest = position;
        this->positionHighest = positionLowest + glm::vec3(0, 10, 0);
        this->rotation = glm::vec3(0, 0, 0);
        this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
        collider = OBBCollider(position, scale, glm::mat3(1.0f));
        SetCornerPositions(textureID, chosen);
        UpdateVertices();
    }

    Cube(glm::vec3 position, float textureID, float chosen = 0)
    {
        this->position = position;
        this->positionLowest = position;
        this->positionHighest = positionLowest + glm::vec3(0, 10, 0);
        this->rotation = glm::vec3();
        this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
        collider = OBBCollider(position, scale, glm::mat3(1.0f));
        SetCornerPositions(textureID, chosen);
        UpdateVertices();
    }

    Cube(glm::vec3 pos, float textureID) : position(pos), textureIndex(textureID)
    {
        float chosen = 0;
        this->positionLowest = position;
        this->positionHighest = positionLowest + glm::vec3(0, 10, 0);
        this->rotation = glm::vec3();
        this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
        collider = OBBCollider(position, scale, glm::mat3(1.0f));
        SetCornerPositions(textureID, chosen);
        UpdateVertices();
    }

    void print() const
    {
        auto printVec3 = [](const std::string &name, const glm::vec3 &vec) {
            std::cout << name << ": (" << vec.x << ", " << vec.y << ", " << vec.z << ")\n";
        };

        std::cout << "Cube Details:\n";
        printVec3("Position", position);
        printVec3("Rotation", rotation);
        printVec3("Scale", scale);

        std::cout << "Vertices: [";
        for (const auto &vertex : vertices)
        {
            std::cout << " (" << vertex.x << ", " << vertex.y << ", " << vertex.z << ")";
        }
        std::cout << " ]\n";

        std::cout << "Texture Index: " << textureIndex << "\n";

        std::cout << "Vertices After Transformation: [";
        for (const auto &vertex : verticesAfterTransformation)
        {
            std::cout << " (" << vertex.x << ", " << vertex.y << ", " << vertex.z << ")";
        }
        std::cout << " ]\n";

        std::cout << "Collider Position: (" << collider.position.x << ", " << collider.position.y << ", "
                  << collider.position.z << ")\n";
        std::cout << "Collider Size: (" << collider.size.x << ", " << collider.size.y << ", " << collider.size.z
                  << ")\n";

        std::cout << "Index: " << index << "\n";
        std::cout << "Invisible: " << (invisible ? "true" : "false") << "\n";

        std::cout << "Corner Positions: [";
        for (size_t i = 0; i < cornerPositions.size(); i += 10)
        {
            std::cout << " (";
            for (size_t j = 0; j < 10; ++j)
            {
                std::cout << cornerPositions[i + j] << (j < 9 ? ", " : "");
            }
            std::cout << ")";
        }
        std::cout << " ]\n";

        std::cout << "Indices: [";
        for (size_t i = 0; i < indices.size(); ++i)
        {
            std::cout << indices[i] << (i < indices.size() - 1 ? ", " : "");
        }
        std::cout << " ]\n";
    }

    void SetCornerPositions(float textureID, float chosen)
    {
        // clang-format off
        this->cornerPositions = 
        {
            // Front face
            -1,  1,  1, 0.0f, 0.0f, textureID,  0,  0,  1, chosen,
            1,  1,  1, 0.1f, 0.0f, textureID,  0,  0,  1, chosen,
             1, -1,  1, 0.1f, 0.1f, textureID,  0,  0,  1, chosen,
            -1, -1,  1, 0.0f, 0.1f, textureID,  0,  0,  1, chosen,

            // Back face
            1, -1, -1, 0.0f, 0.0f, textureID,  0,  0, -1, chosen,
             1, 1, -1, 0.1f, 0.0f, textureID,  0,  0, -1, chosen,
             -1,  1, -1, 0.1f, 0.1f, textureID,  0,  0, -1, chosen,
            -1,  -1, -1, 0.0f, 0.1f, textureID,  0,  0, -1, chosen,

            // Top face
            -1,  1, -1, 0.0f, 0.0f, textureID,  0,  1,  0, chosen,
             1,  1, -1, 0.1f, 0.0f, textureID,  0,  1,  0, chosen,
             1,  1,  1, 0.1f, 0.1f, textureID,  0,  1,  0, chosen,
            -1,  1,  1, 0.0f, 0.1f, textureID,  0,  1,  0, chosen,

            // Bottom face
            -1, -1, 1, 0.0f, 0.0f, textureID,  0, -1,  0, chosen,
             1, -1, 1, 0.1f, 0.0f, textureID,  0, -1,  0, chosen,
             1, -1,  -1, 0.1f, 0.1f, textureID,  0, -1,  0, chosen,
            -1, -1,  -1, 0.0f, 0.1f, textureID,  0, -1,  0, chosen,

            // Right face
             1, -1, -1, 0.0f, 0.0f, textureID,  1,  0,  0, chosen,
             1,  -1, 1, 0.0f, 0.1f, textureID,  1,  0,  0, chosen,
             1,  1,  1, 0.1f, 0.1f, textureID,  1,  0,  0, chosen,
             1, 1,  -1, 0.1f, 0.0f, textureID,  1,  0,  0, chosen,

            // Left face
            -1, -1, -1, 0.0f, 0.0f, textureID, -1,  0,  0, chosen,
            -1,  1, -1, 0.0f, 0.1f, textureID, -1,  0,  0, chosen,
            -1,  1,  1, 0.1f, 0.1f, textureID, -1,  0,  0, chosen,
            -1, -1,  1, 0.1f, 0.0f, textureID, -1,  0,  0, chosen,
        };

        // clang-format on

        cornerPositionsOriginal = cornerPositions;
    }

    void UpdateVertices()
    {
        vertices = {
            glm::vec3(position.x - scale.x, position.y - scale.y, position.z + scale.z), // 0
            glm::vec3(position.x + scale.x, position.y - scale.y, position.z + scale.z), // 1
            glm::vec3(position.x + scale.x, position.y + scale.y, position.z + scale.z), // 2
            glm::vec3(position.x - scale.x, position.y + scale.y, position.z + scale.z), // 3
            glm::vec3(position.x - scale.x, position.y - scale.y, position.z - scale.z), // 4
            glm::vec3(position.x + scale.x, position.y - scale.y, position.z - scale.z), // 5
            glm::vec3(position.x + scale.x, position.y + scale.y, position.z - scale.z), // 6
            glm::vec3(position.x - scale.x, position.y + scale.y, position.z - scale.z)  // 7
        };
    }

    void SetInvisible(bool yes = true)
    {
        invisible = yes;
    }

    void Increase(sf::Sound &soundToPlay)
    {
        if (position.y < positionHighest.y)
        {
            position.y += 0.02;
            collider.position.y += 0.02;
            // soundToPlay.play();
        }
    }

    void Decrease()
    {
        if (position.y > positionLowest.y)
        {
            position.y -= 0.01f;
            collider.position.y -= 0.01f;
        }
    }

    bool isPointInside(glm::vec3 point)
    {
        glm::vec3 localPoint = glm::inverse(collider.orientation) * (point - collider.position);
        return glm::all(glm::lessThan(glm::abs(localPoint), collider.size));
    }
};

class Object
{
  public:
    std::vector<Cube> voxels;
    std::vector<glm::vec3> offsets;
    glm::vec3 position = glm::vec3(0);
    std::string name;

    Object(std::string name)
    {
        this->name = name;
    }

    void AddCube(glm::vec3 distanceToOrigin, float textureID, glm::vec3 scale)
    {
        Cube cube(distanceToOrigin, textureID, 0);
        cube.scale = scale;

        voxels.push_back(cube);
        offsets.push_back(distanceToOrigin);
    }
};

class Image
{
  public:
    glm::vec3 position;
    glm::vec3 scale;
    std::vector<glm::vec3> vertices;
    int textureIndex = 0;
    std::vector<glm::vec3> verticesAfterTransformation = {};
    CubeCollider collider;
    int index = 0;
    bool invisible = false;
    std::vector<float> cornerPositions;
    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0, // front face
    };

    Image(glm::vec3 position, glm::vec3 scale)
    {
        this->position = position;
        this->scale = scale;
    }

    void SetCornerPositions(float textureID, float invisible)
    {
        // clang-format off
        this->cornerPositions = 
        {
            // Front face
            -1,  1,  1, 0.0f, 0.0f, textureID,
             1,  1,  1, 0.1f, 0.0f, textureID,
             1, -1,  1, 0.1f, 0.1f, textureID,
            -1, -1,  1, 0.0f, 0.1f, textureID
        };
        // clang-format on
    }

    void Update(glm::vec3 playerPosition)
    {
    }
};

struct Notification
{
    std::string message = "";
    float time = 0;
};

std::vector<Notification *> notifications;

void addNotification(std::string message, float time)
{
    Notification *notification = new Notification();
    notification->message = message;
    notification->time = time;
    notifications.push_back(notification);
}

bool needsRefresh = true;

void RotatePoint(glm::vec3 &point, const glm::vec3 &pointRotatingAround, const glm::vec3 &eulerRotation,
                 const glm::vec3 &originalPosition)

{
    // Convert Euler angles (in degrees) to radians
    glm::vec3 radiansRotation = glm::radians(eulerRotation);

    // Convert Euler angles to a quaternion
    glm::quat rotationQuat = glm::quat(radiansRotation);

    // Rotate the point using the quaternion
    glm::vec3 rotatedPoint = rotationQuat * originalPosition;

    // Translate the point back to its original position
    point = rotatedPoint;
}

glm::vec3 CastPointForward(float distance, glm::vec3 startingPosition)
{
    glm::vec3 direction = camera.target - camera.position;
    direction.x = direction.x * distance;
    direction.y = direction.y * distance;
    direction.z = direction.z * distance;

    direction.x += startingPosition.x;
    direction.y += startingPosition.y;
    direction.z += startingPosition.z;
    return direction;
}

void SaveCubesToFile(const std::vector<Cube> &cubes, const std::string &filename)
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    for (const auto &cube : cubes)
    {
        file << "Cube\n";
        file << "Position: " << cube.position.x << " " << cube.position.y << " " << cube.position.z << "\n";
        file << "Rotation: " << cube.rotation.x << " " << cube.rotation.y << " " << cube.rotation.z << "\n";
        file << "Scale: " << cube.scale.x << " " << cube.scale.y << " " << cube.scale.z << "\n";
        file << "Door: " << cube.door << "\n";
        file << cube.requiresRedKey << "\n";
        file << cube.requiresGreenKey << "\n";
        file << cube.requiresBlueKey << "\n";
        file << "TextureIndex: " << cube.textureIndex << "\n";
        file << "VerticesAfterTransformation: ";
        for (const auto &vertex : cube.verticesAfterTransformation)
        {
            file << vertex.x << " " << vertex.y << " " << vertex.z << " ";
        }
        file << "\n";
        file << "Index: " << cube.index << "\n";
        file << "Invisible: " << (cube.invisible ? "true" : "false") << "\n";
        file << "CornerPositions: ";
        for (const auto &corner : cube.cornerPositions)
        {
            file << corner << " ";
        }
        file << "\n";
        file << "Indices: ";
        for (const auto &index : cube.indices)
        {
            file << index << " ";
        }
        file << "\n";
    }

    file.close();
}

std::vector<Cube> LoadCubesFromFile(const std::string &filename)
{
    std::vector<Cube> cubes;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return cubes;
    }

    // std::cout << "loading voxels..." << std::endl;

    std::string line;
    while (std::getline(file, line))
    {
        if (line == "Cube")
        {
            Cube cube; // Default constructor, position and textureID will be updated

            std::getline(file, line);
            std::istringstream posStream(line.substr(10));
            posStream >> cube.position.x >> cube.position.y >> cube.position.z;
            cube.positionLowest = cube.position;
            cube.positionHighest = cube.positionLowest + glm::vec3(0, 10, 0);

            std::getline(file, line);
            std::istringstream rotStream(line.substr(10));
            rotStream >> cube.rotation.x >> cube.rotation.y >> cube.rotation.z;

            std::getline(file, line);
            std::istringstream scaleStream(line.substr(7));
            scaleStream >> cube.scale.x >> cube.scale.y >> cube.scale.z;

            std::string doorString;
            std::getline(file, line);
            std::istringstream doorStream(line.substr(6));
            doorStream >> doorString;
            cube.door = (doorString == "1");

            std::getline(file, line);
            std::string redstring;
            std::istringstream redStream(line);
            redStream >> redstring;
            cube.requiresRedKey = (redstring == "1");

            std::getline(file, line);
            std::string greenstring;
            std::istringstream greenStream(line);
            greenStream >> greenstring;
            cube.requiresGreenKey = (greenstring == "1");

            std::getline(file, line);
            std::string bluestring;
            std::istringstream blueStream(line);
            blueStream >> bluestring;
            cube.requiresBlueKey = (bluestring == "1");

            std::getline(file, line);
            cube.textureIndex = std::stoi(line.substr(14));

            std::getline(file, line);
            std::istringstream vatStream(line.substr(28));
            cube.verticesAfterTransformation.clear();
            glm::vec3 vertex;
            while (vatStream >> vertex.x >> vertex.y >> vertex.z)
            {
                cube.verticesAfterTransformation.push_back(vertex);
            }

            std::getline(file, line);
            cube.index = std::stoi(line.substr(7));

            std::getline(file, line);
            cube.invisible = (line.substr(11) == "true");

            std::getline(file, line);
            std::istringstream cpStream(line.substr(17));
            cube.cornerPositions.clear();
            float corner;
            while (cpStream >> corner)
            {
                cube.cornerPositions.push_back(corner);
            }

            std::getline(file, line);
            std::istringstream indicesStream(line.substr(9));
            cube.indices.clear();
            unsigned int index;
            while (indicesStream >> index)
            {
                cube.indices.push_back(index);
            }

            cubes.push_back(cube);

            // std::cout << "cube loaded!" << std::endl;
        }
    }

    file.close();
    return cubes;
}

void SaveLightsToFile(const std::vector<Light *> &lights, const std::string &filename)
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    for (const auto &light : lights)
    {
        file << "Light\n";
        file << "Position: " << light->position.x << " " << light->position.y << " " << light->position.z << "\n";
        file << "Direction: " << light->direction.x << " " << light->direction.y << " " << light->direction.z << "\n";
        file << "RGBA: " << light->rgba.r << " " << light->rgba.g << " " << light->rgba.b << " " << light->rgba.a
             << "\n";
        file << "Down: " << (light->down ? "true" : "false") << "\n";
        file << "Brightness: " << light->brightness << "\n";
    }

    file.close();
}

std::vector<Light *> LoadLightsFromFile(const std::string &filename)
{
    std::vector<Light *> lights;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return lights;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line == "Light")
        {
            glm::vec3 position;
            glm::vec3 direction;
            glm::vec4 rgba;
            bool down;
            float brightness;

            std::getline(file, line);
            std::istringstream posStream(line.substr(10));
            posStream >> position.x >> position.y >> position.z;

            std::getline(file, line);
            std::istringstream dirStream(line.substr(11));
            dirStream >> direction.x >> direction.y >> direction.z;

            std::getline(file, line);
            std::istringstream rgbaStream(line.substr(6));
            rgbaStream >> rgba.r >> rgba.g >> rgba.b >> rgba.a;

            std::getline(file, line);
            down = (line.substr(6) == "true");

            std::getline(file, line);
            brightness = std::stof(line.substr(12));

            Light *light = new Light(position);
            light->direction = direction;
            light->rgba = rgba;
            light->down = down;
            light->brightness = brightness;

            lights.push_back(light);
        }
    }

    file.close();
    return lights;
}

void Refresh(int &indicesCount, std::vector<Cube> &voxels, unsigned int AMOUNT_OF_INDICES, unsigned int *&indicesAfter,
             float *&positions, VertexBufferLayout &layout, VertexArray &va, VertexBuffer &vb, IndexBuffer &ib,
             unsigned int FULL_STRIDE, bool PRINTLOOPLOG, unsigned int STRIDE, unsigned int AMOUNT_OF_INDICES2,
             unsigned int *&indicesAfter2, std::vector<Image> &images, int &indicesCount2, unsigned int FULL_STRIDE2)
{
    // addNotification("Refreshing map", 10);
    if (PRINTLOOPLOG)
        std::cout << "refreshing map..." << std::endl;
    needsRefresh = false;
    indicesCount = voxels.size() * AMOUNT_OF_INDICES;

    if (indicesAfter != nullptr)
    {
        delete[] indicesAfter;
        indicesAfter = nullptr;
    }

    indicesAfter = new unsigned int[indicesCount];

    for (int i = 0; i < voxels.size(); i++)
    {
        for (int j = 0; j < voxels[i].indices.size(); j++)
        {
            indicesAfter[i * AMOUNT_OF_INDICES + j] = voxels[i].indices[j] + i * AMOUNT_OF_INDICES;
        }
        voxels[i].index = i;
    }

    std::vector<float> scales;

    for (int i = 0; i < voxels.size(); i++)
    {
        scales.push_back(voxels[i].scale.x);
        scales.push_back(voxels[i].scale.y);
        scales.push_back(voxels[i].scale.z);
    }

    if (positions != nullptr)
    {
        delete[] positions;
        positions = nullptr;
    }
    positions = new float[(voxels.size() * STRIDE * AMOUNT_OF_INDICES)];

    for (int i = 0; i < voxels.size(); i++)
    {

        for (int j = 0; j < voxels[i].cornerPositions.size(); j++)
        {
            positions[i * STRIDE * AMOUNT_OF_INDICES + j] = voxels[i].cornerPositions[j];
            if (j == 0)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 1)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 2)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 10)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 11)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 12)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 20)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 21)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 22)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 30)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 31)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 32)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 40)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 41)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 42)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 50)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 51)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 52)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 60)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 61)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 62)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 70)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 71)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 72)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 80)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 81)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 82)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 90)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 91)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 92)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 100)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 101)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 102)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 110)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 111)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 112)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 120)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 121)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 122)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 130)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 131)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 132)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 140)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 141)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 142)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 150)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 151)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 152)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 160)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 161)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 162)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 170)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 171)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 172)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 180)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 181)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 182)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 190)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 191)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 192)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 200)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 201)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 202)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 210)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 211)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 212)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 220)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 221)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 222)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j == 230)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3];
            if (j == 231)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 1];
            if (j == 232)
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] *= scales[i * 3 + 2];

            if (j % STRIDE == 0) // x
            {
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.x;
            }
            else if (j % STRIDE == 1) // y
            {
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.y;
            }
            else if (j % STRIDE == 2) // z
            {
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.z;
            }
            // else if (j % STRIDE == 3) //u
            //{
            //     positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.z;
            // }
            // else if (j % STRIDE == 4) //v
            //{
            //     positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.z;
            // }
            else if (j % STRIDE == 5) // textureID
            {
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] = voxels[i].textureIndex;
            }
            // else if (j % STRIDE == 6) //normal.x
            //{
            //     positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.z;
            // }
            // else if (j % STRIDE == 7) //normal.y
            //{
            //     positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.z;
            // }
            // else if (j % STRIDE == 8) //normal.z
            //{
            //     positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.z;
            //}
            else if (j % STRIDE == 9) // invisible
            {
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] = voxels[i].invisible;
            }
        }
    }

    va.Unbind();
    vb.Unbind();
    ib.Unbind();

    vb.UpdateBuffer(positions, (voxels.size() * FULL_STRIDE));
    va.AddBuffer(vb, layout);
    ib.UpdateBuffer(indicesAfter, indicesCount);

    va.Bind();
    vb.Bind();
    ib.Bind();
    if (PRINTLOOPLOG)
        std::cout << "done refreshing map!" << std::endl;
}

void SaveObject(std::string name, std::vector<Cube> &voxels)
{
    std::string filename = "res/objects/" + name + ".txt";

    // save all variables for the object and it's voxels to a txt file in 'res/objects/name.txt'
    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    Object object(name);
    for (int i = 0; i < voxels.size(); i++)
    {
        object.AddCube(voxels[i].position, voxels[i].textureIndex, voxels[i].scale);
    }

    for (const auto &cube : voxels)
    {
        file << "Cube\n";
        file << "Position: " << cube.position.x << " " << cube.position.y << " " << cube.position.z << "\n";
        file << "Rotation: " << cube.rotation.x << " " << cube.rotation.y << " " << cube.rotation.z << "\n";
        file << "Scale: " << cube.scale.x << " " << cube.scale.y << " " << cube.scale.z << "\n";
        file << "TextureIndex: " << cube.textureIndex << "\n";
        file << "VerticesAfterTransformation: ";
        for (const auto &vertex : cube.verticesAfterTransformation)
        {
            file << vertex.x << " " << vertex.y << " " << vertex.z << " ";
        }
        file << "\n";
        file << "Index: " << cube.index << "\n";
        file << "Invisible: " << (cube.invisible ? "true" : "false") << "\n";
        file << "CornerPositions: ";
        for (const auto &corner : cube.cornerPositions)
        {
            file << corner << " ";
        }
        file << "\n";
        file << "Indices: ";
        for (const auto &index : cube.indices)
        {
            file << index << " ";
        }
        file << "\n";
    }

    file.close();
}

glm::vec3 quatToEuler(glm::vec4 q)
{
    glm::vec3 euler;

    // Yaw (y-axis rotation)
    float siny_cosp = 2.0 * (q.w * q.z + q.x * q.y);
    float cosy_cosp = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
    euler.y = glm::atan(siny_cosp, cosy_cosp);

    // Pitch (x-axis rotation)
    float sinp = 2.0 * (q.w * q.y - q.z * q.x);
    if (abs(sinp) >= 1.0)
        euler.x = copysign(3.14159265358979323846 / 2.0, sinp); // use 90 degrees if out of range
    else
        euler.x = asin(sinp);

    // Roll (z-axis rotation)
    float sinr_cosp = 2.0 * (q.w * q.x + q.y * q.z);
    float cosr_cosp = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
    euler.z = glm::atan(sinr_cosp, cosr_cosp);

    return euler;
}

float radiansToDegrees(float radians)
{
    return radians * (180.0 / 3.14159265358979323846);
}

void SetRotsRadians(glm::vec3 &rots, glm::vec3 &rotsRadians, Cube *&cubeLookingAt)
{
    rots = quatToEuler(glm::vec4(cubeLookingAt->collider.orientation.x, cubeLookingAt->collider.orientation.y,
                                 cubeLookingAt->collider.orientation.z, cubeLookingAt->collider.orientation.w));
    rotsRadians = rots;

    rots.x = radiansToDegrees(rots.x);
    rots.y = radiansToDegrees(rots.y);
    rots.z = radiansToDegrees(rots.z);

    std::cout << rots.x << ", " << rots.y << ", " << rots.z << std::endl;
    std::cout << rotsRadians.x << ", " << rotsRadians.y << ", " << rotsRadians.z << std::endl << std::endl;
}

#pragma endregion

Cube cubeDummy(glm::vec3(0, 0, 0), 99, 0);
const unsigned int STRIDE = 10;
const unsigned int STRIDE2 = 6;
int cooldownForBreak = 0;
bool pinned = false;
double lastFrameTime = 0.0;
glm::vec3 spawnPoint = glm::vec3(0, 3, 0);
int health = 40;
int maxHealth = 40;
int healthCooldown = 0;
int energyCooldown = 0;
int energy = 10;
int maxEnergy = 10;
bool recharging = false;
bool needsRecharging = false;
int energyRechargeCooldown = 0;
int numOfFeetOnGround = 0;
bool levelSelect = false;
std::string levelText = "Unchosen";
bool amountOfLevels = 10;

// clang-format off
std::vector<const char*> descriptions =
{
    "Select a level to see information about it",
    "The starting point - Currently a work in progress!",
    "THIS LEVEL IS BLANK - IT HAS NOT BEEN STARTED YET",
    "THIS LEVEL IS BLANK - IT HAS NOT BEEN STARTED YET",
    "THIS LEVEL IS BLANK - IT HAS NOT BEEN STARTED YET",
    "THIS LEVEL IS BLANK - IT HAS NOT BEEN STARTED YET",
    "THIS LEVEL IS BLANK - IT HAS NOT BEEN STARTED YET",
    "THIS LEVEL IS BLANK - IT HAS NOT BEEN STARTED YET",
    "THIS LEVEL IS BLANK - IT HAS NOT BEEN STARTED YET",
    "THIS LEVEL IS BLANK - IT HAS NOT BEEN STARTED YET",


};
// clang-format on

const bool PRINTLOG = true;      // for debugging start of program
const bool PRINTLOOPLOG = false; // for debugging loop  in program

bool placingObject = false;
int objectOffset = -1;

void UpdateRotation(Cube *cubeLookingAt, std::vector<Cube> &voxels, glm::vec3 rotation)
{
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> pointsOriginal;
    glm::vec3 pos = glm::vec3(0);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[0];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[1];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[2];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[10];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[11];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[12];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[20];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[21];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[22];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[30];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[31];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[32];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[40];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[41];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[42];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[50];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[51];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[52];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[60];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[61];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[62];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[70];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[71];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[72];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[80];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[81];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[82];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[90];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[91];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[92];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[100];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[101];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[102];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[110];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[111];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[112];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[120];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[121];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[122];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[130];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[131];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[132];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[140];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[141];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[142];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[150];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[151];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[152];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[160];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[161];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[162];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[170];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[171];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[172];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[180];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[181];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[182];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[190];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[191];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[192];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[200];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[201];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[202];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[210];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[211];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[212];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[220];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[221];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[222];
    points.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositions[230];
    pos.y = voxels[cubeLookingAt->index].cornerPositions[231];
    pos.z = voxels[cubeLookingAt->index].cornerPositions[232];
    points.push_back(pos);

    ///////////////////////

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[0];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[1];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[2];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[10];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[11];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[12];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[20];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[21];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[22];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[30];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[31];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[32];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[40];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[41];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[42];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[50];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[51];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[52];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[60];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[61];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[62];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[70];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[71];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[72];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[80];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[81];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[82];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[90];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[91];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[92];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[100];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[101];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[102];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[110];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[111];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[112];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[120];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[121];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[122];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[130];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[131];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[132];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[140];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[141];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[142];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[150];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[151];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[152];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[160];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[161];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[162];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[170];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[171];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[172];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[180];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[181];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[182];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[190];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[191];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[192];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[200];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[201];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[202];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[210];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[211];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[212];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[220];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[221];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[222];
    pointsOriginal.push_back(pos);

    pos.x = voxels[cubeLookingAt->index].cornerPositionsOriginal[230];
    pos.y = voxels[cubeLookingAt->index].cornerPositionsOriginal[231];
    pos.z = voxels[cubeLookingAt->index].cornerPositionsOriginal[232];
    pointsOriginal.push_back(pos);

    for (int i = 0; i < points.size(); i++)
    {
        RotatePoint(points[i], voxels[cubeLookingAt->index].position, rotation, pointsOriginal[i]);
    }

    // Convert Euler angles to rotation matrix using XYZ order
    glm::mat4 rotationMatrix = glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);

    // Convert rotation matrix to quaternion
    glm::quat quaternion2 = glm::quat_cast(rotationMatrix);

    // Set the collider rotation using the quaternion
    voxels[cubeLookingAt->index].collider.setRotation(quaternion2);

    voxels[cubeLookingAt->index].cornerPositions[0] = points[0].x;
    voxels[cubeLookingAt->index].cornerPositions[1] = points[0].y;
    voxels[cubeLookingAt->index].cornerPositions[2] = points[0].z;

    voxels[cubeLookingAt->index].cornerPositions[10] = points[1].x;
    voxels[cubeLookingAt->index].cornerPositions[11] = points[1].y;
    voxels[cubeLookingAt->index].cornerPositions[12] = points[1].z;

    voxels[cubeLookingAt->index].cornerPositions[20] = points[2].x;
    voxels[cubeLookingAt->index].cornerPositions[21] = points[2].y;
    voxels[cubeLookingAt->index].cornerPositions[22] = points[2].z;

    voxels[cubeLookingAt->index].cornerPositions[30] = points[3].x;
    voxels[cubeLookingAt->index].cornerPositions[31] = points[3].y;
    voxels[cubeLookingAt->index].cornerPositions[32] = points[3].z;

    voxels[cubeLookingAt->index].cornerPositions[40] = points[4].x;
    voxels[cubeLookingAt->index].cornerPositions[41] = points[4].y;
    voxels[cubeLookingAt->index].cornerPositions[42] = points[4].z;

    voxels[cubeLookingAt->index].cornerPositions[50] = points[5].x;
    voxels[cubeLookingAt->index].cornerPositions[51] = points[5].y;
    voxels[cubeLookingAt->index].cornerPositions[52] = points[5].z;

    voxels[cubeLookingAt->index].cornerPositions[60] = points[6].x;
    voxels[cubeLookingAt->index].cornerPositions[61] = points[6].y;
    voxels[cubeLookingAt->index].cornerPositions[62] = points[6].z;

    voxels[cubeLookingAt->index].cornerPositions[70] = points[7].x;
    voxels[cubeLookingAt->index].cornerPositions[71] = points[7].y;
    voxels[cubeLookingAt->index].cornerPositions[72] = points[7].z;

    voxels[cubeLookingAt->index].cornerPositions[80] = points[8].x;
    voxels[cubeLookingAt->index].cornerPositions[81] = points[8].y;
    voxels[cubeLookingAt->index].cornerPositions[82] = points[8].z;

    voxels[cubeLookingAt->index].cornerPositions[90] = points[9].x;
    voxels[cubeLookingAt->index].cornerPositions[91] = points[9].y;
    voxels[cubeLookingAt->index].cornerPositions[92] = points[9].z;

    voxels[cubeLookingAt->index].cornerPositions[100] = points[10].x;
    voxels[cubeLookingAt->index].cornerPositions[101] = points[10].y;
    voxels[cubeLookingAt->index].cornerPositions[102] = points[10].z;

    voxels[cubeLookingAt->index].cornerPositions[110] = points[11].x;
    voxels[cubeLookingAt->index].cornerPositions[111] = points[11].y;
    voxels[cubeLookingAt->index].cornerPositions[112] = points[11].z;

    voxels[cubeLookingAt->index].cornerPositions[120] = points[12].x;
    voxels[cubeLookingAt->index].cornerPositions[121] = points[12].y;
    voxels[cubeLookingAt->index].cornerPositions[122] = points[12].z;

    voxels[cubeLookingAt->index].cornerPositions[130] = points[13].x;
    voxels[cubeLookingAt->index].cornerPositions[131] = points[13].y;
    voxels[cubeLookingAt->index].cornerPositions[132] = points[13].z;

    voxels[cubeLookingAt->index].cornerPositions[140] = points[14].x;
    voxels[cubeLookingAt->index].cornerPositions[141] = points[14].y;
    voxels[cubeLookingAt->index].cornerPositions[142] = points[14].z;

    voxels[cubeLookingAt->index].cornerPositions[150] = points[15].x;
    voxels[cubeLookingAt->index].cornerPositions[151] = points[15].y;
    voxels[cubeLookingAt->index].cornerPositions[152] = points[15].z;

    voxels[cubeLookingAt->index].cornerPositions[160] = points[16].x;
    voxels[cubeLookingAt->index].cornerPositions[161] = points[16].y;
    voxels[cubeLookingAt->index].cornerPositions[162] = points[16].z;

    voxels[cubeLookingAt->index].cornerPositions[170] = points[17].x;
    voxels[cubeLookingAt->index].cornerPositions[171] = points[17].y;
    voxels[cubeLookingAt->index].cornerPositions[172] = points[17].z;

    voxels[cubeLookingAt->index].cornerPositions[180] = points[18].x;
    voxels[cubeLookingAt->index].cornerPositions[181] = points[18].y;
    voxels[cubeLookingAt->index].cornerPositions[182] = points[18].z;

    voxels[cubeLookingAt->index].cornerPositions[190] = points[19].x;
    voxels[cubeLookingAt->index].cornerPositions[191] = points[19].y;
    voxels[cubeLookingAt->index].cornerPositions[192] = points[19].z;

    voxels[cubeLookingAt->index].cornerPositions[200] = points[20].x;
    voxels[cubeLookingAt->index].cornerPositions[201] = points[20].y;
    voxels[cubeLookingAt->index].cornerPositions[202] = points[20].z;

    voxels[cubeLookingAt->index].cornerPositions[210] = points[21].x;
    voxels[cubeLookingAt->index].cornerPositions[211] = points[21].y;
    voxels[cubeLookingAt->index].cornerPositions[212] = points[21].z;

    voxels[cubeLookingAt->index].cornerPositions[220] = points[22].x;
    voxels[cubeLookingAt->index].cornerPositions[221] = points[22].y;
    voxels[cubeLookingAt->index].cornerPositions[222] = points[22].z;

    voxels[cubeLookingAt->index].cornerPositions[230] = points[23].x;
    voxels[cubeLookingAt->index].cornerPositions[231] = points[23].y;
    voxels[cubeLookingAt->index].cornerPositions[232] = points[23].z;
}

std::vector<int> heartImages = {2, 3, 4, 5, 6};
std::vector<int> levelImages = {8, 9, 10, 11, 12, 13, 14, 15, 16, 15, 15, 15};

bool HasInInventory(std::string whatToLookFor)
{
    // Find the iterator pointing to "Red Key" (if it exists)
    auto it = std::find(inventory.begin(), inventory.end(), whatToLookFor);

    // Check if the iterator reached the end (not found)
    return it != inventory.end();
}

GLuint GetHeartImage(int health, std::vector<int> &heartImages)
{
    if (health <= 0)
        return heartImages[0];
    int index = (health % 4);
    return heartImages[index];
}

GLuint GetLevelImage(int level, std::vector<int> &levelImages)
{
    return levelImages[level];
}

void Damage(int amount, int &health, int &healthCooldown, int maxHealth)
{
    if (healthCooldown > 0)
        return;
    healthCooldown = 30;
    health -= amount;
    if (health < 1)
    {
        health = maxHealth;
        addNotification("YOU HAVE DIED", 100);
    }
}

void Damage(int amount)
{
    Damage(amount, health, healthCooldown, maxHealth);
}

void Sap(int amount, int &energy, int &energyCooldown, int &maxEnergy)
{
    if (energyCooldown > 0)
        return;
    energyCooldown = 1000;
    energy -= amount;
    if (energy <= 0)
    {
        recharging = true;
        needsRecharging = true;
        addNotification("YOU HAVE NO MORE ENERGY", 200);
    }
}

void Sap(int amount)
{
    Sap(amount, energy, energyCooldown, maxEnergy);
}

void HandleImpactSound(Cube &voxel, SoundManager &soundManager)
{
    if (((int)voxel.position.y == (int)voxel.positionHighest.y && voxel.playedSoundAlready == false) ||
        ((int)voxel.position.y == (int)voxel.positionLowest.y && voxel.playedSoundAlready == false))
    {
        soundManager.RegisterSoundEvent("impactSound", voxel.position);
        voxel.playedSoundAlready = true;
    }
    if ((int)voxel.position.y != (int)voxel.positionHighest.y && (int)voxel.position.y != (int)voxel.positionLowest.y &&
        voxel.playedSoundAlready == true)
    {
        voxel.playedSoundAlready = false;
    }
}

int main()
{
    SoundManager soundManager;

    sf::SoundBuffer blipSelectBuffer;
    if (!blipSelectBuffer.loadFromFile("res/sounds/blipSelect.wav"))
    {
        std::cout << "couldn't load SELECT sound" << std::endl;
    }
    sf::Sound selectSound;
    selectSound.setBuffer(blipSelectBuffer);

    sf::SoundBuffer stoneImpactBuffer;
    if (!stoneImpactBuffer.loadFromFile("res/sounds/stoneImpact.wav"))
    {
        std::cout << "couldn't load IMPACT sound" << std::endl;
    }
    sf::Sound impactSound;
    impactSound.setBuffer(stoneImpactBuffer);

    sf::SoundBuffer stoneSlideBuffer;
    if (!stoneSlideBuffer.loadFromFile("res/sounds/stoneSlide.wav"))
    {
        std::cout << "couldn't load SLIDE sound" << std::endl;
    }
    sf::Sound slideSound;
    slideSound.setBuffer(stoneSlideBuffer);

#pragma region INITIALIZATION

#pragma region MAPSETUP

    if (PRINTLOG)
        std::cout << "Building map from map.txt..." << std::endl;
    std::vector<Cube> voxels;
    std::vector<Object> objects;
    std::vector<Image> images;
    Image img(glm::vec3(.5f, .5f, .5f), glm::vec3(1, 1, 1));
    images.push_back(img);
    voxels.push_back(cubeDummy);

    voxels = LoadCubesFromFile("res/maps/mapVoxels" + std::to_string(currentLevel) + ".txt");
    lights = LoadLightsFromFile("res/maps/mapLights" + std::to_string(currentLevel) + ".txt");
    needsRefresh = true;

    if (voxels.size() == 0)
    {
        std::cout << "Missing base cube from save file! Attempting to create..." << std::endl;
        Cube cubeZero;
        cubeZero.textureIndex = 99;
        voxels.push_back(cubeZero);
    }

    if (PRINTLOG)
        std::cout << "Map built successfully! Constructing voxels from data..." << std::endl;
#pragma endregion MAPSETUP

#pragma region GLINIT
    const unsigned int AMOUNT_OF_INDICES = 36;
    const unsigned int AMOUNT_OF_INDICES2 = 6;
    const unsigned int FULL_STRIDE = STRIDE * AMOUNT_OF_INDICES * VertexBufferElement::GetSizeOfType(GL_FLOAT);
    const unsigned int FULL_STRIDE2 = STRIDE2 * AMOUNT_OF_INDICES2 * VertexBufferElement::GetSizeOfType(GL_FLOAT);
    int indicesCount = voxels.size() * AMOUNT_OF_INDICES;
    int indicesCount2 = images.size() * AMOUNT_OF_INDICES2;
    unsigned int *indicesAfter = new unsigned int[indicesCount];
    unsigned int *indicesAfter2 = new unsigned int[indicesCount2];
    for (int i = 0; i < voxels.size(); i++)
    {
        for (int j = 0; j < voxels[i].indices.size(); j++)
        {
            indicesAfter[i * AMOUNT_OF_INDICES + j] = voxels[i].indices[j] + i * AMOUNT_OF_INDICES;
        }
    }
    for (int i = 0; i < images.size(); i++)
    {
        for (int j = 0; j < images[i].indices.size(); j++)
        {
            indicesAfter2[i * AMOUNT_OF_INDICES2 + j] = images[i].indices[j] + i * AMOUNT_OF_INDICES2;
        }
    }

    if (PRINTLOG)
        std::cout << "Math for buffers done, creating vectors!" << std::endl;

    // this is the size of each tri's info (6, 3 for position, 2 for texture coordinates, 1 textureID) * 36 (number
    // of indices in our cube)
    float *positions = new float[(voxels.size() * STRIDE * AMOUNT_OF_INDICES)];
    float *positionsUI = new float[(images.size() * STRIDE2 * AMOUNT_OF_INDICES2)];

    for (int i = 0; i < voxels.size(); i++)
    {
        voxels[i].collider.setSize(voxels[i].scale);
        voxels[i].collider.setPosition(voxels[i].position);
        for (int j = 0; j < voxels[i].cornerPositions.size(); j++)
        {
            positions[i * STRIDE * AMOUNT_OF_INDICES + j] = voxels[i].cornerPositions[j];

            if (j % STRIDE == 0)
            {
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.x;
            }
            else if (j % STRIDE == 1)
            {
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.y;
            }
            else if (j % STRIDE == 2)
            {
                positions[i * STRIDE * AMOUNT_OF_INDICES + j] += voxels[i].position.z;
            }
        }
    }

    for (int i = 0; i < images.size(); i++)
    {
        for (int j = 0; j < images[i].cornerPositions.size(); j++)
        {
            positionsUI[i * STRIDE2 * AMOUNT_OF_INDICES2 + j] = images[i].cornerPositions[j];

            if (j % STRIDE2 == 0)
            {
                positionsUI[i * STRIDE2 * AMOUNT_OF_INDICES2 + j] += images[i].position.x;
            }
            else if (j % STRIDE2 == 1)
            {
                positionsUI[i * STRIDE2 * AMOUNT_OF_INDICES2 + j] += images[i].position.y;
            }
            else if (j % STRIDE2 == 2)
            {
                positionsUI[i * STRIDE2 * AMOUNT_OF_INDICES2 + j] += images[i].position.z;
            }
        }
    }

    if (PRINTLOG)
        std::cout << "Initialising GLFW and GL3W..." << std::endl;

    if (!glfwInit())
    {
        std::cerr << "FAILED to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(width, height, "DDGL", NULL, NULL);
    if (!window)
    {
        std::cerr << "FAILED to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "FAILED to initialize GLEW\n";
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    if (PRINTLOG)
        std::cout << "Set depth-testing to true\nSet blending to true" << std::endl;

    if (PRINTLOG)
        std::cout << "GLFW and GL3W initialised! Setting up VBO/VAO!" << std::endl;

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if (PRINTLOG)
        std::cout << "Finally binding VAO..." << std::endl;

#pragma endregion GLINIT

#pragma region LAYOUT

    int num = 0;
    for (int i = 0; i < objects.size(); i++)
        num += objects[i].voxels.size();

    VertexArray va(1);
    if (PRINTLOG)
        std::cout << "Finally creating vertex buffer from info..." << std::endl;

    VertexBuffer vb(positions, (voxels.size() * FULL_STRIDE));
    if (PRINTLOG)
        std::cout << "Finally creating vertex layout from info..." << std::endl;
    VertexBufferLayout layout;

    if (PRINTLOG)
        std::cout << "VBO/VAO set up! creating index buffer!" << std::endl;

    layout.Push(GL_FLOAT, 3); // position
    layout.Push(GL_FLOAT, 2); // texture coordinates
    layout.Push(GL_FLOAT, 1); // textureID
    layout.Push(GL_FLOAT, 3); // normal
    layout.Push(GL_FLOAT, 1); // cubeLookingAt
    va.AddBuffer(vb, layout);

    IndexBuffer ib(indicesAfter, indicesCount);

#pragma endregion LAYOUT

#pragma region GLINIT2

    glm::mat4 proj = glm::perspective(glm::radians(camera.fov), (float)width / (float)height, 0.1f, 5000.0f);
    glm::mat4 view = glm::lookAt(camera.position, camera.target, glm::vec3(0, 1, 0));
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = proj * view * model;

    if (PRINTLOG)
        std::cout << "Voxels generated! Creating shaders..." << std::endl;
    Shader shaderUI("res/shaders/UI.shader");

    Shader shader("res/shaders/Basic.shader");

    shader.Bind();
    shader.SetUniformMat4f("u_MVP", mvp);
    shaderUI.SetUniformMat4f("u_MVP", mvp);
    if (PRINTLOG)
        std::cout << "Bound shaders successfully! Binding textures to GPU...\n";

    Texture u_TextureAtlas("res/textures/atlas.png");
    Texture u_Heart0("res/textures/heart0.png");
    Texture u_Heart1("res/textures/heart1.png");
    Texture u_Heart2("res/textures/heart2.png");
    Texture u_Heart3("res/textures/heart3.png");
    Texture u_Heart4("res/textures/heart4.png");
    Texture u_Energy("res/textures/energy.png");
    Texture u_Level0("res/textures/level0.png");
    Texture u_Level1("res/textures/level1.png");
    Texture u_Level2("res/textures/level2.png");
    Texture u_Level3("res/textures/level3.png");
    Texture u_Level4("res/textures/level4.png");
    Texture u_Level5("res/textures/level5.png");
    Texture u_Level6("res/textures/level6.png");
    Texture u_Level7("res/textures/level7.png");
    Texture u_Level8("res/textures/level8.png");
    Texture u_Level9("res/textures/level9.png");

    u_TextureAtlas.Bind(0);
    u_Heart0.Bind(1);
    u_Heart0.Bind(2);
    u_Heart0.Bind(3);
    u_Heart0.Bind(4);
    u_Heart0.Bind(5);
    u_Energy.Bind(6);
    u_Level0.Bind(7);
    u_Level1.Bind(8);
    u_Level2.Bind(9);
    u_Level3.Bind(10);
    u_Level4.Bind(11);
    u_Level5.Bind(12);
    u_Level6.Bind(13);
    u_Level7.Bind(14);
    u_Level8.Bind(15);
    u_Level9.Bind(16);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (PRINTLOG)
        std::cout << "Textures bound! Setting texture uniforms..." << std::endl;

    shader.SetUniform1i("u_TextureAtlas", 0);
    shaderUI.SetUniform1f("u_Heart0", 1);

    va.Unbind();
    vb.Unbind();
    ib.Unbind();
    shader.Unbind();

    if (PRINTLOG)
        std::cout << "Texture uniforms created! Creating RENDERER..." << std::endl;

    Renderer renderer;

    if (PRINTLOG)
        std::cout << "RENDERER created! Initializing IMGUI..." << std::endl;

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize.x = static_cast<float>(display_w);
    io.DisplaySize.y = static_cast<float>(display_h);

    float dt = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame;
    bool paused = false;
    shader.Bind();

    if (PRINTLOG)
        std::cout << "SETUP complete!" << std::endl;

    Cube *cubeLookingAt = nullptr;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

#pragma endregion GLINIT2

#pragma endregion INITIALIZATION

    while (!glfwWindowShouldClose(window))
    {
        soundManager.Update(camera.position, camera.getViewDirection());
        va.Bind();
        vb.Bind();
        ib.Bind();
        shader.Bind();
        if (healthCooldown > 0)
            healthCooldown--;
        if (energyCooldown > 0)
            energyCooldown--;
        if (showDebugCooldown > 0)
            showDebugCooldown--;
        if (needsRecharging)
            recharging = true;
        if (recharging)
        {
            energyRechargeCooldown++;
            if (energyRechargeCooldown > 144)
            {
                energy++;
                energyRechargeCooldown = 0;
            }
            if (energy == maxEnergy)
            {
                recharging = false;
                needsRecharging = false;
            }
        }

        if (PRINTLOOPLOG)
            std::cout << "checking lights..." << std::endl;

        for (int i = 0; i < lights.size(); i++)
        {
            lights[i]->position.x = (int)lights[i]->position.x;
            lights[i]->position.y = (int)lights[i]->position.y;
            lights[i]->position.z = (int)lights[i]->position.z;
        }

        if (PRINTLOOPLOG)
            std::cout << "checking frame times..." << std::endl;

        double currentTime = glfwGetTime(); // Get the current time
        dt = currentTime - lastFrameTime;   // Calculate the delta time
        lastFrameTime = currentTime;        // Update lastFrameTime to the current time

        float distToCube = 99999999;
        needsRefresh = true;
        if (PRINTLOOPLOG)
            std::cout << "starting rendering..." << std::endl;

        glfwSwapInterval(1);

        renderer.Clear();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (needsRefresh)
        {
            Refresh(indicesCount, voxels, AMOUNT_OF_INDICES, indicesAfter, positions, layout, va, vb, ib, FULL_STRIDE,
                    PRINTLOOPLOG, STRIDE, AMOUNT_OF_INDICES2, indicesAfter2, images, indicesCount2, FULL_STRIDE2);
        }

        if (PRINTLOOPLOG)
            std::cout << "doing matrix transformations..." << std::endl;

#pragma region MVP
        ImGui_ImplOpenGL3_NewFrame();

        glm::mat4 proj = glm::perspective(glm::radians(camera.fov), (float)width / (float)height, 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(camera.position, camera.target, glm::vec3(0, 1, 0));
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = proj * view * model;
        shader.SetUniform1f("ambientLight", ambientLightLevel);
        shader.SetUniformMat4f("u_MVP", mvp);
        int numLights = lights.size();

        for (int i = 0; i < numLights; i++)
        {
            shader.SetUniform3f(("lightPos[" + std::to_string(i) + "]").c_str(), lights[i]->position.x,
                                lights[i]->position.y, lights[i]->position.z);
            shader.SetUniform4f(("lightColor[" + std::to_string(i) + "]").c_str(), lights[i]->rgba.x, lights[i]->rgba.y,
                                lights[i]->rgba.z, lights[i]->rgba.w);
            shader.SetUniform1f(("lightBrightness[" + std::to_string(i) + "]").c_str(), lights[i]->brightness);

            lights[i]->Update();
        }
        shader.SetUniform1i("numLights", numLights);
        renderer.Draw(va, ib, shader);

        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

#pragma endregion MVP

        if (PRINTLOOPLOG)
            std::cout << "done with transformations, handling collision now!" << std::endl;

#pragma region COLLISION

        if (PRINTLOOPLOG)
            std::cout << "setting up 'near player' vector" << std::endl;

        bool onGround = false;

        if (PRINTLOOPLOG)
            std::cout << "updating camera" << std::endl;

        camera.Update(window, dt, mouseControl, energy, recharging, needsRecharging);
        camera.collider.setPosition(camera.position);
        float val = camera.heighte;
        glm::vec3 val2(val / 4, val * 2, val / 4);
        camera.collider.setSize(val2);

        if (PRINTLOOPLOG)
            std::cout << "handling points to check" << std::endl;

        std::vector<glm::vec3> pointsToCheck;

        for (int j = 4; j < 5; j++)
        {
            glm::vec3 pointToCheck = CastPointForward((j + 1) * 2, camera.GetPosition());
            pointsToCheck.push_back(pointToCheck);
        }

        if (PRINTLOOPLOG)
            std::cout << "getting cursor position " << std::endl;

        glm::vec3 cursorPos = CastPointForward(10, camera.GetPosition());
        cursorPos.x = (int)cursorPos.x;

        cursorPos.y = (int)cursorPos.y;

        cursorPos.z = (int)cursorPos.z;

        if (PRINTLOOPLOG)
            std::cout << "updating 3d cursor" << std::endl;

        if (voxels.size() > 0)
        {
            voxels[0].position.x = cursorPos.x;
            voxels[0].position.y = cursorPos.y;
            voxels[0].position.z = cursorPos.z;
        }
        if (PRINTLOOPLOG)
            std::cout << "updating vertex buffer" << std::endl;

        vb.UpdateScale(0, cursorPos, 1, 1, 1, STRIDE);

        if (!paused && !pinned)
            cubeLookingAt = nullptr;

        if (PRINTLOOPLOG)
            std::cout << "resolving collisions finally" << std::endl;
        numOfFeetOnGround = 0;

        for (int i = 0; i < voxels.size(); i++)
        {
            float distanceToCamera = glm::distance(camera.position, voxels[i].position);

            if (!camera.isFlying && voxels[i].door)
            {
                HandleImpactSound(voxels[i], soundManager);
                if (voxels[i].requiresRedKey)
                {
                    if (HasInInventory("Red Key"))
                    {

                        float distanceBetweenDoorAndPlayer =
                            glm::distance(glm::vec3(voxels[i].position.x, voxels[i].position.y - voxels[i].scale.y,
                                                    voxels[i].position.z),
                                          camera.position);
                        if (distanceBetweenDoorAndPlayer < 10)
                        {
                            addNotification("Red Key found in Inventory!", 1);
                            voxels[i].Increase(slideSound);
                            if (voxels[i].position.y < voxels[i].positionHighest.y - .1f)
                                soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                        }
                        else
                        {
                            voxels[i].Decrease();
                            if (voxels[i].position.y > voxels[i].positionLowest.y + .1f)
                                soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                        }
                    }
                    else
                    {
                        voxels[i].Decrease();
                        if (voxels[i].position.y > voxels[i].positionLowest.y + .1f)
                            soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                    }
                }

                else if (voxels[i].requiresGreenKey)
                {
                    if (HasInInventory("Green Key"))
                    {

                        float distanceBetweenDoorAndPlayer =
                            glm::distance(glm::vec3(voxels[i].position.x, voxels[i].position.y - voxels[i].scale.y,
                                                    voxels[i].position.z),
                                          camera.position);
                        if (distanceBetweenDoorAndPlayer < 10)
                        {
                            addNotification("Green Key found in Inventory!", 1);
                            voxels[i].Increase(slideSound);
                            if (voxels[i].position.y < voxels[i].positionHighest.y - .1f)
                                soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                        }
                        else
                        {
                            voxels[i].Decrease();
                            if (voxels[i].position.y > voxels[i].positionLowest.y + .1f)
                                soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                        }
                    }
                    else
                    {
                        voxels[i].Decrease();
                        if (voxels[i].position.y > voxels[i].positionLowest.y + .1f)
                            soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                    }
                }

                else if (voxels[i].requiresBlueKey)
                {
                    if (HasInInventory("Blue Key"))
                    {
                        float distanceBetweenDoorAndPlayer =
                            glm::distance(glm::vec3(voxels[i].position.x, voxels[i].position.y - voxels[i].scale.y,
                                                    voxels[i].position.z),
                                          camera.position);
                        if (distanceBetweenDoorAndPlayer < 10)
                        {
                            addNotification("Blue Key found in Inventory!", 1);
                            voxels[i].Increase(slideSound);
                            if (voxels[i].position.y < voxels[i].positionHighest.y - .1f)
                                soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                        }
                        else
                        {
                            voxels[i].Decrease();
                            if (voxels[i].position.y > voxels[i].positionLowest.y + .1f)
                                soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                        }
                    }
                    else
                    {
                        voxels[i].Decrease();
                        if (voxels[i].position.y > voxels[i].positionLowest.y + .1f)
                            soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                    }
                }

                else
                {
                    float distanceBetweenDoorAndPlayer = glm::distance(
                        glm::vec3(voxels[i].position.x, voxels[i].position.y - voxels[i].scale.y, voxels[i].position.z),
                        camera.position);
                    if (distanceBetweenDoorAndPlayer < 10)
                    {
                        voxels[i].Increase(slideSound);
                        if (voxels[i].position.y < voxels[i].positionHighest.y - .1f)
                            soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                    }
                    else
                    {
                        voxels[i].Decrease();
                        if (voxels[i].position.y > voxels[i].positionLowest.y + .1f)
                            soundManager.RegisterSoundEvent("slideSound", voxels[i].position);
                    }
                }
            }
            if (distanceToCamera > 15)
                continue;
            glm::vec3 radiansRotation = glm::radians(voxels[i].rotation);
            glm::mat4 rotationMatrix = glm::eulerAngleXYZ(radiansRotation.x, radiansRotation.y, radiansRotation.z);

            glm::quat quaternion2 = glm::quat_cast(rotationMatrix);

            voxels[i].collider.setRotation(quaternion2);
            bool colliding = false;
            // check if the camera's collider is colliding with the cube's collider
            if (!camera.isFlying && voxels[i].index != 0 && !voxels[i].invisible &&
                camera.collider.CheckCollision(voxels[voxels[i].index].collider))
            {
                colliding = true;
                glm::vec3 buffer = camera.collider.ResolveCollision(voxels[voxels[i].index].collider);

                float highestYOfBlock = voxels[voxels[i].index].position.y + ((voxels[voxels[i].index].scale.y));

                camera.position -= buffer;
                if (buffer.y != 0)
                {
                    onGround = true;
                    camera.onGround = true;
                    camera.isJumping = false;
                    camera.yVelocity = 0;
                }
                if (buffer.x != 0 || buffer.z != 0)
                {
                    float distanceOf = std::abs(camera.positionFeet.y - highestYOfBlock);
                    if (onGround && distanceOf <= stepHeight)
                        camera.position.y += distanceOf;
                }

                // camera.position.y = (camera.position.y * 100.0f) / 100.0f;
                if (voxels[i].liftingPlatform)
                {
                    voxels[i].Increase(slideSound);
                }
            }

            if (voxels[i].liftingPlatform && !camera.collider.CheckCollision(voxels[voxels[i].index].collider))
            {
                voxels[i].Decrease();
            }

            bool isChosen = false;
            for (int j = 0; j < pointsToCheck.size(); j++) // the 6 is how many steps the ray is cast forward
            {
                if (voxels[i].index != 0 && !voxels[i].invisible && voxels[i].isPointInside(cursorPos))
                {
                    float distance = glm::distance(camera.position, voxels[i].position);
                    if (distance < distToCube)
                    {
                        distToCube = distance;
                        cubeLookingAt = &voxels[i];
                    }
                }
            }
        }

        // MUST BE RIGHT AFTER COLLISION/CORRECTION VVVVVV
        camera.target = camera.position + glm::vec3(cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch)),
                                                    sin(glm::radians(camera.pitch)),
                                                    sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch)));

        if (!onGround)
        {
            camera.onGround = false;
        }

#pragma endregion COLLISION

        if (PRINTLOOPLOG)
            std::cout << "done with collisions, handling keypresses/mouse input now!" << std::endl;

#pragma region KEYPRESSES
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            paused = true;
            mouseControl = true;
        }

        if (showDebugCooldown == 0 && glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
        {
            showDebugCooldown = 30;
            showDebugInfo = !showDebugInfo;
            selectSound.play();
        }

        if (camera.position.y < -50)
        {
            Damage(1);
            camera.position = camera.lastPosition;
            camera.yVelocity = 0;
            camera.velocity = glm::vec3(0);
            addNotification(
                "YOU HAVE FALLEN UNCONSCIOUS AND MYSTERIOUSLY WOKEN\nUP BACK WHERE YOU STARTED, SOMEWHAT HURT", 1000);
        }

        if (!camera.isFlying && camera.isRunning)
        {
            recharging = false;
            Sap(1);
        }

        //  if left mouse is pressed
        if (camera.isFlying && !paused && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (pinned)
            {
                addNotification("BREAKING NOT ALLOWED WHILE PINNED", 100);
            }
            else
            {

                cooldownForBreak++;
                if (cooldownForBreak > 5)
                {
                    if (cubeLookingAt != nullptr)
                    {
                        if (/*cubeLookingAt->index >= 119 && */ cubeLookingAt->index < voxels.size())
                        {
                            voxels.erase(voxels.begin() + cubeLookingAt->index);
                        }
                        else
                        {
                            addNotification("VOXEL PROTECTED FROM BREAKING-", 30);
                        }
                        for (int i = 0; i < voxels.size(); i++)
                        {
                            voxels[i].index = i;
                        }

                        needsRefresh = true;
                        /*cubeLookingAt->SetInvisible();
                        voxels[cubeLookingAt->index].invisible = true;
                        vb.UpdateChosen((cubeLookingAt->index), 1, STRIDE);*/
                    }

                    int vx = (int)voxels[0].position.x;
                    int vy = (int)voxels[0].position.y;
                    int vz = (int)voxels[0].position.z;

                    for (int i = 0; i < lights.size(); i++)
                    {
                        int x = (int)lights[i]->position.x;
                        int y = (int)lights[i]->position.y;
                        int z = (int)lights[i]->position.z;
                        if (vx == x && vy == y && vz == z)
                        {
                            lights.erase(lights.begin() + i);
                            needsRefresh = true;
                        }
                    }
                    cooldownForBreak = 0;
                }
            }
        }

        if (camera.isFlying && !paused && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            cooldownForBreak++;
            if (cooldownForBreak > 5)
            {
                glm::vec3 positionToCheck =
                    glm::vec3((int)voxels[0].position.x, voxels[0].position.y, voxels[0].position.z);

                bool found = false;

                for (int i = 1; i < voxels.size(); i++)
                {
                    if (voxels[i].position == positionToCheck)
                        found = true;
                }

                if (!found /* && currentLevel != 0*/)

                {
                    if (chosenTextureID != 99)
                    {
                        voxels.push_back(
                            Cube(glm::vec3((int)voxels[0].position.x, voxels[0].position.y, voxels[0].position.z),
                                 (chosenTextureID), 0));
                        voxels[voxels.size() - 1].index = (voxels.size() - 1);
                        voxels[voxels.size() - 1].scale = sizeToSave;
                        voxels[voxels.size() - 1].collider.size = sizeToSave;
                        vb.UpdateScale(voxels.size() - 1,
                                       glm::vec3(voxels[0].position.x, voxels[0].position.y, voxels[0].position.z),
                                       sizeToSave.x, sizeToSave.y, sizeToSave.z, STRIDE);

                        needsRefresh = true;
                        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                        {
                            voxels[0].scale = glm::vec3(1, 1, 1);
                            vb.UpdateScale(0, voxels[0].position, sizeToSave.x, sizeToSave.y, sizeToSave.z, STRIDE);
                            voxels[0].collider.setSize(sizeToSave);
                        }
                    }
                    else
                    {
                        bool foundLight = false;
                        for (int i = 0; i < lights.size(); i++)
                        {
                            lights[i]->position.x = std::round(lights[i]->position.x);
                            lights[i]->position.y = std::round(lights[i]->position.y);
                            lights[i]->position.z = std::round(lights[i]->position.z);
                            if (lights[i]->position.x == voxels[0].position.x &&
                                lights[i]->position.y == voxels[0].position.y &&
                                lights[i]->position.z == voxels[0].position.z)
                                foundLight = true;
                        }

                        if (!foundLight && lights.size() < 256)
                        {

                            lights.push_back(new Light(glm::vec3((int)voxels[0].position.x, (int)voxels[0].position.y,
                                                                 (int)voxels[0].position.z)));
                            voxels.push_back(
                                Cube(glm::vec3((int)voxels[0].position.x, voxels[0].position.y, voxels[0].position.z),
                                     chosenTextureID, 0));
                            voxels[voxels.size() - 1].index = (voxels.size() - 1);
                            needsRefresh = true;
                        }
                    }
                }

                /*if (currentLevel == 0)
                    addNotification("Building is not allowed in the spawn level! (level 0)", 30);*/

                /*float cornerPositions[240];
                std::memcpy(cornerPositions, voxels[voxels.size() - 1].cornerPositions, 240 * sizeof(float));
                vb.AddVoxel(voxels[voxels.size() - 1].index, voxels.size(), STRIDE, cornerPositions);*/
            }
        }

        if (camera.isFlying && !paused && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS &&
            glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            if (cubeLookingAt != nullptr)
            {
                chosenTextureID = cubeLookingAt->textureIndex;
                sizeToSave = cubeLookingAt->scale;
                voxels[0].scale = sizeToSave;
                vb.UpdateScale(0, voxels[0].position, sizeToSave.x, sizeToSave.y, sizeToSave.z, STRIDE);
                voxels[0].collider.setSize(sizeToSave);
                addNotification("Copied voxel!", 10);
            }
        }

        if (camera.isFlying && !paused && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS &&
            glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        {
            if (cubeLookingAt != nullptr)
            {
                cubeLookingAt->textureIndex = chosenTextureID;
                vb.UpdateTexture(cubeLookingAt->index, chosenTextureID, STRIDE);
                vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, sizeToSave.x, sizeToSave.y, sizeToSave.z,
                               STRIDE);
                cubeLookingAt->collider.setSize(sizeToSave);
                voxels[0].scale = glm::vec3(1, 1, 1);
                vb.UpdateScale(0, voxels[0].position, sizeToSave.x, sizeToSave.y, sizeToSave.z, STRIDE);
                voxels[0].collider.setSize(sizeToSave);
                addNotification("Pasted voxel!", 10);
            }
        }
#pragma endregion KEYPRESSES

        if (PRINTLOOPLOG)
            std::cout << "starting IMGUI rendering!" << std::endl;

#pragma region IMGUI

        ImGui::NewFrame();

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;
        if (showDebugInfo)
        {

#pragma region CameraInfo
            ImGui::Begin("CAMERA", NULL,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);

            // Title
            ImGui::Text("Camera Information - F1 TO TOGGLE");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();

            // Position
            ImGui::Text("Position:");
            ImGui::Text("  X: %d", (int)camera.position.x);
            ImGui::Text("  Y: %d", (int)camera.position.y);
            ImGui::Text("  Z: %d", (int)camera.position.z);
            ImGui::Spacing();
            ImGui::Spacing();

            // Orientation
            ImGui::Text("Orientation:");
            ImGui::Text("  Yaw: %d", (int)camera.yaw);
            ImGui::Text("  Pitch: %d", (int)camera.pitch);
            ImGui::Text("  Roll: %d", (int)camera.roll);
            ImGui::Spacing();
            ImGui::Spacing();

            // Flags
            ImGui::Text("Status Flags:");
            ImGui::Text("  Mouse Control: %s", mouseControl ? "True" : "False");
            ImGui::Text("  On Ground: %s", onGround ? "True" : "False");
            ImGui::Text("  Jumping: %s", camera.isJumping ? "True" : "False");
            ImGui::Spacing();
            ImGui::Spacing();

            // Velocity
            ImGui::Text("Velocity:");
            ImGui::Text("  Y Velocity: %.1f", camera.yVelocity);
            ImGui::Spacing();

            const int sliderCenter = 0; // Initial value set to center
            int sliderValueYs = camera.fov;

            if (ImGui::Button("<##FOV"))
            {
                if (camera.fov > 25)
                {
                    camera.fov--;
                }
            }
            ImGui::SameLine();
            if (ImGui::DragInt("FOV", &sliderValueYs, 1.0f, 25, 180))
            {
                camera.fov = sliderValueYs;
            }
            ImGui::SameLine();
            if (ImGui::Button(">##FOV"))
            {
                if (camera.fov < 180)
                {
                    camera.fov++;
                }
            }

            ImGui::Spacing();

            float sliderValueYe = camera.heighte;

            if (ImGui::Button("<##Height"))
            {
                if (camera.heighte > .1f)
                {
                    camera.heighte -= 0.1f;
                }
            }
            ImGui::SameLine();
            if (ImGui::DragFloat("Height", &sliderValueYe, .1f, .1f, 10.f))
            {
                camera.heighte = sliderValueYe;
            }
            ImGui::SameLine();
            if (ImGui::Button(">##Height"))
            {
                if (camera.heighte < 10.f)
                {
                    camera.heighte += 0.1f;
                }
            }

            ImGui::Spacing();

            float sliderValueYz = ambientLightLevel;

            if (ImGui::Button("<##Ambient Light"))
            {
                if (ambientLightLevel > 0)
                {
                    ambientLightLevel -= 0.01;
                }
            }
            ImGui::SameLine();
            if (ImGui::DragFloat("Ambient Light", &sliderValueYz, 0.01f, 0, 1))
            {
                ambientLightLevel = sliderValueYz;
            }
            ImGui::SameLine();
            if (ImGui::Button(">##Ambient Light"))
            {
                if (ambientLightLevel < 1)
                {
                    ambientLightLevel += 0.01;
                }
            }

            ImGui::Spacing();

            if (ImGui::Checkbox("Flying?", &camera.isFlying))
            {
            }

            ImGui::End();
#pragma endregion CameraInfo

#pragma region SceneInfo

            ImGui::Begin("SceneInfo", NULL, window_flags);
            ImGui::Text("Objects: %lu", voxels.size());
            ImGui::Text("Indices: %d", indicesCount);
            ImGui::Text("Triangles: %lu", voxels.size() * 12);
            ImGui::Text("Lights: %lu", lights.size());
            ImGui::Text("Images: %lu", images.size());
            static float fps = 0.0f;
            static float lastFrameTime = 0.0f;
            const float now = glfwGetTime();
            fps = 1.0f / (now - lastFrameTime);
            lastFrameTime = now;
            ImGui::Text("FPS: %.1f", fps);
            ImGui::End();

#pragma endregion SceneInfo
        }

#pragma region Health

        int numHearts = maxHealth / 4;
        ImGui::SetNextWindowPos(ImVec2(20, 10));
        ImGui::SetNextWindowSize(ImVec2(30 * numHearts, 30));
        ImGui::Begin("Health", NULL, window_flags);

        for (int i = 0; i < numHearts; ++i)
        {
            int currentHealth = health - (i * 4);
            if (currentHealth > 0)
            {
                GLuint imageID = (currentHealth >= 4) ? heartImages[4] : GetHeartImage(currentHealth, heartImages);
                ImGui::Image((void *)(intptr_t)imageID, ImVec2(25, 25));
            }
            if (i < numHearts - 1)
            {
                ImGui::SameLine();
            }
        }
        ImGui::End();
#pragma endregion Health

#pragma region Energy

        int numEnergy = 10;
        ImGui::SetNextWindowPos(ImVec2(20, 35));
        ImGui::SetNextWindowSize(ImVec2(30 * numEnergy, 30));
        ImGui::Begin("Energy", NULL, window_flags);

        for (int i = 0; i < numEnergy; ++i)
        {
            if (i > energy)
                break;
            if (energy > 0)
            {
                GLuint imageID = 7;
                ImGui::Image((void *)(intptr_t)imageID, ImVec2(25, 25));
            }
            if (i < numEnergy - 1)
            {
                ImGui::SameLine();
            }
        }
        ImGui::End();
#pragma endregion Energy

#pragma region CubeLookingAt

        if (cubeLookingAt != nullptr)
        {

            ImGui::Begin("CubeLookingAt", NULL,
                         /*ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration*/
                         ImGuiWindowFlags_NoCollapse);
            ImGui::Checkbox("Pin", &pinned);
            ImGui::SameLine();
            ImGui::Text("VOXEL %d", cubeLookingAt->index);
            ImGui::Checkbox("Door?", &cubeLookingAt->door);
            ImGui::Checkbox("Requires Red Key?", &cubeLookingAt->requiresRedKey);
            ImGui::Checkbox("Requires Blue Key?", &cubeLookingAt->requiresBlueKey);
            ImGui::Checkbox("Requires Green Key?", &cubeLookingAt->requiresGreenKey);
            ImGui::Checkbox("soundNeedsPlayed?", &cubeLookingAt->needsSoundPlayed);
            ImGui::Checkbox("PlayedSoundAlready?", &cubeLookingAt->playedSoundAlready);
            ImGui::Spacing();
            ImGui::Text("Position:                    (%d,   %d,   %d)", (int)cubeLookingAt->position.x,
                        (int)cubeLookingAt->position.y, (int)cubeLookingAt->position.z);
            ImGui::Text("PositionH:                    (%d,   %d,   %d)", (int)cubeLookingAt->positionHighest.x,
                        (int)cubeLookingAt->positionHighest.y, (int)cubeLookingAt->positionHighest.z);
            ImGui::Text("PositionL:                    (%d,   %d,   %d)", (int)cubeLookingAt->positionLowest.x,
                        (int)cubeLookingAt->positionLowest.y, (int)cubeLookingAt->positionLowest.z);
            ImGui::Text("Scale   :                    (%.1f, %.1f, %.1f)", cubeLookingAt->scale.x,
                        cubeLookingAt->scale.y, cubeLookingAt->scale.z);
            ImGui::Spacing();

            if (cubeLookingAt->textureIndex != 99)
            {
                int xPos = cubeLookingAt->position.x;
                int yPos = cubeLookingAt->position.y;
                int zPos = cubeLookingAt->position.z;

                int textureIDs = cubeLookingAt->textureIndex;

                const int sliderCenter = 0;      // Center value for the slider
                int sliderValueX = sliderCenter; // Initial value set to center
                int sliderValueY = sliderCenter; // Initial value set to center
                int sliderValueZ = sliderCenter;

                if (ImGui::Button("<##PositionX"))
                {

                    xPos--;
                    cubeLookingAt->position.x = xPos;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x,
                                   cubeLookingAt->scale.y, cubeLookingAt->scale.z, STRIDE);
                    voxels[cubeLookingAt->index].collider.setPosition(cubeLookingAt->position);
                }
                ImGui::SameLine();
                if (ImGui::DragInt("  Pos X", &sliderValueX, 2.0f, -2, 2))
                {
                    cubeLookingAt->position.x += sliderValueX;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x,
                                   cubeLookingAt->scale.y, cubeLookingAt->scale.z, STRIDE);
                    voxels[cubeLookingAt->index].collider.setPosition(cubeLookingAt->position);
                    sliderValueX = sliderCenter; // Reset to center
                    glm::vec3 colliderScale =
                        glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                    voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                }
                ImGui::SameLine();
                if (ImGui::Button(">##PositionX"))
                {

                    xPos++;
                    cubeLookingAt->position.x = xPos;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x,
                                   cubeLookingAt->scale.y, cubeLookingAt->scale.z, STRIDE);
                    voxels[cubeLookingAt->index].collider.setPosition(cubeLookingAt->position);
                }

                if (ImGui::Button("<##PositionY"))
                {

                    yPos--;
                    cubeLookingAt->position.y = yPos;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x,
                                   cubeLookingAt->scale.y, cubeLookingAt->scale.z, STRIDE);
                    voxels[cubeLookingAt->index].collider.setPosition(cubeLookingAt->position);
                }
                ImGui::SameLine();
                if (ImGui::DragInt("  Pos Y", &sliderValueY, 2.0f, -2, 2))
                {
                    cubeLookingAt->position.y += sliderValueY;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x,
                                   cubeLookingAt->scale.y, cubeLookingAt->scale.z, STRIDE);
                    voxels[cubeLookingAt->index].collider.setPosition(cubeLookingAt->position);
                    sliderValueY = sliderCenter; // Reset to center
                    glm::vec3 colliderScale =
                        glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                    voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                }
                ImGui::SameLine();
                if (ImGui::Button(">##PositionY"))
                {

                    yPos++;
                    cubeLookingAt->position.y = yPos;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x,
                                   cubeLookingAt->scale.y, cubeLookingAt->scale.z, STRIDE);
                    voxels[cubeLookingAt->index].collider.setPosition(cubeLookingAt->position);
                }

                if (ImGui::Button("<##PositionZ"))
                {

                    zPos--;
                    cubeLookingAt->position.z = zPos;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x,
                                   cubeLookingAt->scale.y, cubeLookingAt->scale.z, STRIDE);
                    voxels[cubeLookingAt->index].collider.setPosition(cubeLookingAt->position);
                }
                ImGui::SameLine();
                if (ImGui::DragInt("  Pos Z", &sliderValueZ, 2.0f, -2, 2))
                {
                    cubeLookingAt->position.z += sliderValueZ;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x,
                                   cubeLookingAt->scale.y, cubeLookingAt->scale.z, STRIDE);
                    voxels[cubeLookingAt->index].collider.setPosition(cubeLookingAt->position);
                    sliderValueZ = sliderCenter; // Reset to center
                    glm::vec3 colliderScale =
                        glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                    voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                }
                ImGui::SameLine();
                if (ImGui::Button(">##PositionZ"))
                {

                    zPos++;
                    cubeLookingAt->position.z = zPos;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x,
                                   cubeLookingAt->scale.y, cubeLookingAt->scale.z, STRIDE);
                    voxels[cubeLookingAt->index].collider.setPosition(cubeLookingAt->position);
                }

                ImGui::Spacing();
                glm::vec3 rots = glm::vec3(0);
                glm::vec3 rotsRadians = glm::vec3(0);
                // Add sliders for scale
                float xrot = cubeLookingAt->rotation.x;
                float yrot = cubeLookingAt->rotation.y;
                float zrot = cubeLookingAt->rotation.z;

                if (ImGui::Button("<##ROTX"))
                {
                    if (xrot > 0)
                    {
                        xrot -= 0.5f;

                        cubeLookingAt->rotation.x = xrot;
                        voxels[cubeLookingAt->index].rotation = cubeLookingAt->rotation;
                        SetRotsRadians(rots, rotsRadians, cubeLookingAt);
                        voxels[0].rotation = rots;
                        voxels[0].scale = voxels[cubeLookingAt->index].scale;

                        UpdateRotation(cubeLookingAt, voxels, cubeLookingAt->rotation);
                        UpdateRotation(&voxels[0], voxels, rots);
                        vb.UpdateRotation(0, cubeLookingAt->position, cubeLookingAt->position, rots, STRIDE);
                        vb.UpdateRotation(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->position,
                                          cubeLookingAt->rotation, STRIDE);
                    }
                }
                ImGui::SameLine();
                if (ImGui::DragFloat("   ROTX", &xrot, 0.5f, 0, 360))
                {
                    cubeLookingAt->rotation.x = xrot;
                    voxels[cubeLookingAt->index].rotation = cubeLookingAt->rotation;
                    SetRotsRadians(rots, rotsRadians, cubeLookingAt);
                    voxels[0].rotation = rots;
                    voxels[0].scale = voxels[cubeLookingAt->index].scale;

                    UpdateRotation(cubeLookingAt, voxels, cubeLookingAt->rotation);
                    UpdateRotation(&voxels[0], voxels, rots);
                    vb.UpdateRotation(0, cubeLookingAt->position, cubeLookingAt->position, rots, STRIDE);
                    vb.UpdateRotation(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->position,
                                      cubeLookingAt->rotation, STRIDE);
                }
                ImGui::SameLine();
                if (ImGui::Button(">##ROTX"))
                {
                    if (xrot < 360)
                    {
                        xrot += 0.5f;

                        cubeLookingAt->rotation.x = xrot;
                        voxels[cubeLookingAt->index].rotation = cubeLookingAt->rotation;
                        SetRotsRadians(rots, rotsRadians, cubeLookingAt);
                        voxels[0].rotation = rots;
                        voxels[0].scale = voxels[cubeLookingAt->index].scale;

                        UpdateRotation(cubeLookingAt, voxels, cubeLookingAt->rotation);
                        UpdateRotation(&voxels[0], voxels, rots);
                        vb.UpdateRotation(0, cubeLookingAt->position, cubeLookingAt->position, rots, STRIDE);
                        vb.UpdateRotation(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->position,
                                          cubeLookingAt->rotation, STRIDE);
                    }
                }

                /////

                if (ImGui::Button("<##ROTY"))
                {
                    if (yrot > 0)
                    {
                        yrot -= 0.5f;

                        cubeLookingAt->rotation.y = yrot;
                        voxels[cubeLookingAt->index].rotation = cubeLookingAt->rotation;
                        SetRotsRadians(rots, rotsRadians, cubeLookingAt);
                        voxels[0].rotation = rots;
                        voxels[0].scale = voxels[cubeLookingAt->index].scale;

                        UpdateRotation(cubeLookingAt, voxels, cubeLookingAt->rotation);
                        UpdateRotation(&voxels[0], voxels, rots);
                        vb.UpdateRotation(0, cubeLookingAt->position, cubeLookingAt->position, rots, STRIDE);
                        vb.UpdateRotation(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->position,
                                          cubeLookingAt->rotation, STRIDE);
                    }
                }
                ImGui::SameLine();
                if (ImGui::DragFloat("   ROTY", &yrot, 0.5f, 0, 360))
                {
                    cubeLookingAt->rotation.y = yrot;
                    voxels[cubeLookingAt->index].rotation = cubeLookingAt->rotation;
                    SetRotsRadians(rots, rotsRadians, cubeLookingAt);
                    voxels[0].rotation = rots;
                    voxels[0].scale = voxels[cubeLookingAt->index].scale;

                    UpdateRotation(cubeLookingAt, voxels, cubeLookingAt->rotation);
                    UpdateRotation(&voxels[0], voxels, rots);
                    vb.UpdateRotation(0, cubeLookingAt->position, cubeLookingAt->position, rots, STRIDE);
                    vb.UpdateRotation(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->position,
                                      cubeLookingAt->rotation, STRIDE);
                }
                ImGui::SameLine();
                if (ImGui::Button(">##ROTY"))
                {
                    if (yrot < 360)
                    {
                        yrot += 0.5f;

                        cubeLookingAt->rotation.y = yrot;
                        voxels[cubeLookingAt->index].rotation = cubeLookingAt->rotation;
                        SetRotsRadians(rots, rotsRadians, cubeLookingAt);
                        voxels[0].rotation = rots;
                        voxels[0].scale = voxels[cubeLookingAt->index].scale;

                        UpdateRotation(cubeLookingAt, voxels, cubeLookingAt->rotation);
                        UpdateRotation(&voxels[0], voxels, rots);
                        vb.UpdateRotation(0, cubeLookingAt->position, cubeLookingAt->position, rots, STRIDE);
                        vb.UpdateRotation(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->position,
                                          cubeLookingAt->rotation, STRIDE);
                    }
                }

                ///////

                if (ImGui::Button("<##ROTZ"))
                {
                    if (zrot > 0.5f)
                    {
                        zrot -= 0.5f;

                        cubeLookingAt->rotation.z = zrot;
                        voxels[cubeLookingAt->index].rotation = cubeLookingAt->rotation;
                        SetRotsRadians(rots, rotsRadians, cubeLookingAt);
                        voxels[0].rotation = rots;
                        voxels[0].scale = voxels[cubeLookingAt->index].scale;

                        UpdateRotation(cubeLookingAt, voxels, cubeLookingAt->rotation);
                        UpdateRotation(&voxels[0], voxels, rots);
                        vb.UpdateRotation(0, cubeLookingAt->position, cubeLookingAt->position, rots, STRIDE);
                        vb.UpdateRotation(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->position,
                                          cubeLookingAt->rotation, STRIDE);
                    }
                }
                ImGui::SameLine();
                if (ImGui::DragFloat("   ROTZ", &zrot, 0.5f, 0, 360))
                {
                    cubeLookingAt->rotation.z = zrot;
                    voxels[cubeLookingAt->index].rotation = cubeLookingAt->rotation;
                    SetRotsRadians(rots, rotsRadians, cubeLookingAt);
                    voxels[0].rotation = rots;
                    voxels[0].scale = voxels[cubeLookingAt->index].scale;

                    UpdateRotation(cubeLookingAt, voxels, cubeLookingAt->rotation);
                    UpdateRotation(&voxels[0], voxels, rots);
                    vb.UpdateRotation(0, cubeLookingAt->position, cubeLookingAt->position, rots, STRIDE);
                    vb.UpdateRotation(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->position,
                                      cubeLookingAt->rotation, STRIDE);
                }
                ImGui::SameLine();
                if (ImGui::Button(">##ROTZ"))
                {
                    if (zrot < 360)
                    {
                        zrot += 0.5f;

                        cubeLookingAt->rotation.z = zrot;
                        voxels[cubeLookingAt->index].rotation = cubeLookingAt->rotation;
                        SetRotsRadians(rots, rotsRadians, cubeLookingAt);
                        voxels[0].rotation = rots;
                        voxels[0].scale = voxels[cubeLookingAt->index].scale;

                        UpdateRotation(cubeLookingAt, voxels, cubeLookingAt->rotation);
                        UpdateRotation(&voxels[0], voxels, rots);
                        vb.UpdateRotation(0, cubeLookingAt->position, cubeLookingAt->position, rots, STRIDE);
                        vb.UpdateRotation(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->position,
                                          cubeLookingAt->rotation, STRIDE);
                    }
                }

                ImGui::Spacing();

                // Add sliders for scale
                float xScale = cubeLookingAt->scale.x;
                float yScale = cubeLookingAt->scale.y;
                float zScale = cubeLookingAt->scale.z;

                if (ImGui::Button("<##ScaleX"))
                {
                    if (xScale > 0.5f)
                    {
                        xScale -= 0.5f;
                        cubeLookingAt->scale.x = xScale;
                        vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x, yScale,
                                       zScale, STRIDE);
                        voxels[cubeLookingAt->index].scale = cubeLookingAt->scale;
                        glm::vec3 colliderScale =
                            glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                        voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                        cubeLookingAt->collider.setSize(colliderScale);
                    }
                }
                ImGui::SameLine();
                if (ImGui::DragFloat("Scale X", &xScale, 0.5f, 0.5f, 50.0f))
                {
                    cubeLookingAt->scale.x = xScale;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x, yScale,
                                   zScale, STRIDE);
                    voxels[cubeLookingAt->index].scale = cubeLookingAt->scale;
                    glm::vec3 colliderScale =
                        glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                    voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                    cubeLookingAt->collider.setSize(colliderScale);
                }
                ImGui::SameLine();
                if (ImGui::Button(">##ScaleX"))
                {
                    if (xScale < 50.0f)
                    {
                        xScale += 0.5f;
                        cubeLookingAt->scale.x = xScale;
                        vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, cubeLookingAt->scale.x, yScale,
                                       zScale, STRIDE);
                        voxels[cubeLookingAt->index].scale = cubeLookingAt->scale;
                        glm::vec3 colliderScale =
                            glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                        voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                        cubeLookingAt->collider.setSize(colliderScale);
                    }
                }

                if (ImGui::Button("<##ScaleY"))
                {
                    if (yScale > 0.5f)
                    {
                        yScale -= 0.5f;
                        cubeLookingAt->scale.y = yScale;
                        vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, xScale, cubeLookingAt->scale.y,
                                       zScale, STRIDE);
                        voxels[cubeLookingAt->index].scale = cubeLookingAt->scale;
                        glm::vec3 colliderScale =
                            glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                        voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                        cubeLookingAt->collider.setSize(colliderScale);
                    }
                }
                ImGui::SameLine();
                if (ImGui::DragFloat("Scale Y", &yScale, 0.5f, 0.5f, 50.0f))
                {
                    cubeLookingAt->scale.y = yScale;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, xScale, cubeLookingAt->scale.y,
                                   zScale, STRIDE);
                    voxels[cubeLookingAt->index].scale = cubeLookingAt->scale;
                    glm::vec3 colliderScale =
                        glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                    voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                    cubeLookingAt->collider.setSize(colliderScale);
                }
                ImGui::SameLine();
                if (ImGui::Button(">##ScaleY"))
                {
                    if (yScale < 50.0f)
                    {
                        yScale += 0.5f;
                        cubeLookingAt->scale.y = yScale;
                        vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, xScale, cubeLookingAt->scale.y,
                                       zScale, STRIDE);
                        voxels[cubeLookingAt->index].scale = cubeLookingAt->scale;
                        glm::vec3 colliderScale =
                            glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                        voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                        cubeLookingAt->collider.setSize(colliderScale);
                    }
                }

                if (ImGui::Button("<##ScaleZ"))
                {
                    if (zScale > 0.5f)
                    {
                        zScale -= 0.5f;
                        cubeLookingAt->scale.z = zScale;
                        vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, xScale, yScale,
                                       cubeLookingAt->scale.z, STRIDE);
                        voxels[cubeLookingAt->index].scale = cubeLookingAt->scale;
                        glm::vec3 colliderScale =
                            glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                        voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                        cubeLookingAt->collider.setSize(colliderScale);
                    }
                }
                ImGui::SameLine();
                if (ImGui::DragFloat("Scale Z", &zScale, 0.5f, 0.5f, 50.0f))
                {
                    cubeLookingAt->scale.z = zScale;
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, xScale, yScale,
                                   cubeLookingAt->scale.z, STRIDE);
                    voxels[cubeLookingAt->index].scale = cubeLookingAt->scale;
                    glm::vec3 colliderScale =
                        glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                    voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                    cubeLookingAt->collider.setSize(colliderScale);
                }
                ImGui::SameLine();
                if (ImGui::Button(">##ScaleZ"))
                {
                    if (zScale < 50.0f)
                    {
                        zScale += 0.5f;
                        cubeLookingAt->scale.z = zScale;
                        vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, xScale, yScale,
                                       cubeLookingAt->scale.z, STRIDE);
                        voxels[cubeLookingAt->index].scale = cubeLookingAt->scale;
                        glm::vec3 colliderScale =
                            glm::vec3(cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
                        voxels[cubeLookingAt->index].collider.setSize(colliderScale);
                        cubeLookingAt->collider.setSize(colliderScale);
                    }
                }
            }
            else
            {
                bool foundLight = false;
                int indexs = -1;
                for (int i = 0; i < lights.size(); i++)
                {
                    if (lights[i]->position == cubeLookingAt->position)
                    {
                        foundLight = true;
                        indexs = i;
                    }
                }

                if (foundLight)
                {
                    std::vector<float> rgbaHere;

                    float r = lights[indexs]->rgba[0];
                    float g = lights[indexs]->rgba[1];
                    float b = lights[indexs]->rgba[2];
                    float intensity = lights[indexs]->brightness;

                    if (ImGui::Button("<##Red"))
                    {
                        if (r > 0)
                        {
                            r -= .01f;
                            lights[indexs]->rgba[0] = r;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::DragFloat("Red", &r, .01f, 0, 1))
                    {
                        lights[indexs]->rgba[0] = r;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(">##Red"))
                    {
                        if (r < 1)
                        {
                            r += .01f;
                            lights[indexs]->rgba[0] = r;
                        }
                    }

                    //////////////////////////

                    if (ImGui::Button("<##Green"))
                    {
                        if (g > 0)
                        {
                            g -= .01f;
                            lights[indexs]->rgba[1] = g;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::DragFloat("Green", &g, .01f, 0, 1))
                    {
                        lights[indexs]->rgba[1] = g;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(">##Green"))
                    {
                        if (g < 255)
                        {
                            g += .01f;
                            lights[indexs]->rgba[1] = g;
                        }
                    }

                    //////////////////////////

                    if (ImGui::Button("<##Blue"))
                    {
                        if (b > 0)
                        {
                            b -= .01f;
                            lights[indexs]->rgba[2] = b;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::DragFloat("Blue", &b, .01f, 0, 1))
                    {
                        lights[indexs]->rgba[2] = b;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(">##Blue"))
                    {
                        if (b < 255)
                        {
                            b += .01f;
                            lights[indexs]->rgba[2] = b;
                        }
                    }

                    //////////////////////////

                    if (ImGui::Button("<##Brightness"))
                    {
                        if (intensity > -.1f)
                        {
                            intensity -= .1f;
                            lights[indexs]->brightness = intensity;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::DragFloat("Brightness", &intensity, .1f, 0, 255))
                    {
                        lights[indexs]->brightness = intensity;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(">##Brightness"))
                    {
                        if (intensity < 255)
                        {
                            intensity += .1f;
                            lights[indexs]->brightness = intensity;
                        }
                    }
                }
            }
            ImGui::Spacing();
            ImGui::Spacing();

            int minTextureIndex = 0;
            int maxTextureIndex = 99;
            int sliderValueX2 = cubeLookingAt->textureIndex; // Initial value set to center

            if (ImGui::Button("<##Texture"))
            {
                if (sliderValueX2 > minTextureIndex)
                {
                    sliderValueX2--;
                    cubeLookingAt->textureIndex = sliderValueX2;
                    vb.UpdateTexture(cubeLookingAt->index, sliderValueX2, STRIDE);
                    chosenTextureID = sliderValueX2;
                    voxels[cubeLookingAt->index].textureIndex = sliderValueX2;

                    if (sliderValueX2 == 98)
                    {
                        voxels[cubeLookingAt->index].positionLowest = voxels[cubeLookingAt->index].position;
                        voxels[cubeLookingAt->index].positionHighest =
                            voxels[cubeLookingAt->index].positionLowest + glm::vec3(0, 10, 0);
                    }
                    if (sliderValueX2 == 99)
                    {
                        bool foundLight = false;
                        for (int i = 0; i < lights.size(); i++)
                        {
                            lights[i]->position.x = std::round(lights[i]->position.x);
                            lights[i]->position.y = std::round(lights[i]->position.y);
                            lights[i]->position.z = std::round(lights[i]->position.z);
                            if (lights[i]->position.x == voxels[0].position.x &&
                                lights[i]->position.y == voxels[0].position.y &&
                                lights[i]->position.z == voxels[0].position.z)
                                foundLight = true;
                        }

                        if (!foundLight && lights.size() < 256)
                        {

                            lights.push_back(new Light(glm::vec3((int)voxels[0].position.x, (int)voxels[0].position.y,
                                                                 (int)voxels[0].position.z)));
                            voxels.push_back(
                                Cube(glm::vec3((int)voxels[0].position.x, voxels[0].position.y, voxels[0].position.z),
                                     chosenTextureID, 0));
                            voxels[voxels.size() - 1].index = (voxels.size() - 1);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < lights.size(); i++)
                        {
                            if (cubeLookingAt->position == lights[i]->position)
                            {
                                lights.erase(lights.begin() + i);
                            }
                        }
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::SliderInt("TEXTURE", &sliderValueX2, minTextureIndex, maxTextureIndex))
            {
                cubeLookingAt->textureIndex = sliderValueX2;
                vb.UpdateTexture(cubeLookingAt->index, sliderValueX2, STRIDE);
                chosenTextureID = sliderValueX2;
                voxels[cubeLookingAt->index].textureIndex = sliderValueX2;

                if (sliderValueX2 == 98)
                {
                    voxels[cubeLookingAt->index].positionLowest = voxels[cubeLookingAt->index].position;
                    voxels[cubeLookingAt->index].positionHighest =
                        voxels[cubeLookingAt->index].positionLowest + glm::vec3(0, 10, 0);
                }
                if (sliderValueX2 == 99)
                {
                    bool foundLight = false;
                    for (int i = 0; i < lights.size(); i++)
                    {
                        lights[i]->position.x = (int)std::round(lights[i]->position.x);
                        lights[i]->position.y = (int)std::round(lights[i]->position.y);
                        lights[i]->position.z = (int)std::round(lights[i]->position.z);
                        if (lights[i]->position.x == voxels[0].position.x &&
                            lights[i]->position.y == voxels[0].position.y &&
                            lights[i]->position.z == voxels[0].position.z)
                            foundLight = true;
                    }

                    if (!foundLight && lights.size() < 256)
                    {

                        lights.push_back(new Light(glm::vec3((int)voxels[0].position.x, (int)voxels[0].position.y,
                                                             (int)voxels[0].position.z)));
                        voxels.push_back(
                            Cube(glm::vec3((int)voxels[0].position.x, voxels[0].position.y, voxels[0].position.z),
                                 chosenTextureID, 0));
                        voxels[voxels.size() - 1].index = (voxels.size() - 1);
                    }
                }
                else
                {
                    for (int i = 0; i < lights.size(); i++)
                    {
                        if (cubeLookingAt->position == lights[i]->position)
                        {
                            lights.erase(lights.begin() + i);
                        }
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(">##Texture"))
            {
                if (sliderValueX2 < maxTextureIndex)
                {
                    sliderValueX2++;
                    cubeLookingAt->textureIndex = sliderValueX2;
                    vb.UpdateTexture(cubeLookingAt->index, sliderValueX2, STRIDE);
                    chosenTextureID = sliderValueX2;
                    voxels[cubeLookingAt->index].textureIndex = sliderValueX2;

                    if (sliderValueX2 == 98)
                    {
                        voxels[cubeLookingAt->index].positionLowest = voxels[cubeLookingAt->index].position;
                        voxels[cubeLookingAt->index].positionHighest =
                            voxels[cubeLookingAt->index].positionLowest + glm::vec3(0, 10, 0);
                    }
                    if (sliderValueX2 == 99)
                    {
                        bool foundLight = false;
                        for (int i = 0; i < lights.size(); i++)
                        {
                            lights[i]->position.x = std::round(lights[i]->position.x);
                            lights[i]->position.y = std::round(lights[i]->position.y);
                            lights[i]->position.z = std::round(lights[i]->position.z);
                            if (lights[i]->position.x == voxels[0].position.x &&
                                lights[i]->position.y == voxels[0].position.y &&
                                lights[i]->position.z == voxels[0].position.z)
                                foundLight = true;
                        }

                        if (!foundLight && lights.size() < 256)
                        {

                            lights.push_back(new Light(glm::vec3((int)voxels[0].position.x, (int)voxels[0].position.y,
                                                                 (int)voxels[0].position.z)));
                            voxels.push_back(
                                Cube(glm::vec3((int)voxels[0].position.x, voxels[0].position.y, voxels[0].position.z),
                                     chosenTextureID, 0));
                            voxels[voxels.size() - 1].index = (voxels.size() - 1);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < lights.size(); i++)
                        {
                            if (cubeLookingAt->position == lights[i]->position)
                            {
                                lights.erase(lights.begin() + i);
                            }
                        }
                    }
                }
            }

            ImGuiIO &io = ImGui::GetIO();
            if (io.MouseWheel > 0)
            {
                if (sliderValueX2 < maxTextureIndex)
                {
                    sliderValueX2++;
                    cubeLookingAt->textureIndex = sliderValueX2;
                    vb.UpdateTexture(cubeLookingAt->index, sliderValueX2, STRIDE);
                    chosenTextureID = sliderValueX2;
                    voxels[cubeLookingAt->index].textureIndex = sliderValueX2;
                    if (sliderValueX2 == 98)
                    {
                        voxels[cubeLookingAt->index].positionLowest = voxels[cubeLookingAt->index].position;
                        voxels[cubeLookingAt->index].positionHighest =
                            voxels[cubeLookingAt->index].positionLowest + glm::vec3(0, 10, 0);
                    }
                }
            }
            else if (io.MouseWheel < 0)
            {
                if (sliderValueX2 > minTextureIndex)
                {
                    sliderValueX2--;
                    cubeLookingAt->textureIndex = sliderValueX2;
                    vb.UpdateTexture(cubeLookingAt->index, sliderValueX2, STRIDE);
                    voxels[cubeLookingAt->index].textureIndex = sliderValueX2;
                    chosenTextureID = sliderValueX2;
                    if (sliderValueX2 == 98)
                    {
                        voxels[cubeLookingAt->index].positionLowest = voxels[cubeLookingAt->index].position;
                        voxels[cubeLookingAt->index].positionHighest =
                            voxels[cubeLookingAt->index].positionLowest + glm::vec3(0, 10, 0);
                    }
                }
            }

            ImGui::End();
        }

#pragma endregion CubeLookingAt

#pragma region PauseMenu

        if (paused)
        {
            ImGui::Begin("PAUSED", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::SetWindowFontScale(1.0f);
            ImGui::Spacing();

            ImGui::SliderInt("Master Volume", &soundManager.masterVolume, 0, 100, "%d");
            ImGui::SliderInt("Music Volume", &soundManager.musicVolume, 0, 100, "%d");
            ImGui::SliderInt("Effect Volume", &soundManager.soundVolume, 0, 100, "%d");

            ImVec2 windowPos =
                ImVec2((width - ImGui::GetWindowWidth()) / 2, ((height - ImGui::GetWindowHeight()) / 2) + 250);
            ImGui::SetWindowPos(windowPos);

            ImGui::Spacing();
            ImGui::Spacing();

            float buttonWidth = ImGui::CalcTextSize("LOAD TUTORIAL MAP").x + 20;
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) / 2);
            if (ImGui::Button("RESUME", ImVec2(buttonWidth, 0)))
            {
                paused = false;
                mouseControl = false;
            }

            buttonWidth = ImGui::CalcTextSize("LOAD TUTORIAL MAP").x + 20;
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) / 2);
            if (ImGui::Button("LEVEL SELECT", ImVec2(buttonWidth, 0)))
            {
                levelSelect = true;
            }

            buttonWidth = ImGui::CalcTextSize("LOAD TUTORIAL MAP").x + 20;
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) / 2);
            if (ImGui::Button("UNSTUCK", ImVec2(buttonWidth, 0)))
            {
                camera.position = camera.lastPosition;
                camera.yVelocity = 0;
            }
            buttonWidth = ImGui::CalcTextSize("LOAD TUTORIAL MAP").x + 20;
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) / 2);
            if (ImGui::Button("WIPE MAP", ImVec2(buttonWidth, 0)))
            {
                voxels = LoadCubesFromFile("res/maps/mapVoxelsBackup.txt");
                lights = LoadLightsFromFile("res/maps/mapLightsBackup.txt");
                needsRefresh = true;
            }

            ImGui::Spacing();
            buttonWidth = ImGui::CalcTextSize("LOAD TUTORIAL MAP").x + 20;
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - (buttonWidth * 2 + 10)) / 2);
            if (ImGui::Button("SAVE MAP", ImVec2(buttonWidth, 0)))
            {
                SaveCubesToFile(voxels, "res/maps/mapVoxels" + std::to_string(currentLevel) + ".txt");
                SaveLightsToFile(lights, "res/maps/mapLights" + std::to_string(currentLevel) + ".txt");
            }
            ImGui::SameLine();
            if (ImGui::Button("LOAD TUTORIAL MAP", ImVec2(buttonWidth, 0)))
            {
                currentLevel = 0;
                voxels = LoadCubesFromFile("res/maps/mapVoxels0.txt");
                lights = LoadLightsFromFile("res/maps/mapLights0.txt");
                for (int i = 0; i < voxels.size(); i++)
                {
                    voxels[i].collider.setSize(voxels[i].scale);
                    voxels[i].collider.setPosition(voxels[i].position);
                }
                needsRefresh = true;
            }
            ImGui::Spacing();

            /*static char name[128] = "Object";

            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) / 2);
            ImGui::InputText("Filename", name, IM_ARRAYSIZE(name));

            if (ImGui::Button("SAVE", ImVec2(buttonWidth, 0)))
            {
                SaveObject(name, voxels);
            }*/

            // ImGui::Spacing();

            buttonWidth = ImGui::GetWindowWidth() - 20;
            if (ImGui::Button("EXIT", ImVec2(buttonWidth, 50)))
            {
                glfwSetWindowShouldClose(window, true);
            }

            ImGui::End();
        }

#pragma endregion PauseMenu

#pragma region LevelSelect

        if (levelSelect)
        {

            ImGui::Begin("LEVEL SELECT", &levelSelect, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

            ImGui::Dummy(ImVec2(5.0f, 5.0f));

            ImGui::BeginGroup();

            const char *lvlText = levelText.c_str();

            GLuint imageID = GetLevelImage(currentLevel, levelImages);
            ImGui::Image((void *)(intptr_t)imageID, ImVec2(100, 100));

            ImGui::SameLine();

            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 200.0f);
            ImGui::TextWrapped(descriptions[currentLevel]);
            ImGui::PopTextWrapPos();

            ImGui::EndGroup();

            ImGui::Dummy(ImVec2(0.0f, 20.0f));

            for (int i = 1; i <= 9; i++)
            {
                if (i % 3 != 1)
                {
                    ImGui::SameLine();
                }

                char label[16];
                snprintf(label, sizeof(label), "Level %d", i);
                if (ImGui::Button(label, ImVec2(100, 100)))
                {

                    currentLevel = i;
                    levelText = "Level: " + std::to_string(i);
                }
            }

            ImGui::Dummy(ImVec2(0.0f, 0.0f));

            if (ImGui::Button("LOAD LEVEL", ImVec2(ImGui::GetContentRegionAvail().x, 50)))
            {
                voxels = LoadCubesFromFile("res/maps/mapVoxels" + std::to_string(currentLevel) + ".txt");
                lights = LoadLightsFromFile("res/maps/mapLights" + std::to_string(currentLevel) + ".txt");
                for (int i = 0; i < voxels.size(); i++)
                {
                    voxels[i].collider.setSize(voxels[i].scale);
                    voxels[i].collider.setPosition(voxels[i].position);
                }
                needsRefresh = true;
                addNotification("LOADED LEVEL: " + std::to_string(currentLevel), 100);
                paused = false;
                levelSelect = false;
                mouseControl = false;
            }

            ImGui::End();
        }

#pragma endregion LevelSelect

#pragma region Notifications
        ImGui::SetNextWindowSize(ImVec2(500, 500));

        for (int i = 0; i < notifications.size(); i++)
        {
            ImGui::Begin("Notification", NULL, window_flags);
            ImGui::SetWindowPos(ImVec2(10, 100));

            ImGui::Text("%s", notifications[i]->message.c_str());
            ImGui::End();
            notifications[i]->time -= 1;
            if (notifications[i]->time <= 0)
            {
                notifications.erase(notifications.begin() + i);
            }
        }

#pragma endregion Notifications

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#pragma endregion IMGUI

        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
