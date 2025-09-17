#pragma once

#include "Shader.h"
#include <memory>
#include <glm.hpp>

namespace Cerberus {
    class Grid {
    public:
        Grid();
        ~Grid();

        void Draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos);

    private:
        std::unique_ptr<Shader> shader_;
        unsigned int VAO_, VBO_;
        unsigned int vertexCount_;
    };
}