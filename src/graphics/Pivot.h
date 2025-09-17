#pragma once

#include "Shader.h"
#include <memory>
#include <glm.hpp>

namespace Cerberus {
    class PivotVisualizer {
    public:
        PivotVisualizer();
        ~PivotVisualizer();

        void UpdateSize(float modelSize);
        void Draw(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);

    private:
        std::unique_ptr<Shader> shader_;
        unsigned int VAO_, VBO_;
    };
}