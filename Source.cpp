#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include "Ray.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include <iterator>
#include "Renderer.h"
#include <fstream>
#include <string>
#include <sstream>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "src/Texture.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "FastNoiseLite.h"
#include <vector>
#include <thread>
#include "CubeCollider.h"




Camera camera;
int width = 1920;
int height = 1080;
bool mouseControl = false;






class Cube
{
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    std::vector<glm::vec3> vertices;
    int textureIndex = 0;
    std::vector<glm::vec3> verticesAfterTransformation = {};
    CubeCollider collider;
    int index = 0;


    std::vector<float> cornerPositions;

    std::vector<unsigned int> indices = 
    { 
        0, 1, 2, 2, 3, 0, //front face
        4, 5, 6, 6, 7, 4, //back face
        8, 9, 10, 10, 11, 8, //top face
        12, 13, 14, 14, 15, 12, //bottom face
        16, 17, 18, 18, 19, 16, //right face
        20, 21, 22, 22, 23, 20 //left face
    };

    
    Cube(glm::vec3 position, float textureID)
    {
        this->position = position;glm::vec3();
        this->textureIndex = textureID;
        this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
        collider.setPosition(position);
        collider.setSize(scale);
        this->cornerPositions = 
        {
            // Front face
            -1, -1,  1, 0.0f, 0.0f, textureID, 0, 0, 1,
             1, -1,  1, 1.0f, 0.0f, textureID, 0, 0, 1,
             1,  1,  1, 1.0f, 1.0f, textureID, 0, 0, 1,
            -1,  1,  1, 0.0f, 1.0f, textureID, 0, 0, 1,

            // Back face <<<<<<<
            -1, -1, -1, 0.0f, 0.0f, textureID, 0, 0, -1,
             1, -1, -1, 1.0f, 0.0f, textureID, 0, 0, -1,
             1,  1, -1, 1.0f, 1.0f, textureID, 0, 0, -1,
            -1,  1, -1, 0.0f, 1.0f, textureID, 0, 0, -1,

            // Top face
            -1,  1, -1, 0.0f, 0.0f, textureID, 0, 1, 0,
             1,  1, -1, 1.0f, 0.0f, textureID, 0, 1, 0,
             1,  1,  1, 1.0f, 1.0f, textureID, 0, 1, 0,
            -1,  1,  1, 0.0f, 1.0f, textureID, 0, 1, 0,

            // Bottom face
            -1, -1, -1, 0.0f, 0.0f, textureID, 0, -1, 0,
             1, -1, -1, 1.0f, 0.0f, textureID, 0, -1, 0,
             1, -1,  1, 1.0f, 1.0f, textureID, 0, -1, 0,
            -1, -1,  1, 0.0f, 1.0f, textureID, 0, -1, 0,

            // Right face
            1, -1, -1, 0.0f, 0.0f, textureID, 1, 0, 0,
            1,  1, -1, 0.0f, 1.0f, textureID, 1, 0, 0,
            1,  1,  1, 1.0f, 1.0f, textureID, 1, 0, 0,
            1, -1,  1, 1.0f, 0.0f, textureID, 1, 0, 0,

            // Left face
            -1, -1, -1, 0.0f, 0.0f, textureID, -1, 0, 0,
            -1,  1, -1, 0.0f, 1.0f, textureID, -1, 0, 0,
            -1,  1,  1, 1.0f, 1.0f, textureID, -1, 0, 0,
            -1, -1,  1, 1.0f, 0.0f, textureID, -1, 0, 0
        };


        vertices = {
            glm::vec3(position.x - scale.x, position.y - scale.y, position.z + scale.z), //0
            glm::vec3(position.x + scale.x, position.y - scale.y, position.z + scale.z), //1
            glm::vec3(position.x + scale.x, position.y + scale.y, position.z + scale.z), //2
            glm::vec3(position.x - scale.x, position.y + scale.y, position.z + scale.z), //3
            glm::vec3(position.x - scale.x, position.y - scale.y, position.z - scale.z), //4
            glm::vec3(position.x + scale.x, position.y - scale.y, position.z - scale.z), //5
            glm::vec3(position.x + scale.x, position.y + scale.y, position.z - scale.z), //6
            glm::vec3(position.x - scale.x, position.y + scale.y, position.z - scale.z) //7



        };

        
    }

    void UpdateVertices()
    {
        vertices = {
            glm::vec3(position.x - scale.x, position.y - scale.y, position.z + scale.z), //0
            glm::vec3(position.x + scale.x, position.y - scale.y, position.z + scale.z), //1
            glm::vec3(position.x + scale.x, position.y + scale.y, position.z + scale.z), //2
            glm::vec3(position.x - scale.x, position.y + scale.y, position.z + scale.z), //3
            glm::vec3(position.x - scale.x, position.y - scale.y, position.z - scale.z), //4
            glm::vec3(position.x + scale.x, position.y - scale.y, position.z - scale.z), //5
            glm::vec3(position.x + scale.x, position.y + scale.y, position.z - scale.z), //6
            glm::vec3(position.x - scale.x, position.y + scale.y, position.z - scale.z) //7
        };

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
    std::string message;
    float time;
};

const unsigned int STRIDE = 9;
std::vector<Notification*> notifications;
int cooldownForBreak = 0;

void addNotification(std::string message, float time)
{
    Notification* notification = new Notification();
    notification->message = message;
    notification->time = time;
    notifications.push_back(notification);
}

bool needsRefresh = false;

float calculatePenetrationDepth(const glm::vec3& point, const Cube& voxel)
{
    // Calculate the closest point on the voxel to the point
    glm::vec3 closestPointOnVoxel = glm::clamp(point, voxel.position - voxel.scale, voxel.position + voxel.scale);
    
    // Calculate the difference vector between the point and the closest point on the voxel
    glm::vec3 difference = closestPointOnVoxel - point;
    
    // Calculate the length of the difference vector
    float penetrationDepth = glm::length(difference);
    
    return penetrationDepth;
}

void SetPosition(float* positions, int index, glm::vec3 position, unsigned int amountOfValuesPerTri)
{
    // Front face
    positions[index*amountOfValuesPerTri] = position.x - 1;
    positions[index*amountOfValuesPerTri+1] = position.y - 1;
    positions[index*amountOfValuesPerTri+2] = position.z + 1;

    positions[(index+1)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+1)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+1)*amountOfValuesPerTri+2] = position.z + 1;

    positions[(index+2)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+2)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+2)*amountOfValuesPerTri+2] = position.z + 1;

    positions[(index+3)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+3)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+3)*amountOfValuesPerTri+2] = position.z + 1;

    // Back face
    positions[(index+4)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+4)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+4)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+5)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+5)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+5)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+6)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+6)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+6)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+7)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+7)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+7)*amountOfValuesPerTri+2] = position.z - 1;

    // Top face
    positions[(index+8)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+8)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+8)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+9)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+9)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+9)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+10)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+10)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+10)*amountOfValuesPerTri+2] = position.z + 1;

    positions[(index+11)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+11)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+11)*amountOfValuesPerTri+2] = position.z + 1;

    // Bottom face
    positions[(index+12)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+12)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+12)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+13)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+13)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+13)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+14)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+14)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+14)*amountOfValuesPerTri+2] = position.z + 1;

    positions[(index+15)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+15)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+15)*amountOfValuesPerTri+2] = position.z + 1;

    // Right face
    positions[(index+16)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+16)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+16)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+17)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+17)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+17)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+18)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+18)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+18)*amountOfValuesPerTri+2] = position.z + 1;

    positions[(index+19)*amountOfValuesPerTri] = position.x + 1;
    positions[(index+19)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+19)*amountOfValuesPerTri+2] = position.z + 1;

    // Left face
    positions[(index+20)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+20)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+20)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+21)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+21)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+21)*amountOfValuesPerTri+2] = position.z - 1;

    positions[(index+22)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+22)*amountOfValuesPerTri+1] = position.y + 1;
    positions[(index+22)*amountOfValuesPerTri+2] = position.z + 1;

    positions[(index+23)*amountOfValuesPerTri] = position.x - 1;
    positions[(index+23)*amountOfValuesPerTri+1] = position.y - 1;
    positions[(index+23)*amountOfValuesPerTri+2] = position.z + 1;
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

glm::vec3 sunPosition = glm::vec3(0,5,0);

int main() 
{

#pragma region INITIALIZATION
    std::cout << "Setting up noise values!" << std::endl;
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(4.0);
    noise.SetFractalLacunarity(5.0f);
    noise.SetFractalGain(0.5f);
    noise.SetFrequency(0.06f);
    noise.SetSeed(rand());

    std::cout << "Building map from map.txt..." << std::endl;
    std::vector<Cube> voxels;

    //open map.txt and read the map
    std::ifstream file("map.txt");
    std::string line;
    if (file.is_open())
    {
        int mapHeight = 0;
        int mapWidth = 0;
        while (std::getline(file, line))
        {
            mapHeight++;
            mapWidth = line.size();
        }

        //put the file back to the beginning
        file.clear();
        file.seekg(0);

        int xOffset = 0;
        int zOffset = 0;
        int startingPosX = -mapWidth / 2;
        int startingPosZ = -mapHeight / 2;
        
        while (std::getline(file, line))
        {
            for (int i = 0; i < line.size(); i++)
            {
                if (line[i] == '1')
                {
                    //wall
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, 6, startingPosZ + zOffset), 1));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, 4, startingPosZ + zOffset), 1));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, 2, startingPosZ + zOffset), 1));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, 0, startingPosZ + zOffset), 1));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -2, startingPosZ + zOffset), 1));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -4, startingPosZ + zOffset), 1));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                }
                if (line[i] == '2')
                {
                    //box
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -4, startingPosZ + zOffset), 2));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                }
                if (line[i] == '3')
                {
                    //box (2 tall)
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -2, startingPosZ + zOffset), 2));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -4, startingPosZ + zOffset), 2));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                }
                if (line[i] == '4')
                {
                    //floor
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -6, startingPosZ + zOffset), 0));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                }
                if (line[i] == '5')
                {
                    //floor
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -6, startingPosZ + zOffset), 0));
                    voxels[voxels.size()-1].index = voxels.size()-1;
                    camera.lastPosition = glm::vec3(startingPosX + xOffset, 3, startingPosZ + zOffset);
                    camera.position = glm::vec3(startingPosX + xOffset, 3, startingPosZ + zOffset);
                }
                

                voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, 8, startingPosZ + zOffset), 0));
                    voxels[voxels.size()-1].index = voxels.size()-1;

                xOffset++;
                xOffset++;
            }
            xOffset = 0;
            zOffset++;
            zOffset++;
            
        }
    }
    else
    {
        std::cout << "FAILED to open map.txt" << std::endl;
    }

    

    std::cout << "Map built successfully! Constructing voxels from data..." << std::endl;

    
    const unsigned int AMOUNT_OF_INDICES = voxels[0].indices.size();
    const unsigned int FULL_STRIDE  = STRIDE * AMOUNT_OF_INDICES * VertexBufferElement::GetSizeOfType(GL_FLOAT);
    int indicesCount = voxels.size() * AMOUNT_OF_INDICES;
    unsigned int* indicesAfter = new unsigned int[indicesCount];
    for (int i = 0; i < voxels.size(); i++)
    {
        for (int j = 0; j < voxels[i].indices.size(); j++)
        {
            indicesAfter [i * AMOUNT_OF_INDICES + j] = voxels[i].indices[j] + i * AMOUNT_OF_INDICES;
        }
    }

    //this is the size of each tri's info (6, 3 for position, 2 for texture coordinates, 1 textureID) * 36 (number of indices in our cube)
    float* positions = new float[voxels.size() * STRIDE * AMOUNT_OF_INDICES];
    for (int i = 0; i < voxels.size(); i++)
    {
        for (int j = 0; j < voxels[i].cornerPositions.size(); j++)
        {
            positions [i * STRIDE * AMOUNT_OF_INDICES + j] = voxels[i].cornerPositions[j];

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

    if (!glfwInit()) 
    {
        std::cerr << "FAILED to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(width, height, "DDGL", NULL, NULL);
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

    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GLCall(glEnable(GL_BLEND));
    GLCall(glEnable(GL_DEPTH_TEST));

    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));



    //this is the size of each tri's info (5, 3 for position, 2 for texture coordinates) * 36 (number of indices in our cube)
    int positionCount = STRIDE * AMOUNT_OF_INDICES * voxels.size();
    VertexArray va;
    VertexBuffer vb(positions, voxels.size() * FULL_STRIDE);
    VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 3); //position
    layout.Push(GL_FLOAT, 2); //texture coordinates
    layout.Push(GL_FLOAT, 1); //textureID
    layout.Push(GL_FLOAT, 3); //normal
    va.AddBuffer(vb,layout);
    IndexBuffer ib(indicesAfter,indicesCount);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 5000.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = proj * view * model;

    std::cout << "Voxels generated! Creating shaders..." << std::endl;
    Shader shader("res/shaders/Basic.shader");

    shader.Bind();
    shader.SetUniformMat4f("u_MVP", mvp);
    std::cout << "Bound shaders successfully! Binding textures to GPU...\n";
    
    Texture u_TextureBrick1("res/textures/brick.png");
    Texture u_TextureBrick2("res/textures/brick2.png");
    Texture u_TextureBrick3("res/textures/brick3.png");
    Texture u_TextureBrick4("res/textures/brick4.png");

    u_TextureBrick1.Bind(0);
    u_TextureBrick2.Bind(1);
    u_TextureBrick3.Bind(2);
    u_TextureBrick4.Bind(3);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    std::cout << "Textures bound! Setting texture uniforms..." << std::endl;

    shader.SetUniform1i("u_TextureBrick1", 0);
    shader.SetUniform1i("u_TextureBrick2", 1);
    shader.SetUniform1i("u_TextureBrick3", 2);
    shader.SetUniform1i("u_TextureBrick4", 3);

    va.Unbind();
    vb.Unbind();
    ib.Unbind();
    shader.Unbind();

    std::cout << "Texture uniforms created! Creating RENDERER..." << std::endl;

    Renderer renderer;

    std::cout << "RENDERER created! Initializing IMGUI..." << std::endl;

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = static_cast<float>(display_w);
    io.DisplaySize.y = static_cast<float>(display_h);


    float dt = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame;
    bool paused = false;
    shader.Bind();

    std::cout << "SETUP complete!" << std::endl;

    std::vector<Cube*> voxelsCloseToPlayer;
    Cube* cubeLookingAt = nullptr;
    #pragma endregion INITIALIZATION
    
    while (!glfwWindowShouldClose(window)) 
    {
        //get direction vector from camera to camera.target
        glm::vec3 direction = camera.target - camera.position;
        direction.x = direction.x * 3;
        direction.y = direction.y * 3;
        direction.z = direction.z * 3;

        SetPosition(positions, 0, sunPosition, STRIDE);
        
        //lock to 60fps
        auto frameStart = std::chrono::steady_clock::now();
        float distToCube = 99999999;

        //print the first three elements of the position array
        //std::cout << positions[0] << " " << positions[1] << " " << positions[2] << std::endl;

        if (needsRefresh)
        {
            needsRefresh = false;
            indicesCount = voxels.size() * AMOUNT_OF_INDICES;
            indicesAfter = new unsigned int[indicesCount];
            for (int i = 0; i < voxels.size(); i++)
            {
                for (int j = 0; j < voxels[i].indices.size(); j++)
                {
                    indicesAfter [i * AMOUNT_OF_INDICES + j] = voxels[i].indices[j] + i * AMOUNT_OF_INDICES;
                }
                voxels[i].index = i;
            }

            //this is the size of each tri's info (6, 3 for position, 2 for texture coordinates, 1 textureID) * 36 (number of indices in our cube)
            positions = new float[voxels.size() * STRIDE * AMOUNT_OF_INDICES];
            for (int i = 0; i < voxels.size(); i++)
            {
                for (int j = 0; j < voxels[i].cornerPositions.size(); j++)
                {
                    positions [i * STRIDE * AMOUNT_OF_INDICES + j] = voxels[i].cornerPositions[j];

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

            va.Unbind();
            vb.Unbind();
            ib.Unbind();

            vb.UpdateBuffer(positions, voxels.size() * FULL_STRIDE);
            va.AddBuffer(vb, layout);
            ib.UpdateBuffer(indicesAfter, indicesCount);

            va.Bind();
            vb.Bind();
            ib.Bind();
        }

        //SetPosition(positions, 0, camera.target + direction, STRIDE);
        vb.UpdateBuffer(positions, voxels.size() * FULL_STRIDE);
        va.UpdateBuffer(vb, layout);

        
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            paused = true;
            mouseControl = true;
        }

        if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
        {
            sunPosition.z += 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
        {
            sunPosition.z -= 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
        {
            sunPosition.x += 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
        {
            sunPosition.x -= 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS)
        {
            sunPosition.y += 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS)
        {
            sunPosition.y -= 0.1f;
        }



#pragma region MVP
        ImGui_ImplOpenGL3_NewFrame();
        currentFrame = glfwGetTime();
        dt = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwSwapInterval(1);
        renderer.Clear();
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = proj * view * model;
        shader.SetUniformMat4f("u_MVP", mvp);
        shader.SetUniform3f("lightPos", sunPosition.x, sunPosition.y, sunPosition.z);
        renderer.Draw(va, ib, shader);

#pragma endregion MVP

#pragma region COLLISION

        voxelsCloseToPlayer.clear();
        for (int i = 0; i < voxels.size(); i++)
        {
            float distance = glm::distance(camera.position, voxels[i].position);
            if (distance < 15)
            {
                voxelsCloseToPlayer.push_back(&voxels[i]);
            }
        }
        bool onGround = false;

        camera.Update(window, dt, mouseControl);
        camera.collider.setPosition(camera.position + glm::vec3(0,-4,0));

        camera.pointXMinusColliding = false;
        camera.pointXPlusColliding = false;
        camera.pointZMinusColliding = false;
        camera.pointZPlusColliding = false;

        std::vector<glm::vec3> pointsToCheck;

        for (int j = 0; j < 6; j++)
        {
            glm::vec3 pointToCheck = CastPointForward((j+1)*2, camera.GetPosition());
            pointsToCheck.push_back(pointToCheck);
            
        }

        cubeLookingAt = nullptr;

        for (int i = 0; i < voxelsCloseToPlayer.size(); i++)
        {
            //check if the camera's collider is colliding with the cube's collider
            if (camera.collider.CheckCollision(voxelsCloseToPlayer[i]->collider))
            {
                glm::vec3 buffer = camera.collider.ResolveCollision(voxelsCloseToPlayer[i]->collider);
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
            }

            if (voxelsCloseToPlayer[i]->isPointInside(camera.pointXPlus))
            {
                camera.pointXPlusColliding = true;
            }
            if (voxelsCloseToPlayer[i]->isPointInside(camera.pointXMinus))
            {
                camera.pointXMinusColliding = true;
            }
            if (voxelsCloseToPlayer[i]->isPointInside(camera.pointZPlus))
            {
                camera.pointZPlusColliding = true;
            }
            if (voxelsCloseToPlayer[i]->isPointInside(camera.pointZMinus))
            {
                camera.pointZMinusColliding = true;
            }


            for (int j = 0; j < 6; j++)
            {
                if (voxelsCloseToPlayer[i]->isPointInside(pointsToCheck[j]))
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
            if (!camera.getOnGround() && voxelsCloseToPlayer[i]->isPointInside(camera.positionFeet + glm::vec3(0,-1,0)))
            {
                onGround = true;
            }
        }

        //MUST BE RIGHT AFTER COLLISION/CORRECTION VVVVVV
        camera.target = camera.position + glm::vec3(
            cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch)),
            sin(glm::radians(camera.pitch)),
            sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch))
        );

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

        //if left mouse is pressed
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            cooldownForBreak++;
            if (cooldownForBreak > 5)
            {
                if (cubeLookingAt != nullptr)
                {
                    voxels.erase(std::remove_if(voxels.begin(), voxels.end(), [cubeLookingAt](const Cube& voxel) {
                        return &voxel == cubeLookingAt;
                    }), voxels.end());
                    needsRefresh = true;
                }
                cooldownForBreak = 0;
            }
        }


#pragma region IMGUI

        ImGui::NewFrame();

        ImGui::Begin("CAMERA", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Camera Position: (%f, %f, %f)", camera.position.x, camera.position.y, camera.position.z);
        ImGui::Text("Yaw: %f", camera.yaw);
        ImGui::Text("Pitch: %f", camera.pitch);
        ImGui::Text("Roll: %f", camera.roll);
        ImGui::Text("isMouseAllowed?: %s", mouseControl ? "True" : "False");
        ImGui::Text("OnGround?: %s", onGround ? "True" : "False");
        ImGui::Text("IsJumping?: %s", camera.isJumping ? "True" : "False");
        ImGui::Text("YVelocity: %f", camera.yVelocity);
        ImGui::End();

        ImGui::Begin(" ", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("voxels: %lu", voxels.size());
        ImGui::Text("Indices: %d", indicesCount);
        ImGui::Text("Triangles: %lu", voxels.size()*12);
        ImGui::End();

        if (cubeLookingAt != nullptr)
        {
            ImGui::Begin("CubeLookingAt", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Position: (%f, %f, %f)", cubeLookingAt->position.x, cubeLookingAt->position.y, cubeLookingAt->position.z);
            ImGui::End();

            
        }
        
        
        if (camera.position.y < -10)
        {
            camera.position = camera.lastPosition;
            camera.yVelocity = 0;
        }

        if (paused)
        {
            ImGui::Begin("PAUSED", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            //set font size to 20
            ImGui::SetWindowFontScale(2.0f);
            //set window position to center
            ImGui::SetWindowPos(ImVec2(width / 2 - 100, height / 2 - 100));

            ImGui::Spacing();
            ImGui::Spacing();
            
            if (ImGui::Button("Resume Program")) 
            {
                paused = false;
                mouseControl = false;
            }

            ImGui::Spacing();

            if (ImGui::Button("UNSTUCK")) 
            {
                camera.position = glm::vec3(0, 3.0f, 0);
                camera.yVelocity = 0;

            }

            ImGui::Spacing();
            
            if (ImGui::Button("Exit Program")) 
            {
                glfwSetWindowShouldClose(window, true);
            }

            ImGui::End();
        }

        //draw notifications from the notification queue
        for (int i = 0; i < notifications.size(); i++)
        {
            ImGui::Begin("Notification", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::SetWindowPos(ImVec2(10, 10));
            ImGui::Text("%s", notifications[i]->message.c_str());
            ImGui::End();
            notifications[i]->time -= dt;
            if (notifications[i]->time <= 0)
            {
                notifications.erase(notifications.begin() + i);
            }
        }
        

        
        
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#pragma endregion IMGUI

        GLCall(glfwSwapBuffers(window));

        // Calculate time taken for the frame
        auto frameEnd = std::chrono::steady_clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);

        // Delay to maintain 60 FPS
        std::chrono::milliseconds targetFrameDuration(1000 / 60);
        auto sleepTime = targetFrameDuration - frameDuration;
        if (sleepTime > std::chrono::milliseconds::zero()) {
            std::this_thread::sleep_for(sleepTime);
        }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

