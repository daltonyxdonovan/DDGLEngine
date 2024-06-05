#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

class InputHandler
{
  public:
    struct ControllerConfig
    {
        int axisMoveForward;
        int axisMoveBackward;
        int axisMoveLeft;
        int axisMoveRight;
        int axisLookHorizontal;
        int axisLookVertical;
        int buttonRun;
        int buttonJump;
        int buttonPause;
        int buttonUnpause;
        int buttonPlace;
        int buttonDelete;
    };

    InputHandler()
    {
        // Define default controller configuration for Xbox One S controller
        ControllerConfig xboxConfig = {
            1,  // axisMoveForward
            1,  // axisMoveBackward
            0,  // axisMoveLeft
            0,  // axisMoveRight
            2,  // axisLookHorizontal
            3,  // axisLookVertical
            13, // buttonRun
            0,  // buttonJump
            7,  // buttonPause
            6,  // buttonUnpause
            4,  // buttonPlace
            5   // buttonDelete
        };
        controllerConfigs["Xbox One S"] = xboxConfig;
    }

    void Update(GLFWwindow *window, float dt, bool &isMouseActive, int energy, bool recharging, bool needsRecharging,
                bool &placeSomething, bool &deleteSomething)
    {
        // (Your update logic here, refactored to use the InputHandler's methods and members)

        // Poll events
        glfwPollEvents();

        // Handle controller input
        int controllerIndex = GetConnectedControllerIndex();
        if (controllerIndex != -1)
        {
            const unsigned char *buttons = glfwGetJoystickButtons(controllerIndex, &buttonCount);
            const float *axes = glfwGetJoystickAxes(controllerIndex, &axisCount);

            if (buttons && axes)
            {
                std::string controllerName = glfwGetJoystickName(controllerIndex);
                ControllerConfig config = GetControllerConfig(controllerName);

                // Check buttons and axes based on the config
                if (buttons[config.buttonPause] == GLFW_PRESS)
                {
                    isMouseActive = true;
                    cooldown = 20;
                    std::cout << "Pause pressed!" << std::endl;
                }
                if (buttons[config.buttonUnpause] == GLFW_PRESS)
                {
                    isMouseActive = false;
                    cooldown = 20;
                    std::cout << "Unpause pressed!" << std::endl;
                }

                // (More logic here for movement and actions)
            }
        }

        // Handle keyboard input
        // (Your existing keyboard input logic here)
    }

    void AddControllerConfig(const std::string &name, const ControllerConfig &config)
    {
        controllerConfigs[name] = config;
    }

  private:
    std::unordered_map<std::string, ControllerConfig> controllerConfigs;
    int axisCount, buttonCount;
    int cooldown = 0;

    int GetConnectedControllerIndex()
    {
        for (int i = 0; i < GLFW_JOYSTICK_LAST; ++i)
        {
            if (glfwJoystickPresent(i))
            {
                return i;
            }
        }
        return -1;
    }

    ControllerConfig GetControllerConfig(const std::string &name)
    {
        for (const auto &pair : controllerConfigs)
        {
            if (name.find(pair.first) != std::string::npos)
            {
                return pair.second;
            }
        }
        // Return a default config if no matching config is found
        return controllerConfigs["Xbox One S"];
    }
};
