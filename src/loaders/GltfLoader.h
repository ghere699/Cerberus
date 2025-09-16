#pragma once

#include "../graphics/Mesh.h"
#include <string>
#include <memory>

namespace Cerberus {
    class GltfLoader {
    public:
        static std::unique_ptr<Mesh> LoadModel(const std::string& path);
    };
}