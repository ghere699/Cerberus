#pragma once

#include "../graphics/Mesh.h"
#include <string>
#include <memory>
#include "tiny_gltf.h"
namespace Cerberus {
    class GltfLoader {
    public:
        static std::unique_ptr<Mesh> LoadModel(const std::string& path);
        static std::unique_ptr<Mesh> LoadModelFromMemory(const unsigned char* data, size_t size);

    private:
        static std::unique_ptr<Mesh> ProcessModel(const tinygltf::Model& model);
    };
}