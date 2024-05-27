
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// Do NOT connect these two
#include "Camera.h"
#include "CubeCollider.h"
#include "FastNoiseLite.h"
#include "IndexBuffer.h"
#include "Light.h"
#include "Ray.h"
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "src/Texture.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

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
    CubeCollider collider;
    int index = 0;
    bool invisible = false;
    std::vector<float> cornerPositions;
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
        positionLowest = position;
        positionHighest = positionLowest + glm::vec3(0, 10, 0);
        this->rotation = glm::vec3(0, 0, 0);
        this->textureIndex = 0;
        this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
        collider.setPosition(position);
        collider.setSize(scale);
        SetCornerPositions(textureID, chosen);
        UpdateVertices();
    }

    Cube(glm::vec3 position, float textureID, float chosen = 0)
    {
        this->position = position;
        positionLowest = position;
        positionHighest = positionLowest + glm::vec3(0, 10, 0);

        this->rotation = glm::vec3();
        this->textureIndex = textureID;
        this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
        collider.setPosition(position);
        collider.setSize(scale);
        SetCornerPositions(textureID, chosen);
        UpdateVertices();
    }

    Cube(glm::vec3 pos, float textureID) : position(pos), textureIndex(textureID), collider()
    {
        float chosen = 0;

        this->position = position;
        positionLowest = position;
        positionHighest = positionLowest + glm::vec3(0, 10, 0);

        this->rotation = glm::vec3();
        this->textureIndex = textureID;
        this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
        collider.setPosition(position);
        collider.setSize(scale);
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
        printVec3("Position Lowest", positionLowest);
        printVec3("Position Highest", positionHighest);
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
            -1, -1,  1, 0.0f, 0.0f, textureID,  0,  0,  1, chosen,
             1, -1,  1, 0.1f, 0.0f, textureID,  0,  0,  1, chosen,
             1,  1,  1, 0.1f, 0.1f, textureID,  0,  0,  1, chosen,
            -1,  1,  1, 0.0f, 0.1f, textureID,  0,  0,  1, chosen,

            // Back face
            -1, -1, -1, 0.0f, 0.0f, textureID,  0,  0, -1, chosen,
             1, -1, -1, 0.1f, 0.0f, textureID,  0,  0, -1, chosen,
             1,  1, -1, 0.1f, 0.1f, textureID,  0,  0, -1, chosen,
            -1,  1, -1, 0.0f, 0.1f, textureID,  0,  0, -1, chosen,

            // Top face
            -1,  1, -1, 0.0f, 0.0f, textureID,  0,  1,  0, chosen,
             1,  1, -1, 0.1f, 0.0f, textureID,  0,  1,  0, chosen,
             1,  1,  1, 0.1f, 0.1f, textureID,  0,  1,  0, chosen,
            -1,  1,  1, 0.0f, 0.1f, textureID,  0,  1,  0, chosen,

            // Bottom face
            -1, -1, -1, 0.0f, 0.0f, textureID,  0, -1,  0, chosen,
             1, -1, -1, 0.1f, 0.0f, textureID,  0, -1,  0, chosen,
             1, -1,  1, 0.1f, 0.1f, textureID,  0, -1,  0, chosen,
            -1, -1,  1, 0.0f, 0.1f, textureID,  0, -1,  0, chosen,

            // Right face
             1, -1, -1, 0.0f, 0.0f, textureID,  1,  0,  0, chosen,
             1,  1, -1, 0.0f, 0.1f, textureID,  1,  0,  0, chosen,
             1,  1,  1, 0.1f, 0.1f, textureID,  1,  0,  0, chosen,
             1, -1,  1, 0.1f, 0.0f, textureID,  1,  0,  0, chosen,

            // Left face
            -1, -1, -1, 0.0f, 0.0f, textureID, -1,  0,  0, chosen,
            -1,  1, -1, 0.0f, 0.1f, textureID, -1,  0,  0, chosen,
            -1,  1,  1, 0.1f, 0.1f, textureID, -1,  0,  0, chosen,
            -1, -1,  1, 0.1f, 0.0f, textureID, -1,  0,  0, chosen,
        };

        // clang-format on
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

    void Increase()
    {
        if (position.y < positionHighest.y)
        {
            position.y += 0.02;
            collider.position.y += 0.02;
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
        if (point.x > position.x - scale.x && point.x < position.x + scale.x)
        {
            if (point.y > position.y - scale.y && point.y < position.y + scale.y)
            {
                if (point.z > position.z - scale.z && point.z < position.z + scale.z)
                {
                    return true;
                }
            }
        }
        return false;
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

float calculatePenetrationDepth(const glm::vec3 &point, const Cube &voxel)
{
    // Calculate the closest point on the voxel to the point
    glm::vec3 closestPointOnVoxel = glm::clamp(point, voxel.position - voxel.scale, voxel.position + voxel.scale);

    // Calculate the difference vector between the point and the closest point on the voxel
    glm::vec3 difference = closestPointOnVoxel - point;

    // Calculate the length of the difference vector
    float penetrationDepth = glm::length(difference);

    return penetrationDepth;
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

#pragma endregion

Cube cubeDummy(glm::vec3(0, 0, 0), 99, 0);
const unsigned int STRIDE = 10;
int cooldownForBreak = 0;
bool pinned = false;
double lastFrameTime = 0.0;
glm::vec3 spawnPoint = glm::vec3(0, 3, 0);

const bool PRINTLOG = true;      // for debugging start of program
const bool PRINTLOOPLOG = false; // for debugging loop  in program

int main()
{

#pragma region INITIALIZATION

#pragma region MAPSETUP
    if (PRINTLOG)
        std::cout << "Setting up noise values!" << std::endl;
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(4.0);
    noise.SetFractalLacunarity(5.0f);
    noise.SetFractalGain(0.5f);
    noise.SetFrequency(0.06f);
    noise.SetSeed(rand());

    if (PRINTLOG)
        std::cout << "Building map from map.txt..." << std::endl;
    std::vector<Cube> voxels;
    voxels.push_back(cubeDummy);

    voxels = LoadCubesFromFile("res/mapVoxels.txt");
    lights = LoadLightsFromFile("res/mapLights.txt");
    needsRefresh = true;

    if (PRINTLOG)
        std::cout << "Map built successfully! Constructing voxels from data..." << std::endl;
#pragma endregion MAPSETUP

#pragma region GLINIT
    const unsigned int AMOUNT_OF_INDICES = 36;
    const unsigned int FULL_STRIDE = STRIDE * AMOUNT_OF_INDICES * VertexBufferElement::GetSizeOfType(GL_FLOAT);
    int indicesCount = voxels.size() * AMOUNT_OF_INDICES;
    unsigned int *indicesAfter = new unsigned int[indicesCount];
    for (int i = 0; i < voxels.size(); i++)
    {
        for (int j = 0; j < voxels[i].indices.size(); j++)
        {
            indicesAfter[i * AMOUNT_OF_INDICES + j] = voxels[i].indices[j] + i * AMOUNT_OF_INDICES;
        }
    }

    if (PRINTLOG)
        std::cout << "Math for buffers done, creating vectors!" << std::endl;

    // this is the size of each tri's info (6, 3 for position, 2 for texture coordinates, 1 textureID) * 36 (number of
    // indices in our cube)
    float *positions = new float[voxels.size() * STRIDE * AMOUNT_OF_INDICES];
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

    if (PRINTLOG)
        std::cout << "GLFW and GL3W initialised! Setting up VBO/VAO!" << std::endl;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    if (PRINTLOG)
        std::cout << "Set depth-testing to true\nSet blending to true" << std::endl;

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if (PRINTLOG)
        std::cout << "Finally binding VAO..." << std::endl;

#pragma endregion GLINIT

    // this is the size of each tri's info (5, 3 for position, 2 for texture coordinates) * 36 (number of indices in our
    // cube)
    int positionCount = STRIDE * AMOUNT_OF_INDICES * voxels.size();
    VertexArray va;
    if (PRINTLOG)
        std::cout << "Finally creating vertex buffer from info..." << std::endl;

    VertexBuffer vb(positions, voxels.size() * FULL_STRIDE);
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

#pragma region GLINIT2

    glm::mat4 proj = glm::perspective(glm::radians(camera.fov), (float)width / (float)height, 0.1f, 5000.0f);
    glm::mat4 view = glm::lookAt(camera.position, camera.target, glm::vec3(0, 1, 0));
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = proj * view * model;

    if (PRINTLOG)
        std::cout << "Voxels generated! Creating shaders..." << std::endl;
    Shader shader("res/shaders/Basic.shader");

    shader.Bind();
    shader.SetUniformMat4f("u_MVP", mvp);
    if (PRINTLOG)
        std::cout << "Bound shaders successfully! Binding textures to GPU...\n";

    Texture u_TextureAtlas("res/textures/atlas.png");

    u_TextureAtlas.Bind(0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (PRINTLOG)
        std::cout << "Textures bound! Setting texture uniforms..." << std::endl;

    shader.SetUniform1i("u_TextureAtlas", 0);

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

    std::vector<Cube *> voxelsCloseToPlayer;
    Cube *cubeLookingAt = nullptr;

#pragma endregion GLINIT2

    /*glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);*/

#pragma endregion INITIALIZATION

    while (!glfwWindowShouldClose(window))
    {
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

        double currentTime = glfwGetTime();
        double frameDuration = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        currentFrame = glfwGetTime();
        dt = currentFrame - lastFrame;
        lastFrame = currentFrame;
        double targetFrameDuration = 1.0 / 60.0;
        double timeLeftForNextFrame = targetFrameDuration - frameDuration;
        auto frameStart = std::chrono::steady_clock::now();
        float distToCube = 99999999;
        needsRefresh = true;
        if (PRINTLOOPLOG)
            std::cout << "starting rendering..." << std::endl;

        if (timeLeftForNextFrame > 0.0)
        {

            if (needsRefresh)
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

                // this is the size of each tri's info (6, 3 for position, 2 for texture coordinates, 1 textureID) * 36
                // (number of indices in our cube)
                // positions = new float[voxels.size() * STRIDE * AMOUNT_OF_INDICES];

                if (positions != nullptr)
                {
                    delete[] positions;
                    positions = nullptr;
                }
                positions = new float[voxels.size() * STRIDE * AMOUNT_OF_INDICES];
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

                vb.UpdateBuffer(positions, voxels.size() * FULL_STRIDE);
                va.AddBuffer(vb, layout);
                ib.UpdateBuffer(indicesAfter, indicesCount);

                va.Bind();
                vb.Bind();
                ib.Bind();
                if (PRINTLOOPLOG)
                    std::cout << "done refreshing map!" << std::endl;
            }

            if (PRINTLOOPLOG)
                std::cout << "doing matrix transformations..." << std::endl;

#pragma region MVP
            ImGui_ImplOpenGL3_NewFrame();
            glfwSwapInterval(1);
            renderer.Clear();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
                shader.SetUniform4f(("lightColor[" + std::to_string(i) + "]").c_str(), lights[i]->rgba.x,
                                    lights[i]->rgba.y, lights[i]->rgba.z, lights[i]->rgba.w);
                shader.SetUniform1f(("lightBrightness[" + std::to_string(i) + "]").c_str(), lights[i]->brightness);

                lights[i]->Update();
            }
            shader.SetUniform1i("numLights", numLights);
            renderer.Draw(va, ib, shader);

#pragma endregion MVP

            if (PRINTLOOPLOG)
                std::cout << "done with transformations, handling collision now!" << std::endl;

#pragma region COLLISION

            voxelsCloseToPlayer.clear();

            if (PRINTLOOPLOG)
                std::cout << "setting up 'near player' vector" << std::endl;

            for (int i = 0; i < voxels.size(); i++)
            {

                /*float distance = glm::distance(camera.position, voxels[i].position);
                if (distance < 10 && distance > -10)
                {
                    voxelsCloseToPlayer.push_back(&voxels[i]);
                    voxelsCloseToPlayer[voxelsCloseToPlayer.size() - 1]->collider = voxels[i].collider;
                }*/

                voxelsCloseToPlayer.push_back(&voxels[i]);
                voxelsCloseToPlayer[voxelsCloseToPlayer.size() - 1]->collider = voxels[i].collider;
            }
            bool onGround = false;

            if (PRINTLOOPLOG)
                std::cout << "updating camera" << std::endl;

            camera.Update(window, dt, mouseControl);
            camera.collider.setPosition(camera.position + glm::vec3(0, -4, 0));

            camera.pointXMinusColliding = false;
            camera.pointXPlusColliding = false;
            camera.pointZMinusColliding = false;
            camera.pointZPlusColliding = false;

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

            for (int i = 0; i < voxelsCloseToPlayer.size(); i++)
            {
                bool colliding = false;
                // check if the camera's collider is colliding with the cube's collider
                if (voxelsCloseToPlayer[i]->index != 0 && !voxelsCloseToPlayer[i]->invisible &&
                    camera.collider.CheckCollision(voxels[voxelsCloseToPlayer[i]->index].collider))
                {
                    colliding = true;
                    glm::vec3 buffer = camera.collider.ResolveCollision(voxels[voxelsCloseToPlayer[i]->index].collider);
                    if (!camera.getOnGround())
                    {
                        onGround = true;
                        camera.onGround = true;
                        camera.isJumping = false;
                        camera.yVelocity = 0;
                    }
                    buffer.x = 0;
                    buffer.z = 0;
                    camera.position += buffer;
                    // camera.position.y = (camera.position.y * 100.0f) / 100.0f;
                    if (voxelsCloseToPlayer[i]->textureIndex == 98)
                    {
                        voxelsCloseToPlayer[i]->Increase();
                    }
                }

                if (voxelsCloseToPlayer[i]->textureIndex == 98 &&
                    !camera.collider.CheckCollision(voxels[voxelsCloseToPlayer[i]->index].collider))
                {
                    voxelsCloseToPlayer[i]->Decrease();
                }

                if (!voxelsCloseToPlayer[i]->invisible && voxelsCloseToPlayer[i]->isPointInside(camera.pointXPlus))
                {
                    camera.pointXPlusColliding = true;
                }
                if (!voxelsCloseToPlayer[i]->invisible && voxelsCloseToPlayer[i]->isPointInside(camera.pointXMinus))
                {
                    camera.pointXMinusColliding = true;
                }
                if (!voxelsCloseToPlayer[i]->invisible && voxelsCloseToPlayer[i]->isPointInside(camera.pointZPlus))
                {
                    camera.pointZPlusColliding = true;
                }
                if (!voxelsCloseToPlayer[i]->invisible && voxelsCloseToPlayer[i]->isPointInside(camera.pointZMinus))
                {
                    camera.pointZMinusColliding = true;
                }

                bool isChosen = false;
                for (int j = 0; j < pointsToCheck.size(); j++) // the 6 is how many steps the ray is cast forward
                {
                    if (voxelsCloseToPlayer[i]->index != 0 && !voxelsCloseToPlayer[i]->invisible &&
                        voxelsCloseToPlayer[i]->isPointInside(cursorPos))
                    {
                        float distance = glm::distance(camera.position, voxelsCloseToPlayer[i]->position);
                        if (distance < distToCube)
                        {
                            distToCube = distance;
                            cubeLookingAt = voxelsCloseToPlayer[i];
                        }
                    }
                }

                // //check if camera.positionFeet is inside the cube
                if (!camera.getOnGround() &&
                    voxelsCloseToPlayer[i]->isPointInside(camera.positionFeet + glm::vec3(0, -.1, 0)))
                {
                    onGround = true;
                }
            }

            // MUST BE RIGHT AFTER COLLISION/CORRECTION VVVVVV
            camera.target =
                camera.position + glm::vec3(cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch)),
                                            sin(glm::radians(camera.pitch)),
                                            sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch)));

            float collisionCorrectionSpeed = 0;

            if (camera.velocity.x != 0 || camera.velocity.z != 0)
            {
                if (std::abs(camera.velocity.x) > std::abs(camera.velocity.z))
                {
                    collisionCorrectionSpeed = std::abs(camera.velocity.x);
                }
                else
                {
                    collisionCorrectionSpeed = std::abs(camera.velocity.z);
                }
            }

            if (camera.pointXMinusColliding)
            {
                camera.position.x += collisionCorrectionSpeed;
                camera.target.x += collisionCorrectionSpeed;
            }
            if (camera.pointXPlusColliding)
            {
                camera.position.x -= collisionCorrectionSpeed;
                camera.target.x -= collisionCorrectionSpeed;
            }
            if (camera.pointZMinusColliding)
            {
                camera.position.z += collisionCorrectionSpeed;
                camera.target.z += collisionCorrectionSpeed;
            }
            if (camera.pointZPlusColliding)
            {
                camera.position.z -= collisionCorrectionSpeed;
                camera.target.z -= collisionCorrectionSpeed;
            }
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

            //  if left mouse is pressed
            if (!paused && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
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

            if (!paused && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
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
                            voxels[voxels.size() - 1].index = voxels.size() - 1;
                            voxels[voxels.size() - 1].scale = sizeToSave;
                            voxels[voxels.size() - 1].collider.size = sizeToSave;
                            vb.UpdateScale(voxels.size() - 1,
                                           glm::vec3(voxels[0].position.x, voxels[0].position.y, voxels[0].position.z),
                                           sizeToSave.x, sizeToSave.y, sizeToSave.z, STRIDE);

                            needsRefresh = true;
                            voxels[0].scale = glm::vec3(1, 1, 1);
                            vb.UpdateScale(0, voxels[0].position, sizeToSave.x, sizeToSave.y, sizeToSave.z, STRIDE);
                            voxels[0].collider.UpdateScale(sizeToSave);
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

                                lights.push_back(new Light(glm::vec3(
                                    (int)voxels[0].position.x, (int)voxels[0].position.y, (int)voxels[0].position.z)));
                                voxels.push_back(Cube(
                                    glm::vec3((int)voxels[0].position.x, voxels[0].position.y, voxels[0].position.z),
                                    chosenTextureID, 0));
                                voxels[voxels.size() - 1].index = voxels.size() - 1;
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

            if (!paused && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS &&
                glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            {
                if (cubeLookingAt != nullptr)
                {
                    chosenTextureID = cubeLookingAt->textureIndex;
                    sizeToSave = cubeLookingAt->scale;
                    voxels[0].scale = sizeToSave;
                    vb.UpdateScale(0, voxels[0].position, sizeToSave.x, sizeToSave.y, sizeToSave.z, STRIDE);
                    voxels[0].collider.UpdateScale(sizeToSave);
                    addNotification("Copied voxel!", 10);
                }
            }

            if (!paused && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS &&
                glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
            {
                if (cubeLookingAt != nullptr)
                {
                    cubeLookingAt->textureIndex = chosenTextureID;
                    vb.UpdateTexture(cubeLookingAt->index, chosenTextureID, STRIDE);
                    vb.UpdateScale(cubeLookingAt->index, cubeLookingAt->position, sizeToSave.x, sizeToSave.y,
                                   sizeToSave.z, STRIDE);
                    cubeLookingAt->collider.UpdateScale(sizeToSave);
                    voxels[0].scale = glm::vec3(1, 1, 1);
                    vb.UpdateScale(0, voxels[0].position, sizeToSave.x, sizeToSave.y, sizeToSave.z, STRIDE);
                    voxels[0].collider.UpdateScale(sizeToSave);
                    addNotification("Pasted voxel!", 10);
                }
            }
#pragma endregion KEYPRESSES

            if (PRINTLOOPLOG)
                std::cout << "starting IMGUI rendering!" << std::endl;

#pragma region IMGUI

            ImGui::NewFrame();

#pragma region CameraInfo

            int distance = glm::distance(camera.position, glm::vec3(90, -10, 45));
            /*if (distance < 10)
            {
                camera.position = glm::vec3(-185, 70, 50);
                if (currentLevel < maxLevelAllowed)
                    currentLevel++;
                std::string levelName = "res/mapVoxels" + std::to_string(currentLevel) + ".txt";
                voxels = LoadCubesFromFile(levelName);
                for (int i = 0; i < voxels.size(); i++)
                {
                    voxels[i].collider.UpdateScale(voxels[i].scale);
                    voxels[i].collider.setPosition(voxels[i].position);
                }
            }*/

            ImGui::Begin("CAMERA", NULL, ImGuiWindowFlags_AlwaysAutoResize);

            // Title
            ImGui::Text("Camera Information");
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

            // Flying Checkbox
            if (ImGui::Checkbox("Flying?", &camera.isFlying))
            {
                // Handle the change in flying status
            }

            // End window
            ImGui::End();
#pragma endregion CameraInfo

#pragma region SceneInfo

            ImGui::Begin("SceneInfo", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Objects: %lu", voxels.size());
            ImGui::Text("Indices: %d", indicesCount);
            ImGui::Text("Triangles: %lu", voxels.size() * 12);
            ImGui::Text("Lights: %lu", lights.size());
            // Calculate and display FPS
            static float fps = 0.0f;
            static float lastFrameTime = 0.0f;
            const float now = glfwGetTime(); // Assuming you're using GLFW
            fps = 1.0f / (now - lastFrameTime);
            lastFrameTime = now;
            ImGui::Text("FPS: %.1f", fps);
            ImGui::End();

#pragma endregion SceneInfo

#pragma region CubeLookingAt

            if (cubeLookingAt != nullptr)
            {

                ImGui::Begin("CubeLookingAt", NULL, ImGuiWindowFlags_AlwaysAutoResize);
                if (ImGui::Checkbox("Pin", &pinned))
                {
                    // yes
                }
                ImGui::SameLine();
                ImGui::Text("VOXEL %d", cubeLookingAt->index);

                ImGui::Spacing();

                ImGui::Text("posLowest:               (%d,   %d,   %d)", (int)cubeLookingAt->positionLowest.x,
                            (int)cubeLookingAt->positionLowest.y, (int)cubeLookingAt->positionLowest.z);

                ImGui::Spacing();
                ImGui::Text("Position:                    (%d,   %d,   %d)", (int)cubeLookingAt->position.x,
                            (int)cubeLookingAt->position.y, (int)cubeLookingAt->position.z);
                ImGui::Text("Scale   :                    (%.1f, %.1f, %.1f)", cubeLookingAt->scale.x,
                            cubeLookingAt->scale.y, cubeLookingAt->scale.z);
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
                        voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
                        cubeLookingAt->collider.UpdateScale(colliderScale);
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
                    voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
                    cubeLookingAt->collider.UpdateScale(colliderScale);
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
                        voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
                        cubeLookingAt->collider.UpdateScale(colliderScale);
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
                        voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
                        cubeLookingAt->collider.UpdateScale(colliderScale);
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
                    voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
                    cubeLookingAt->collider.UpdateScale(colliderScale);
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
                        voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
                        cubeLookingAt->collider.UpdateScale(colliderScale);
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
                        voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
                        cubeLookingAt->collider.UpdateScale(colliderScale);
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
                    voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
                    cubeLookingAt->collider.UpdateScale(colliderScale);
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
                        voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
                        cubeLookingAt->collider.UpdateScale(colliderScale);
                    }
                }

                ImGui::Spacing();
                ImGui::Spacing();

                int xPos = cubeLookingAt->position.x;
                int yPos = cubeLookingAt->position.y;
                int zPos = cubeLookingAt->position.z;

                int textureIDs = cubeLookingAt->textureIndex;

                const int sliderCenter = 0;      // Center value for the slider
                int sliderValueX = sliderCenter; // Initial value set to center
                int sliderValueY = sliderCenter; // Initial value set to center
                int sliderValueZ = sliderCenter;
                int sliderValueX2 = cubeLookingAt->textureIndex; // Initial value set to center

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
                    voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
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
                    voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
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
                    voxels[cubeLookingAt->index].collider.UpdateScale(colliderScale);
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

                int minTextureIndex = 0;
                int maxTextureIndex = 99;

                if (ImGui::Button("<##Texture"))
                {
                    if (sliderValueX2 > minTextureIndex)
                    {
                        sliderValueX2--;
                        cubeLookingAt->textureIndex = sliderValueX2;
                        vb.UpdateTexture(cubeLookingAt->index, sliderValueX2, STRIDE);
                        chosenTextureID = sliderValueX2;
                        voxels[cubeLookingAt->index].textureIndex = sliderValueX2;

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

                                lights.push_back(new Light(glm::vec3(
                                    (int)voxels[0].position.x, (int)voxels[0].position.y, (int)voxels[0].position.z)));
                                voxels.push_back(Cube(
                                    glm::vec3((int)voxels[0].position.x, voxels[0].position.y, voxels[0].position.z),
                                    chosenTextureID, 0));
                                voxels[voxels.size() - 1].index = voxels.size() - 1;
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
                            voxels[voxels.size() - 1].index = voxels.size() - 1;
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

                                lights.push_back(new Light(glm::vec3(
                                    (int)voxels[0].position.x, (int)voxels[0].position.y, (int)voxels[0].position.z)));
                                voxels.push_back(Cube(
                                    glm::vec3((int)voxels[0].position.x, voxels[0].position.y, voxels[0].position.z),
                                    chosenTextureID, 0));
                                voxels[voxels.size() - 1].index = voxels.size() - 1;
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
                    }
                }

                ImGui::End();
            }

#pragma endregion CubeLookingAt

#pragma region PauseMenu

            if (paused)
            {
                ImGui::Begin("PAUSED", NULL, ImGuiWindowFlags_AlwaysAutoResize);

                // Set font size to 20
                ImGui::SetWindowFontScale(2.0f);

                // Set window position to center
                ImVec2 windowPos =
                    ImVec2((width - ImGui::GetWindowWidth()) / 2, (height - ImGui::GetWindowHeight()) / 2);
                ImGui::SetWindowPos(windowPos);

                ImGui::Spacing();
                ImGui::Spacing();

                // Centered "Resume Program" button
                float buttonWidth = ImGui::CalcTextSize("Resume Program").x + 20; // Adding some padding
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) / 2);
                if (ImGui::Button("Resume Program", ImVec2(buttonWidth, 0)))
                {
                    paused = false;
                    mouseControl = false;
                }

                ImGui::Spacing();

                // Centered "UNSTUCK" button
                buttonWidth = ImGui::CalcTextSize("UNSTUCK").x + 20; // Adding some padding
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) / 2);
                if (ImGui::Button("UNSTUCK", ImVec2(buttonWidth, 0)))
                {
                    camera.position = glm::vec3(0, 3.0f, 0);
                    camera.yVelocity = 0;
                }
                buttonWidth = ImGui::CalcTextSize("WIPE MAP").x + 20;
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) / 2);
                if (ImGui::Button("WIPE MAP", ImVec2(buttonWidth, 0)))
                {
                    voxels = LoadCubesFromFile("res/mapVoxelsBackup.txt");
                    lights = LoadLightsFromFile("res/mapLightsBackup.txt");
                    needsRefresh = true;
                }

                ImGui::Spacing();

                // Centered "SAVE" and "LOAD" buttons
                buttonWidth = ImGui::CalcTextSize("SAVE").x + 20; // Adding some padding
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - (buttonWidth * 2 + 10)) /
                                     2); // Centering both buttons with some spacing
                if (ImGui::Button("SAVE", ImVec2(buttonWidth, 0)))
                {
                    SaveCubesToFile(voxels, "res/mapVoxels.txt");
                    SaveLightsToFile(lights, "res/mapLights.txt");
                }
                ImGui::SameLine();
                if (ImGui::Button("LOAD", ImVec2(buttonWidth, 0)))
                {
                    voxels = LoadCubesFromFile("res/mapVoxels.txt");
                    lights = LoadLightsFromFile("res/mapLights.txt");
                    for (int i = 0; i < voxels.size(); i++)
                    {
                        voxels[i].collider.UpdateScale(voxels[i].scale);
                        voxels[i].collider.setPosition(voxels[i].position);
                    }
                    needsRefresh = true;
                }

                ImGui::Spacing();

                // Centered "Exit Program" button
                buttonWidth = ImGui::CalcTextSize("Exit Program").x + 20; // Adding some padding
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) / 2);
                if (ImGui::Button("Exit Program", ImVec2(buttonWidth, 0)))
                {
                    glfwSetWindowShouldClose(window, true);
                }

                ImGui::End();
            }

#pragma endregion PauseMenu

#pragma region Notifications

            // draw notifications from the notification queue
            for (int i = 0; i < notifications.size(); i++)
            {
                ImGui::Begin("Notification", NULL, ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::SetWindowPos(ImVec2(10, 10));
                ImGui::Text("%s", notifications[i]->message.c_str());
                ImGui::End();
                notifications[i]->time -= .1f;
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
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
