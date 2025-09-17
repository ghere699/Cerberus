#include "Pivot.h"
#include <glad/glad.h>
#include <vector>

namespace Cerberus {
    PivotVisualizer::PivotVisualizer() {
        // We reuse the same simple shader as the world gizmo
        shader_ = std::make_unique<Shader>("res/shaders/gizmo.vert", "res/shaders/gizmo.frag");

        float gizmoSize = 0.4f;
        float vertices[] = {
             0.0f, 0.0f, 0.0f,              1.0f, 0.2f, 0.2f, // Origin (Dim Red)
             gizmoSize, 0.0f, 0.0f,          1.0f, 0.2f, 0.2f, // X Axis End

             0.0f, 0.0f, 0.0f,              0.2f, 1.0f, 0.2f, // Origin (Dim Green)
             0.0f, gizmoSize, 0.0f,          0.2f, 1.0f, 0.2f, // Y Axis End

             0.0f, 0.0f, 0.0f,              0.2f, 0.2f, 1.0f, // Origin (Dim Blue)
             0.0f, 0.0f, gizmoSize,          0.2f, 0.2f, 1.0f  // Z Axis End
        };

        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);

        glBindVertexArray(VAO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Color attribute (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    PivotVisualizer::~PivotVisualizer() {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
    }

    void PivotVisualizer::UpdateSize(float modelSize) {
        float gizmoAxisSize = modelSize * 0.15f;

        float vertices[] = {
             0.0f, 0.0f, 0.0f,              1.0f, 0.2f, 0.2f,
             gizmoAxisSize, 0.0f, 0.0f,      1.0f, 0.2f, 0.2f,
             0.0f, 0.0f, 0.0f,              0.2f, 1.0f, 0.2f,
             0.0f, gizmoAxisSize, 0.0f,      0.2f, 1.0f, 0.2f,
             0.0f, 0.0f, 0.0f,              0.2f, 0.2f, 1.0f,
             0.0f, 0.0f, gizmoAxisSize,      0.2f, 0.2f, 1.0f
        };

        glBindVertexArray(VAO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        // Use glBufferData to upload the new vertex data
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void PivotVisualizer::Draw(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
        glDisable(GL_DEPTH_TEST);

        glLineWidth(2.0f);
        shader_->Use();

        // Calculate the combined Model-View-Projection matrix
        glm::mat4 mvp = projection * view * model;

        // Send the single MVP matrix to the shader with the correct uniform name
        shader_->SetMat4("u_MVP", mvp);

        glBindVertexArray(VAO_);
        glDrawArrays(GL_LINES, 0, 6);
        glBindVertexArray(0);

        // Re-enable depth testing for the rest of the scene
        glEnable(GL_DEPTH_TEST);

    }
}