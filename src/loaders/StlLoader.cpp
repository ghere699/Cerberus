#include "StlLoader.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <gtc/type_ptr.hpp>

namespace Cerberus {

    std::unique_ptr<Mesh> StlLoader::LoadModel(const std::string& path) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file) {
            std::cerr << "Error: Could not open STL file: " << path << std::endl;
            return nullptr;
        }

        char header[6] = { 0 };
        file.read(header, 5);
        file.seekg(0, std::ios::beg); 

        if (std::string(header, 5) == "solid") {
            std::cout << "Loading ASCII STL file..." << std::endl;
            return LoadASCII(file);
        }
        else {
            std::cout << "Loading Binary STL file..." << std::endl;
            return LoadBinary(file);
        }
    }

    std::unique_ptr<Mesh> StlLoader::LoadASCII(std::ifstream& file) {
        std::vector<Vertex> vertices;
        std::string line;
        glm::vec3 currentNormal(0.0f);

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string keyword;
            ss >> keyword;

            if (keyword == "facet") {
                ss >> keyword; // "normal"
                ss >> currentNormal.x >> currentNormal.y >> currentNormal.z;
            }
            else if (keyword == "vertex") {
                Vertex v;
                v.Normal = currentNormal;
                ss >> v.Position.x >> v.Position.y >> v.Position.z;
                vertices.push_back(v);
            }
        }

        // STL is unindexed so we generate indices
        std::vector<unsigned int> indices(vertices.size());
        for (unsigned int i = 0; i < vertices.size(); ++i) {
            indices[i] = i;
        }

        std::cout << "Loaded STL model with " << vertices.size() << " vertices." << std::endl;
        return std::make_unique<Mesh>(vertices, indices);
    }

    std::unique_ptr<Mesh> StlLoader::LoadBinary(std::ifstream& file) {
        std::vector<Vertex> vertices;

        file.seekg(80, std::ios::beg);

        uint32_t numTriangles = 0;
        file.read(reinterpret_cast<char*>(&numTriangles), sizeof(uint32_t));

        vertices.reserve(numTriangles * 3);

        for (uint32_t i = 0; i < numTriangles; ++i) {
            float normal[3], v1[3], v2[3], v3[3];

            file.read(reinterpret_cast<char*>(normal), 3 * sizeof(float));
            file.read(reinterpret_cast<char*>(v1), 3 * sizeof(float));
            file.read(reinterpret_cast<char*>(v2), 3 * sizeof(float));
            file.read(reinterpret_cast<char*>(v3), 3 * sizeof(float));

            file.seekg(2, std::ios::cur);

            glm::vec3 n = glm::make_vec3(normal);

            vertices.push_back({ glm::make_vec3(v1), n });
            vertices.push_back({ glm::make_vec3(v2), n });
            vertices.push_back({ glm::make_vec3(v3), n });
        }

        //same as above
        std::vector<unsigned int> indices(vertices.size());
        for (unsigned int i = 0; i < vertices.size(); ++i) {
            indices[i] = i;
        }

        std::cout << "Loaded STL model with " << vertices.size() << " vertices." << std::endl;
        return std::make_unique<Mesh>(vertices, indices);
    }

}