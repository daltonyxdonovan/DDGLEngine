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

    Cube(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
    {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        collider.setPosition(position);
        collider.setSize(scale);
        this->cornerPositions = 
        {
            //front face
            -1, -1, 1, 0.0f, 0.0f, 0,
            1, -1, 1, 1.0f, 0.0f, 0,
            1,  1, 1, 1.0f, 1.0f, 0,
            -1,  1, 1, 0.0f, 1.0f, 0,

            //back face
            -1, -1, -1, 0.0f, 0.0f, 0,
            1, -1, -1, 1.0f, 0.0f, 0,
            1,  1, -1, 1.0f, 1.0f, 0,
            -1,  1, -1, 0.0f, 1.0f, 0,

            //top face
            -1, 1, -1, 0.0f, 0.0f, 0,
            1, 1, -1, 1.0f, 0.0f, 0,
            1, 1,  1, 1.0f, 1.0f, 0,
            -1, 1,  1, 0.0f, 1.0f, 0,

            //bottom face
            -1, -1, -1, 0.0f, 0.0f, 0,
            1, -1, -1, 1.0f, 0.0f, 0,
            1, -1,  1, 1.0f, 1.0f, 0,
            -1, -1,  1, 0.0f, 1.0f, 0,

            //right face
            1, -1, -1, 0.0f, 0.0f, 0,
            1,  1, -1, 1.0f, 0.0f, 0,
            1,  1,  1, 1.0f, 1.0f, 0,
            1, -1,  1, 0.0f, 1.0f, 0,

            //left face
            -1, -1, -1, 0.0f, 0.0f, 0,
            -1,  1, -1, 1.0f, 0.0f, 0,
            -1,  1,  1, 1.0f, 1.0f, 0,
            -1, -1,  1, 0.0f, 1.0f, 0
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

    Cube(glm::vec3 position)
    {
        this->position = position;
        this->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        this->scale = glm::vec3(2.0f, 2.0f, 2.0f);
        collider.setPosition(position);
        collider.setSize(scale);
        this->cornerPositions = 
        {
            //front face
            -1, -1, 1, 0.0f, 0.0f, 0,
            1, -1, 1, 1.0f, 0.0f, 0,
            1,  1, 1, 1.0f, 1.0f, 0,
            -1,  1, 1, 0.0f, 1.0f, 0,

            //back face
            -1, -1, -1, 0.0f, 0.0f, 0,
            1, -1, -1, 1.0f, 0.0f, 0,
            1,  1, -1, 1.0f, 1.0f, 0,
            -1,  1, -1, 0.0f, 1.0f, 0,

            //top face
            -1, 1, -1, 0.0f, 0.0f, 0,
            1, 1, -1, 1.0f, 0.0f, 0,
            1, 1,  1, 1.0f, 1.0f, 0,
            -1, 1,  1, 0.0f, 1.0f, 0,

            //bottom face
            -1, -1, -1, 0.0f, 0.0f, 0,
            1, -1, -1, 1.0f, 0.0f, 0,
            1, -1,  1, 1.0f, 1.0f, 0,
            -1, -1,  1, 0.0f, 1.0f, 0,

            //right face
            1, -1, -1, 0.0f, 0.0f, 0,
            1,  1, -1, 1.0f, 0.0f, 0,
            1,  1,  1, 1.0f, 1.0f, 0,
            1, -1,  1, 0.0f, 1.0f, 0,

            //left face
            -1, -1, -1, 0.0f, 0.0f, 0,
            -1,  1, -1, 1.0f, 0.0f, 0,
            -1,  1,  1, 1.0f, 1.0f, 0,
            -1, -1,  1, 0.0f, 1.0f, 0
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

    Cube(glm::vec3 position, float textureID)
    {
        this->position = position;glm::vec3();
        this->textureIndex = textureID;
        this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
        collider.setPosition(position);
        collider.setSize(scale);
        this->cornerPositions = 
        {
            //front face
            -1, -1, 1, 0.0f, 0.0f, textureID,
             1, -1, 1, 1.0f, 0.0f, textureID,
             1,  1, 1, 1.0f, 1.0f, textureID,
            -1,  1, 1, 0.0f, 1.0f, textureID,

            //back face
            -1, -1, -1, 0.0f, 0.0f, textureID,
             1, -1, -1, 1.0f, 0.0f, textureID,
             1,  1, -1, 1.0f, 1.0f, textureID,
            -1,  1, -1, 0.0f, 1.0f, textureID,

            //top face
            -1, 1, -1, 0.0f, 0.0f, textureID,
             1, 1, -1, 1.0f, 0.0f, textureID,
             1, 1,  1, 1.0f, 1.0f, textureID,
            -1, 1,  1, 0.0f, 1.0f, textureID,

            //bottom face
            -1, -1, -1, 0.0f, 0.0f, textureID,
             1, -1, -1, 1.0f, 0.0f, textureID,
             1, -1,  1, 1.0f, 1.0f, textureID,
            -1, -1,  1, 0.0f, 1.0f, textureID,

            //right face
            1, -1, -1, 0.0f, 0.0f, textureID,
            1,  1, -1, 1.0f, 0.0f, textureID,
            1,  1,  1, 1.0f, 1.0f, textureID,
            1, -1,  1, 0.0f, 1.0f, textureID,

            //left face
            -1, -1, -1, 0.0f, 0.0f, textureID,
            -1,  1, -1, 1.0f, 0.0f, textureID,
            -1,  1,  1, 1.0f, 1.0f, textureID,
            -1, -1,  1, 0.0f, 1.0f, textureID
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

const unsigned int STRIDE = 6;
std::vector<Notification*> notifications;

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

int main() 
{

#pragma region INITIALIZATION
    std::cout << "setting up noise values..." << std::endl;
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(4.0);
    noise.SetFractalLacunarity(5.0f);
    noise.SetFractalGain(0.5f);
    noise.SetFrequency(0.06f);
    noise.SetSeed(rand());

    


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
                if (line[i] == '#')
                {
                    //voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, 2, startingPosZ + zOffset), 3));
                    //voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, 0, startingPosZ + zOffset), 3));
                    //voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -2, startingPosZ + zOffset), 3));
                    //voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -4, startingPosZ + zOffset), 3));
                }
                if (line[i] == '1')
                {
                    
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -4, startingPosZ + zOffset), 1));
                }
                if (line[i] == '2')
                {
                    
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -2, startingPosZ + zOffset), 2));
                    voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -4, startingPosZ + zOffset), 2));
                }
                voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, -6, startingPosZ + zOffset), 0));
                //voxels.push_back(Cube(glm::vec3(startingPosX + xOffset, 4, startingPosZ + zOffset), 0));

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
        std::cout << "Failed to open file" << std::endl;
    }
    



    
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
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL Engine", NULL, NULL);
    if (!window) 
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    if (glewInit() != GLEW_OK) 
    {
        std::cerr << "Failed to initialize GLEW\n";
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
    va.AddBuffer(vb,layout);
    IndexBuffer ib(indicesAfter,indicesCount);



    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 5000.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = proj * view * model;

    

    std::cout << "creating shaders..." << std::endl;
    Shader shader("res/shaders/Basic.shader");


    shader.Bind();
    
    shader.SetUniformMat4f("u_MVP", mvp);
    std::cout << "bound shaders successfully\n";
    
    Texture u_TextureBrick1("res/textures/brick.png");
    Texture u_TextureBrick2("res/textures/brick2.png");
    Texture u_TextureBrick3("res/textures/brick3.png");
    Texture u_TextureBrick4("res/textures/brick4.png");

    std::cout << "binding textures to gpu..." << std::endl;

    u_TextureBrick1.Bind(0);
    u_TextureBrick2.Bind(1);
    u_TextureBrick3.Bind(2);
    u_TextureBrick4.Bind(3);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    std::cout << "setting texture uniforms..." << std::endl;

    shader.SetUniform1i("u_TextureBrick1", 0);
    shader.SetUniform1i("u_TextureBrick2", 1);
    shader.SetUniform1i("u_TextureBrick3", 2);
    shader.SetUniform1i("u_TextureBrick4", 3);

    va.Unbind();
    vb.Unbind();
    ib.Unbind();
    shader.Unbind();

    std::cout << "creating renderer..." << std::endl;

    Renderer renderer;

    std::cout << "initializing IMGUI" << std::endl;

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

    std::cout << "setup complete!" << std::endl;

    std::vector<Cube*> voxelsCloseToPlayer;
    Cube* cubeLookingAt = nullptr;
    #pragma endregion INITIALIZATION
    
    while (!glfwWindowShouldClose(window)) 
    {
        //lock to 60fps
        auto frameStart = std::chrono::steady_clock::now();
        float distToCube = 99999999;

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

        
        //get camera's view direction to see what block we are looking at
        Ray ray(camera.position + glm::vec3(0,1,0), camera.getViewDirection());
        
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            paused = true;
            mouseControl = true;
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
        renderer.Draw(va, ib, shader);

#pragma endregion MVP

        voxelsCloseToPlayer.clear();
        for (int i = 0; i < voxels.size(); i++)
        {
            float distance = glm::distance(camera.position, voxels[i].position);
            if (distance < 10)
            {
                voxelsCloseToPlayer.push_back(&voxels[i]);
            }
        }
        bool onGround = false;

        camera.Update(window, dt, mouseControl);
        camera.collider.setPosition(camera.position + glm::vec3(0,-4,0));

        camera.isPointInBackColliding = false;
        camera.isPointInFrontColliding = false;
        camera.isPointInLeftColliding = false;
        camera.isPointInRightColliding = false;

        for (int j = 0; j < voxelsCloseToPlayer.size(); j++)
        {
            if (voxelsCloseToPlayer[j]->isPointInside(camera.pointInFront))
            {
                camera.isPointInFrontColliding = true;
            }
            if (voxelsCloseToPlayer[j]->isPointInside(camera.pointInBack))
            {
                camera.isPointInBackColliding = true;
            }
            if (voxelsCloseToPlayer[j]->isPointInside(camera.pointInLeft))
            {
                camera.isPointInLeftColliding = true;
            }
            if (voxelsCloseToPlayer[j]->isPointInside(camera.pointInRight))
            {
                camera.isPointInRightColliding = true;
            }
        }

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
                camera.position += buffer;
            }
            if (ray.intersectsCube(voxelsCloseToPlayer[i]->position,2))
            {
                float distance = glm::distance(camera.position, voxelsCloseToPlayer[i]->position);
                if (distance < distToCube)
                {
                    distToCube = distance;
                    cubeLookingAt = voxelsCloseToPlayer[i];
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

        if (!onGround)
        {
            camera.onGround = false;
        }

        if (cubeLookingAt != nullptr)
        {
            cubeLookingAt->verticesAfterTransformation.clear();
            cubeLookingAt->UpdateVertices();

            for (int i = 0; i < cubeLookingAt->vertices.size(); i++)
            {
                cubeLookingAt->verticesAfterTransformation.push_back(glm::vec3(mvp * glm::vec4(cubeLookingAt->vertices[i], 1.0f)));
            }

            for (int i = 0; i < cubeLookingAt->verticesAfterTransformation.size(); i++)
            {
                int nextIndex = i + 1;
                if (nextIndex == cubeLookingAt->verticesAfterTransformation.size())
                {
                    nextIndex = 0;
                }
            }
        }


#pragma region IMGUI

        ImGui::NewFrame();

        if (cubeLookingAt != nullptr)
        {
            ImGui::Begin("Cube", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Cube Position: (%f, %f, %f)", cubeLookingAt->position.x, cubeLookingAt->position.y, cubeLookingAt->position.z);
            ImGui::Text("Cube Scale: (%f, %f, %f)", cubeLookingAt->scale.x, cubeLookingAt->scale.y, cubeLookingAt->scale.z);
            ImGui::Text("Cube Texture: %d", cubeLookingAt->textureIndex);
            ImGui::Text("Cube Collider: (%f, %f, %f)", cubeLookingAt->collider.position.x, cubeLookingAt->collider.position.y, cubeLookingAt->collider.position.z);
            ImGui::Text("Cube Collider Size: (%f, %f, %f)", cubeLookingAt->collider.size.x, cubeLookingAt->collider.size.y, cubeLookingAt->collider.size.z);
            ImGui::End();
        }

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

        ImGui::Begin("VAO and IB", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("VAO: %d", vao);
        ImGui::Text("IB: %d", ib.m_RendererID);
        ImGui::Text("count: %d", ib.GetCount());
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("VBE");
        ImGui::Text("Type: %d", layout.GetElements()[0].type);
        ImGui::Text("Count: %d", layout.GetElements()[0].count);
        ImGui::Text("Normalized: %d", layout.GetElements()[0].normalized);
        ImGui::End();

        ImGui::Begin("CUBES", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("voxels: %lu", voxels.size());
        ImGui::Text("Indices: %d", indicesCount);
        ImGui::Text("Triangles: %lu", voxels.size()*2);
        ImGui::End();
        
        if (camera.position.y < -10)
        {
            camera.position = glm::vec3(0, 3.0f, 0);
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
        else
        {
            //if left click on mouse
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                //if we are looking at a cube
                if (cubeLookingAt != nullptr)
                {
                    //remove the cube we are looking at
                    for (int i = 0; i < voxels.size(); i++)
                    {
                        if (voxels[i].position == cubeLookingAt->position)
                        {
                            voxels.erase(voxels.begin() + i);
                            needsRefresh = true;
                            cubeLookingAt = nullptr;
                            break;
                        }
                    }
                }
            }
            //if right click on mouse
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            {
                //if we are looking at a cube
                if (cubeLookingAt != nullptr)
                {
                    //add a cube in front of the cube we are looking at
                    glm::vec3 newCubePosition = cubeLookingAt->position + camera.getRay() * 2.0f;
                    voxels.push_back(Cube(newCubePosition));
                    needsRefresh = true;
                }
            }
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

