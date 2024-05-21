#include <glm/glm.hpp>

class Light
{
  public:
    glm::vec4 rgba{1, 1, 1, 1};
    glm::vec3 position{0, 0, 0};
    glm::vec3 direction{1, 0, 1};
    bool down = true;
    float brightness = 1;

    Light(glm::vec3 position)
    {

        this->position = glm::vec3((int)position.x, (int)position.y, (int)position.z);
    }

    glm::vec3 &GetPosition()
    {
        return position;
    }

    glm::vec3 &GetDirection()
    {
        return direction;
    }

    glm::vec4 &GetRGBA()
    {
        return rgba;
    }

    void Update()
    {
    }
};
