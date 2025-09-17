#include "Mesh.h"
#include <glad/glad.h>

namespace Cerberus {

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
        this->m_Vertices = vertices;
        this->m_Indices = indices;

        boundingRadius_ = 0.0f;
        for (const auto& vertex : m_Vertices) {
            float distance = glm::length(vertex.Position);
            if (distance > boundingRadius_) {
                boundingRadius_ = distance;
            }
        }
        // Add a small epsilon to prevent a radius of zero for flat planes
        if (boundingRadius_ < 0.001f) {
            boundingRadius_ = 1.0f;
        }


        SetupMesh();
        SetupNormals();
        SetupVertexNormals();
    }

    void Mesh::SetupMesh() {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // Normals
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);

    }

    void Mesh::Draw(Shader& shader) {
        // Bind the VAO and draw the mesh
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_Indices.size()), GL_UNSIGNED_INT, 0);  //Hard AF
        glBindVertexArray(0);
    }

    void Mesh::SetupNormals()
    {
        std::vector<glm::vec3> normalLines;
        float normalLength = 0.4f;

        // Iterate over each triangle
        for (size_t i = 0; i < m_Indices.size(); i += 3) {
            Vertex& v0 = m_Vertices[m_Indices[i]];
            Vertex& v1 = m_Vertices[m_Indices[i + 1]];
            Vertex& v2 = m_Vertices[m_Indices[i + 2]];

            // Calculate triangle center
            glm::vec3 center = (v0.Position + v1.Position + v2.Position) / 3.0f;

            // Calculate face normal
            glm::vec3 edge1 = v1.Position - v0.Position;
            glm::vec3 edge2 = v2.Position - v0.Position;
            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

            // Add the line to our vector
            normalLines.push_back(center);
            normalLines.push_back(center + faceNormal * normalLength);
        }

        faceNormalVertexCount_ = normalLines.size();

        if (faceNormalVertexCount_ == 0) return;

        // Create and configure buffers for the lines
        glGenVertexArrays(1, &faceNormalsVAO_);
        glGenBuffers(1, &faceNormalsVBO_);

        glBindVertexArray(faceNormalsVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, faceNormalsVBO_);
        glBufferData(GL_ARRAY_BUFFER, normalLines.size() * sizeof(glm::vec3), normalLines.data(), GL_STATIC_DRAW);

        // Position attribute for the lines
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindVertexArray(0);
    }

    void Mesh::SetupVertexNormals()
    {
        std::vector<glm::vec3> normalLines;
        float normalLength = 0.4f;

        // Iterate over each unique vertex
        for (const auto& vertex : m_Vertices) {
            normalLines.push_back(vertex.Position); // Start of the line is the vertex itself
            normalLines.push_back(vertex.Position + vertex.Normal * normalLength); // End of the line
        }

        vertexNormalVertexCount_ = normalLines.size();
        if (vertexNormalVertexCount_ == 0) return;

        // Create and configure buffers for the lines
        glGenVertexArrays(1, &vertexNormalsVAO_);
        glGenBuffers(1, &vertexNormalsVBO_);

        glBindVertexArray(vertexNormalsVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, vertexNormalsVBO_);
        glBufferData(GL_ARRAY_BUFFER, normalLines.size() * sizeof(glm::vec3), normalLines.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindVertexArray(0);

    }

    void Mesh::DrawFaceNormals()
    {
        if (faceNormalVertexCount_ == 0) return;

        glLineWidth(2.0f);
        glBindVertexArray(faceNormalsVAO_);
        glDrawArrays(GL_LINES, 0, faceNormalVertexCount_);
        glBindVertexArray(0);
    }

    void Mesh::DrawVertexNormals()
    {
        if (vertexNormalVertexCount_ == 0) return;

        glLineWidth(1.0f);
        glBindVertexArray(vertexNormalsVAO_);
        glDrawArrays(GL_LINES, 0, vertexNormalVertexCount_);
        glBindVertexArray(0);
    }

}