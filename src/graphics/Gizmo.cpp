#include "Gizmo.h"
#include <glad/glad.h>
#include <vector>
#include <gtc/matrix_transform.hpp>
#include <imgui.h>

namespace Cerberus {
    Gizmo::Gizmo() {
        shader_ = std::make_unique<Shader>("res/shaders/gizmo.vert", "res/shaders/gizmo.frag");

        float vertices[] = {
             0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
             1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,

             0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
             0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,

             0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
             0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f 
        };

        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);

        glBindVertexArray(VAO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    Gizmo::~Gizmo() {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
    }

    void Gizmo::Draw(const glm::mat4& cameraView, int viewportWidth, int viewportHeight) {
        GLint oldViewport[4];
        glGetIntegerv(GL_VIEWPORT, oldViewport);

        int gizmoSize = 150;
        int xPos = oldViewport[2] - gizmoSize - 10;
        int yPos = 10;
        glViewport(xPos, yPos, gizmoSize, gizmoSize);

        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 gizmoRotation = glm::mat4(glm::mat3(cameraView));

        glm::mat4 gizmoTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

        glm::mat4 gizmoView = gizmoTranslation * gizmoRotation;

        glm::mat4 gizmoProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

        glLineWidth(3.0f);

        shader_->Use();
        shader_->SetMat4("u_View", gizmoView);
        shader_->SetMat4("u_Projection", gizmoProjection);

        glBindVertexArray(VAO_);
        glDrawArrays(GL_LINES, 0, 6);
        glBindVertexArray(0);

        glm::vec3 axisEndpoints[] = {
            glm::vec3(1.1f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.1f, 0.0f),
            glm::vec3{0.0f, 0.0f, 1.1f}
        };
        const char* label[3] = { "X", "Y", "Z" };
        ImU32 colors[] = { IM_COL32(255, 50, 50, 255), IM_COL32(50, 255, 50, 255), IM_COL32(50, 50, 255, 255) };
        ImDrawList* drawList = ImGui::GetForegroundDrawList();

        for (int i = 0; i < 3; ++i) {
            glm::vec4 clipPos = gizmoProjection * gizmoView * glm::vec4(axisEndpoints[i], 1.0);
            glm::vec3 ndc = glm::vec3(clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w);
            float screenX = (ndc.x + 1.0f) * 0.5f * gizmoSize + xPos;
            float gizmoBottomY_fromTop = viewportHeight - yPos;
            float screenY = gizmoBottomY_fromTop - (ndc.y + 1.0f) * 0.5f * gizmoSize;
            ImVec2 textSize = ImGui::CalcTextSize(label[i]);
            ImVec2 textPos = ImVec2(screenX - textSize.x * 0.5f, screenY - textSize.y * 0.5f);

            drawList->AddText(textPos, colors[i], label[i]);
        }

        // Restore the original viewport
        glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
    }
}