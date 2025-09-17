#pragma once

#include "../graphics/Mesh.h"
#include <string>
#include <memory>
#include <fstream>

namespace Cerberus {
    class StlLoader {
    public:
        static std::unique_ptr<Mesh> LoadModel(const std::string& path);

    private:
        static std::unique_ptr<Mesh> LoadASCII(std::ifstream& file);
        static std::unique_ptr<Mesh> LoadBinary(std::ifstream& file);
    };
}