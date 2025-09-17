#include "Grid.h"
#include <glad/glad.h>
#include <vector>
#include <gtc/matrix_transform.hpp>

namespace Cerberus {

    Grid::Grid() {
        shader_ = std::make_unique<Shader>("res/shaders/grid.vert", "res/shaders/grid.frag");

        // A simple quad that will cover a large area
        float vertices[] = {
            // positions
            -1.0f, 0.0f, -1.0f,
             1.0f, 0.0f, -1.0f,
             1.0f, 0.0f,  1.0f,

            -1.0f, 0.0f, -1.0f,
             1.0f, 0.0f,  1.0f,
            -1.0f, 0.0f,  1.0f
        };

        vertexCount_ = 6;

        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);

        glBindVertexArray(VAO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute only
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    Grid::~Grid() {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
    }

    void Grid::Draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos) {
        //disable culling
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //force solid mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDepthMask(GL_FALSE);

        shader_->Use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cameraPos.x, 0.0f, cameraPos.z));
        model = glm::scale(model, glm::vec3(200.0f)); 

        shader_->SetMat4("u_Model", model);
        shader_->SetMat4("u_View", view);
        shader_->SetMat4("u_Projection", projection);
        shader_->SetVec3("u_CameraPos", cameraPos);

        glBindVertexArray(VAO_);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
        glBindVertexArray(0);

        // Restore OpenGL state for the rest of the scene
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}