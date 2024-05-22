#include "CubeCollider.h"
#include <GLFW/glfw3.h>
#include <cstring>
#include <glm/glm.hpp>
#include <iostream>

class Camera
{
  public:
    glm::mat4 proj;
    int cooldown = 0;
    glm::vec3 position = glm::vec3(0.0f, 3.0f, 0.0f);
    glm::vec3 lastPosition = glm::vec3(0.0f, 3.0f, 0.0f); // this is used for checkpoints
    glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    float yaw = -90.0f; // Starting yaw angle
    float pitch = 0.0f; // Starting pitch angle
    float roll = 0.0f;  // Starting roll angle
    glm::vec3 positionFeet = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 pointXPlus = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 pointXMinus = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 pointZPlus = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 pointZMinus = glm::vec3(0.0f, 0.0f, 0.0f);
    bool pointXPlusColliding = false;
    bool pointXMinusColliding = false;
    bool pointZPlusColliding = false;
    bool pointZMinusColliding = false;
    float yVelocity = 0.0f;
    float jumpHeight = 0.001f;
    int maxVelocity = 2;
    bool onGround = false;
    bool isJumping = false;
    CubeCollider collider;
    bool isRunning = false;
    glm::vec3 target;
    bool isFlying = true;
    float fov = 75;

    Camera(glm::vec3 position = glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float movementSpeed = 5.0f, float rotationSpeed = 1.0f)
        : position(position), target(target), worldUp(up), movementSpeed(movementSpeed), rotationSpeed(rotationSpeed)
    {

        glfwSetJoystickCallback([](int joystick, int event) {
            if (event == GLFW_CONNECTED)
            {
                // Controller connected
                std::cout << "Controller connected: " << glfwGetJoystickName(joystick) << std::endl;
            }
            else if (event == GLFW_DISCONNECTED)
            {
                // Controller disconnected
                std::cout << "Controller disconnected: " << glfwGetJoystickName(joystick) << std::endl;
            }
        });
    }

    /*glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(position, target, worldUp);
    }*/

    bool getOnGround() const
    {
        return onGround;
    }

    void rotate(float xoffset, float yoffset)
    {
        xoffset *= rotationSpeed;
        yoffset *= rotationSpeed;

        glm::vec3 front = glm::normalize(target - position);
        glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        yaw += xoffset;
        pitch += yoffset;

        // Clamp pitch to avoid looking too far up or down
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }

    glm::vec3 getRay()
    {
        glm::vec3 front = glm::normalize(target - position);
        return front;
    }

    void Update(GLFWwindow *window, float dt, bool &isMouseActive)
    {
        // if (dt > 0.016f)
        //     dt = 0.016f;

        bool moveForward = false;
        bool moveBackward = false;
        bool moveLeft = false;
        bool moveRight = false;
        glfwPollEvents();
        int count;
        int properControllerIndex = -1;
        const unsigned char *axes = nullptr;    // Initialize to nullptr
        const unsigned char *buttons = nullptr; // Initialize to nullptr
        const float *thumbstickAxes = nullptr;
        int amountOfJoysticksConnected = 0;
        float cameraSpeed = 250;
        float rotationSpeed = .25f;

        positionFeet = glm::vec3(position.x, position.y - 4, position.z);
        pointXPlus = glm::vec3(position.x + .75f, position.y - 2.5f, position.z);
        pointXMinus = glm::vec3(position.x - .75f, position.y - 2.5f, position.z);
        pointZPlus = glm::vec3(position.x, position.y - 2.5f, position.z + .75f);
        pointZMinus = glm::vec3(position.x, position.y - 2.5f, position.z - .75f);

        for (int i = 0; i < 5; i++)
        {
            if (glfwJoystickPresent(i))
                amountOfJoysticksConnected++;
        }
        for (int j = 0; j < amountOfJoysticksConnected; j++)
        {
            const char *name = glfwGetJoystickName(j);
            if (strstr(name, "XBox") != nullptr)
            {
                properControllerIndex = j;
                break;
            }
            else if (strstr(name, "controller") != nullptr)
            {
                properControllerIndex = j;
                break;
            }
            else if (strstr(name, "Controller") != nullptr)
            {
                properControllerIndex = j;
                break;
            }
        }

        if (properControllerIndex != -1)
        {
            axes = glfwGetJoystickHats(properControllerIndex, &count);
            buttons = glfwGetJoystickButtons(properControllerIndex, &count);
            thumbstickAxes = glfwGetJoystickAxes(properControllerIndex, &count);

            if (buttons[11] == GLFW_PRESS)
            {
                isMouseActive = true;
                cooldown = 20;
            }
            if (buttons[12] == GLFW_PRESS)
            {
                isMouseActive = false;
                cooldown = 20;
            }
        }

        // if leftShift is pressed, run
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        {
            isRunning = true;
        }
        if (isFlying && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            position.y -= .1f;
        }
        /*else if (buttons != nullptr && buttons.size() > 6 && buttons[6] == GLFW_PRESS || buttons[7] == GLFW_PRESS)
        {
            isRunning = true;
            cameraSpeed = .04f;
        }*/
        if (isRunning)
        {
            cameraSpeed = 500 * dt;
        }
        else
        {
            cameraSpeed = 250 * dt;
        }

        if (cooldown > 0)
            cooldown--;

        // if tab is pressed, toggle isMouseActive
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && cooldown == 0)
        {
            isMouseActive = !isMouseActive;
            cooldown = 20;
        }

        if (isMouseActive)
        {
            // in the pause menu
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            return;

            if (properControllerIndex != -1)
            {
                float xoffsetController = 0;
                if (thumbstickAxes[2] > 0.1 || thumbstickAxes[2] < -0.1)
                    xoffsetController = thumbstickAxes[2] * 4;
                float yoffsetController = 0;
                if (thumbstickAxes[3] > 0.1 || thumbstickAxes[3] < -0.1)
                    yoffsetController = -thumbstickAxes[3] * 4;

                // move the mouse using the controller
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                glfwSetCursorPos(window, xpos + xoffsetController, ypos + yoffsetController);
            }
        }
        else
        {
            // actually in the game
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width / 2, height / 2);
            float xoffset = xpos - width / 2;
            float yoffset = height / 2 - ypos;

            rotate(xoffset * rotationSpeed, yoffset * rotationSpeed);

            if (properControllerIndex != -1)
            {
                float xoffsetController = 0;
                if (thumbstickAxes[2] > 0.1 || thumbstickAxes[2] < -0.1)
                    xoffsetController = thumbstickAxes[2] * 16;
                float yoffsetController = 0;
                if (thumbstickAxes[3] > 0.1 || thumbstickAxes[3] < -0.1)
                    yoffsetController = -thumbstickAxes[3] * 16;

                rotate(xoffsetController * rotationSpeed, yoffsetController * rotationSpeed);
            }
        }

        if (properControllerIndex != -1 && axes != nullptr && buttons != nullptr && thumbstickAxes != nullptr)
        {
            if (axes[0] == 1 || thumbstickAxes[1] < -0.5)
            {
                glm::vec3 movement = cameraSpeed * glm::normalize(target - position);
                movement.y = 0;
                velocity += movement;
                moveForward = true;
            }

            if (axes[0] == 4 || thumbstickAxes[1] > 0.5)
            {
                glm::vec3 movement = cameraSpeed * glm::normalize(target - position);
                movement.y = 0;
                velocity -= movement;
                moveBackward = true;
            }

            if (axes[0] == 8 || thumbstickAxes[0] < -0.5)
            {
                glm::vec3 movement = cameraSpeed * glm::normalize(glm::cross(target - position, worldUp));
                movement.y = 0;
                velocity -= movement;
                moveLeft = true;
            }

            if (axes[0] == 2 || thumbstickAxes[0] > 0.5)
            {
                glm::vec3 movement = cameraSpeed * glm::normalize(glm::cross(target - position, worldUp));
                movement.y = 0;
                velocity += movement;
                moveRight = true;
            }

            if (buttons[0] == GLFW_PRESS)
            {
                if (onGround)
                {
                    yVelocity = .2f;
                    onGround = false;
                    isJumping = true;
                }
            }

            if (buttons[1] == GLFW_PRESS)
            {
            }
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            glm::vec3 movement = dt * cameraSpeed * glm::normalize(target - position);
            movement.y = 0;

            velocity += movement;
            moveForward = true;
        }
        else
            isRunning = false;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            glm::vec3 movement = dt * cameraSpeed * glm::normalize(target - position);
            movement.y = 0;
            velocity -= movement;
            moveBackward = true;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            glm::vec3 movement = dt * cameraSpeed * glm::normalize(glm::cross(target - position, worldUp));
            movement.y = 0;
            velocity -= movement;
            moveLeft = true;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            glm::vec3 movement = dt * cameraSpeed * glm::normalize(glm::cross(target - position, worldUp));
            movement.y = 0;
            velocity += movement;
            moveRight = true;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (!isFlying)
            {

                if (onGround)
                {
                    yVelocity =
                        sqrtf(2.0f * jumpHeight * 9.81f); // Calculate initial velocity based on desired jump height
                    onGround = false;
                    isJumping = true;
                }
            }
            else
            {
                position.y += .1f;
            }
        }

        if (velocity.x > maxVelocity)
            velocity.x = maxVelocity;
        if (velocity.x < -maxVelocity)
            velocity.x = -maxVelocity;

        if (yVelocity > 1)
            yVelocity = 1;
        if (yVelocity < -1)
            yVelocity = -1;

        if (velocity.z > maxVelocity)
            velocity.z = maxVelocity;
        if (velocity.z < -maxVelocity)
            velocity.z = -maxVelocity;

        if (!isFlying && !onGround)
        {
            yVelocity -= 0.4f * dt; // Multiply by dt for consistent gravity
            position.y += yVelocity;
        }

        velocity *= 0.9f;
        position += velocity;

        target = position + glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)),
                                      sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
    }

    glm::vec3 &GetPosition()
    {
        return position;
    }

    glm::vec3 getViewDirection() const
    {
        // Calculate the view direction vector
        return glm::normalize(target - position);
    }

  private:
    glm::vec3 worldUp;
    float movementSpeed;
    float rotationSpeed;

    glm::mat4 view;

    glm::mat4 model;
    glm::mat4 mvp;
};
