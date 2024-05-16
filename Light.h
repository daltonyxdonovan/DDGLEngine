#include <glm/glm.hpp>

class Light
{
    public:
    glm::vec4 rgba {1,1,1,1};
    glm::vec3 position {0,0,0};
    glm::vec3 direction {1,0,1};
    bool down = true;

    Light(glm::vec3 position)
    {
        this->position = position;

    }

    glm::vec3& GetPosition()
    {
        return position;
    }

    glm::vec3& GetDirection()
    {
        return direction;
    }

    glm::vec4& GetRGBA()
    {
        return rgba;
    }

    void Update()
    {
        if (down)
        {
            if (position.y > 3)
                position.y -= 0.1f;
            else
                down = !down;

        }
        else
        {
            if (position.y < 6)
                position.y += 0.1f;
            else
                down = !down;
        }
    }


};