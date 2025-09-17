#pragma once

#include <glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"

namespace Cerberus {

    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        //normal
    };

    class Mesh {
    public:
        std::vector<Vertex>       m_Vertices;
        std::vector<unsigned int> m_Indices;

        Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
        void Draw(Shader& shader);

    private:
        unsigned int m_VAO, m_VBO, m_EBO;
        void SetupMesh();
    };

}