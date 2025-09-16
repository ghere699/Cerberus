#include "ObjLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

namespace Cerberus {

    std::unique_ptr<Mesh> ObjLoader::LoadModel(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open model file: " << path << std::endl;
            return nullptr;
        }

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec3> temp_normals;

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {
                glm::vec3 position;
                ss >> position.x >> position.y >> position.z;
                temp_positions.push_back(position);
            }
            else if (prefix == "vn") {
                glm::vec3 normal;
                ss >> normal.x >> normal.y >> normal.z;
                temp_normals.push_back(normal);
            }
            else if (prefix == "f") {
                std::string faceData;
                unsigned int vIndex, nIndex;

                // This logic is simple and assumes "v//vn" format
                for (int i = 0; i < 3; ++i) {
                    ss >> faceData;
                    size_t pos = faceData.find("//");
                    vIndex = std::stoul(faceData.substr(0, pos));
                    nIndex = std::stoul(faceData.substr(pos + 2));

                    // Create a vertex with the correct position and normal
                    Vertex vertex;
                    vertex.Position = temp_positions[vIndex - 1]; // .obj is 1-based
                    vertex.Normal = temp_normals[nIndex - 1];   // .obj is 1-based

                    vertices.push_back(vertex);
                    indices.push_back(vertices.size() - 1);
                }
            }
        }

        // Note: This loader is still inefficient as it creates duplicate vertices.
        // A more advanced loader would use a map to index unique vertex/normal pairs.
        // For our purposes, this is perfectly fine.

        std::cout << "Loaded model: " << path << " with " << vertices.size() << " vertices." << std::endl;

        // We now pass the final combined vertices and new indices
        return std::make_unique<Mesh>(vertices, indices);
    }  //copy pasted
}