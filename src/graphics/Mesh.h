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
    };

    class Mesh {
    public:
        std::vector<Vertex>       m_Vertices;
        std::vector<unsigned int> m_Indices;
        float boundingRadius_;
        glm::vec3 boundingBoxMin_;
        glm::vec3 boundingBoxMax_;

        Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
        void Draw(Shader& shader);
        void DrawFaceNormals();
        void DrawVertexNormals();

    private:
        unsigned int m_VAO, m_VBO, m_EBO;
        void SetupMesh();
        void SetupNormals();
        void SetupVertexNormals();

        unsigned int faceNormalsVAO_, faceNormalsVBO_;
        unsigned int faceNormalVertexCount_;

        unsigned int vertexNormalsVAO_, vertexNormalsVBO_;
        unsigned int vertexNormalVertexCount_;

    };

}