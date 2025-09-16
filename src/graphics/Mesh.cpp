#include "Mesh.h"
#include <glad/glad.h>

namespace Cerberus {

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
        this->m_Vertices = vertices;
        this->m_Indices = indices;

        SetupMesh();
    }

    void Mesh::SetupMesh() {
        // Create buffers/arrays
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

        // Vertex Normals
        glEnableVertexAttribArray(1); // New attribute at location 1
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        glBindVertexArray(0);

    }

    void Mesh::Draw(Shader& shader) {
        // Bind the VAO and draw the mesh
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_Indices.size()), GL_UNSIGNED_INT, 0);  //Hard AF
        glBindVertexArray(0);
    }

}