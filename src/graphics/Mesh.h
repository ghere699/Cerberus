#pragma once

#include <glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"

namespace Cerberus {

    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        // We will add Normals, TexCoords, etc. here later
    };

    class Mesh {
    public:
        // Mesh Data
        std::vector<Vertex>       m_Vertices;
        std::vector<unsigned int> m_Indices;

        Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
        void Draw(Shader& shader);

    private:
        // Render data
        unsigned int m_VAO, m_VBO, m_EBO;
        void SetupMesh();
    };

}