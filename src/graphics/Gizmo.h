#pragma once

#include "Shader.h"
#include <glm.hpp>
#include <memory>

namespace Cerberus {
    class Gizmo {
    public:
        Gizmo();
        ~Gizmo();

        void Draw(const glm::mat4& cameraView, int viewportWidth, int viewportHeight);

    private:
        std::unique_ptr<Shader> shader_;
        unsigned int VAO_, VBO_;
    };
}