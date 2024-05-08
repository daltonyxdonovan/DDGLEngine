#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"

class Object {
private:
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    VertexArray* va;
    VertexBuffer* vb;

    Object(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, VertexArray* va, VertexBuffer* vb) {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        this->va = va;
        this->vb = vb;
    }

    

};
