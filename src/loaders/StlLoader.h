#pragma once

#include "../graphics/Mesh.h"
#include <string>
#include <memory>
#include <sstream>

namespace Cerberus {
    class StlLoader {
    public:
        static std::unique_ptr<Mesh> LoadModel(const std::string& path);
        static std::unique_ptr<Mesh> LoadModelFromMemory(const char* data, size_t size);

    private:
        static std::unique_ptr<Mesh> LoadASCII(std::stringstream& stream);
        static std::unique_ptr<Mesh> LoadBinary(std::stringstream& stream);
    };
}