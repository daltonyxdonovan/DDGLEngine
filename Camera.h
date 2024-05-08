#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <cstring>
#include "CubeCollider.h"


class Camera {
public:
    glm::mat4 proj;
    int cooldown = 0;
    glm::vec3 position = glm::vec3(0.0f, 3.0f, 0.0f);
    glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    float yaw = -90.0f; // Starting yaw angle
    float pitch = 0.0f;  // Starting pitch angle
    float roll = 0.0f;   // Starting roll angle
    glm::vec3 positionFeet = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 colliderPointTopBackLeft;
    glm::vec3 colliderPointTopBackRight;
    glm::vec3 colliderPointTopFrontLeft;
    glm::vec3 colliderPointTopFrontRight;
    glm::vec3 colliderPointBottomBackLeft;
    glm::vec3 colliderPointBottomBackRight;
    glm::vec3 colliderPointBottomFrontLeft;
    glm::vec3 colliderPointBottomFrontRight;  
    float yVelocity = 0.0f;
    bool onGround = false;
    bool isJumping = false;

    CubeCollider collider;



    Camera(glm::vec3 position = glm::vec3(0.0f, 3.0f, 0.0f),
           glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float movementSpeed = 5.0f,
           float rotationSpeed = 1.0f) :
        position(position), target(target), worldUp(up),
        movementSpeed(movementSpeed), rotationSpeed(rotationSpeed) {

             glfwSetJoystickCallback([](int joystick, int event) {
                if (event == GLFW_CONNECTED) {
                    // Controller connected
                    std::cout << "Controller connected: " << glfwGetJoystickName(joystick) << std::endl;
                } else if (event == GLFW_DISCONNECTED) {
                    // Controller disconnected
                    std::cout << "Controller disconnected: " << glfwGetJoystickName(joystick) << std::endl;
                }
            });

        }

    // Accessors
    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, target, worldUp);
    }

    bool getOnGround() const {
        return onGround;
    }

    // Rotation functions (assuming mouse movement for rotation)
    void rotate(float xoffset, float yoffset) {
        xoffset *= rotationSpeed;
        yoffset *= rotationSpeed;

        glm::vec3 front = glm::normalize(target - position);
        glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        yaw += xoffset;
        pitch += yoffset;

        // Clamp pitch to avoid looking too far up or down
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        target = position + glm::vec3(
            cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch))
        );
    }

    glm::vec3 getRay()
    {
        glm::vec3 front = glm::normalize(target - position);
        return front;
    }

    void Update(GLFWwindow* window, float dt, bool& isMouseActive)
    {

        glfwPollEvents();
        int count;
        int properControllerIndex = -1; 
        const unsigned char* axes;
        const unsigned char* buttons;
        const float* thumbstickAxes;
        int amountOfJoysticksConnected = 0;
        float cameraSpeed = .1f;
        float rotationSpeed = .1f;

        positionFeet = glm::vec3(position.x, position.y-3, position.z);

        //collider.setPosition(position);



        for (int i = 0; i < 5; i++)
        {
            if (glfwJoystickPresent(i))
                amountOfJoysticksConnected++;
        }
        for (int j = 0; j < amountOfJoysticksConnected; j++)
        {
            const char* name = glfwGetJoystickName(j);
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
        





        if (cooldown > 0)
            cooldown--;

        

        //if tab is pressed, toggle isMouseActive
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && cooldown == 0)
        {
            isMouseActive = !isMouseActive;
            cooldown = 20;
        }
    
        if (isMouseActive)
        {
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

                //move the mouse using the controller
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                glfwSetCursorPos(window, xpos + xoffsetController, ypos + yoffsetController);
            }
        }
        else
        {
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
                    xoffsetController = thumbstickAxes[2] * 8;
                float yoffsetController = 0;
                if (thumbstickAxes[3] > 0.1 || thumbstickAxes[3] < -0.1)
                    yoffsetController = -thumbstickAxes[3] * 8;

                rotate(xoffsetController * rotationSpeed, yoffsetController * rotationSpeed);
            }
        }

        if (properControllerIndex != -1)
        {
            if ( axes[0] == 1 || thumbstickAxes[1] < -0.5) {
                glm::vec3 movement = cameraSpeed * glm::normalize(target - position);
                movement.y = 0;
                position += movement;
            }

            if (axes[0] == 4 || thumbstickAxes[1] > 0.5) {
                glm::vec3 movement = cameraSpeed * glm::normalize(target - position);
                movement.y = 0;
                position -= movement;
            }

            if (axes[0] == 8 || thumbstickAxes[0] < -0.5)
            {    
                glm::vec3 movement = cameraSpeed * glm::normalize(glm::cross(target - position, worldUp));
                movement.y = 0;
                position -= movement;
            }

            if (axes[0] == 2 || thumbstickAxes[0] > 0.5)
            {
                glm::vec3 movement = cameraSpeed * glm::normalize(glm::cross(target - position, worldUp));
                movement.y = 0;
                position += movement;
            }


            if (buttons[0] == GLFW_PRESS)
            {
                if (onGround)
                {
                    yVelocity = .1f;
                    onGround = false;
                    isJumping = true;
                }

            }

            

            if (buttons[1] == GLFW_PRESS)
            {
                
            }








        }




        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            glm::vec3 movement = cameraSpeed * glm::normalize(target - position);
            movement.y = 0;
            position += movement;
        }



        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            glm::vec3 movement = cameraSpeed * glm::normalize(target - position);
            movement.y = 0;
            position -= movement;
        }


        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {    
            glm::vec3 movement = cameraSpeed * glm::normalize(glm::cross(target - position, worldUp));
            movement.y = 0;
            position -= movement;
        }


        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            glm::vec3 movement = cameraSpeed * glm::normalize(glm::cross(target - position, worldUp));
            movement.y = 0;
            position += movement;
        }


        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (onGround)
            {
                yVelocity = .1f;
                onGround = false;
                isJumping = true;
            }
        }
        //position.y += yVelocity;
        if (!onGround)
        {
            position.y += yVelocity;
        }

        if (!onGround)
        {
            yVelocity -= 0.005f;

        }

        // if (p1Colliding || p2Colliding)
        // {
        //     glm::vec3 movement = (cameraSpeed/2) * glm::normalize(target - position);
        //     movement.y = 0;
        //     position -= movement;
        // }
        // if (p3Colliding || p4Colliding)
        // {
        //     glm::vec3 movement = (cameraSpeed/2) * glm::normalize(target - position);
        //     movement.y = 0;
        //     position += movement;
        // }
        // if (p1Colliding || p3Colliding)
        // {
        //     glm::vec3 movement = (cameraSpeed/2) * glm::normalize(glm::cross(target - position, worldUp));
        //     movement.y = 0;
        //     position += movement;
        // }
        // if (p2Colliding || p4Colliding)
        // {
        //     glm::vec3 movement = (cameraSpeed/2) * glm::normalize(glm::cross(target - position, worldUp));
        //     movement.y = 0;
        //     position -= movement;
        // }


        // if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS )
        // {
        //     glm::vec3 movement = cameraSpeed * glm::normalize(worldUp);
        //     position -= movement;
        // }





        // if (buttons[3] == GLFW_PRESS)
        //     std::cout << "X pressed" << std::endl;
        // if (buttons[4] == GLFW_PRESS)
        //     std::cout << "Y pressed" << std::endl;
        // if (buttons[6] == GLFW_PRESS)
        //     std::cout << "LEFT BUMPER pressed" << std::endl;
        // if (buttons[7] == GLFW_PRESS)
        //     std::cout << "RIGHT BUMPER pressed" << std::endl;
        // if (buttons[10] == GLFW_PRESS)
        //     std::cout << "SELECT pressed" << std::endl;
        // if (buttons[11] == GLFW_PRESS)
        //     std::cout << "START pressed" << std::endl;
        // if (buttons[13] == GLFW_PRESS)
        //     std::cout << "LEFT THUMBSTICK pressed" << std::endl;
        // if (buttons[14] == GLFW_PRESS)
        //     std::cout << "RIGHT THUMBSTICK pressed" << std::endl;
        // if (buttons[15] == GLFW_PRESS)
        //     std::cout << "UP DPAD pressed" << std::endl;
        // if (buttons[16] == GLFW_PRESS)
        //     std::cout << "RIGHT DPAD pressed" << std::endl;
        // if (buttons[17] == GLFW_PRESS)
        //     std::cout << "DOWN DPAD pressed" << std::endl;
        // if (buttons[18] == GLFW_PRESS)
        //     std::cout << "LEFT DPAD pressed" << std::endl;

        // if (axes[0] == 4)
        //     std::cout << "down DPAD pressed" << std::endl;
        // if (axes[0] == 2)
        //     std::cout << "right DPAD pressed" << std::endl;
        // if (axes[0] == 1)
        //     std::cout << "up DPAD pressed" << std::endl;
        // if (axes[0] == 8)
        //     std::cout << "left DPAD pressed" << std::endl;

        // if (thumbstickAxes[4] > 0.5)
        //     std::cout << "RIGHT TRIGGER" << std::endl;
        // if (thumbstickAxes[5] > 0.5)
        //     std::cout << "LEFT TRIGGER" << std::endl;

    }

    glm::vec3 getViewDirection() const
    {
        // Calculate the view direction vector
        return glm::normalize(target - position);
    }


private:
    
    glm::vec3 target;
    glm::vec3 worldUp;
    float movementSpeed;
    float rotationSpeed;

    glm::mat4 view;

    glm::mat4 model;
    glm::mat4 mvp;

    
};
