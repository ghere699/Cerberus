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
		std::vector<glm::vec2> temp_texCoords;
		std::vector<glm::vec3> temp_normals;

		std::map<std::string, unsigned int> vertexCache;

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
			else if (prefix == "vt") {
				glm::vec2 texCoord;
				ss >> texCoord.x >> texCoord.y;
				temp_texCoords.push_back(texCoord);
			}
			else if (prefix == "vn") {
				glm::vec3 normal;
				ss >> normal.x >> normal.y >> normal.z;
				temp_normals.push_back(normal);
			}
			else if (prefix == "f") {
				std::string faceData;
				for (int i = 0; i < 3; ++i) {
					ss >> faceData; // e.g., "1/1/1" or "1//1"

					if (vertexCache.count(faceData)) {
						indices.push_back(vertexCache[faceData]);
					}
					else {
						std::stringstream face_ss(faceData);
						std::string segment;
						unsigned int vIdx = 0, vtIdx = 0, vnIdx = 0;
						std::getline(face_ss, segment, '/');
						vIdx = std::stoul(segment);

						if (face_ss.peek() != '/') {
							std::getline(face_ss, segment, '/');
							vtIdx = std::stoul(segment);
						}
						else {
							face_ss.ignore();
						}

						std::getline(face_ss, segment, '/');
						vnIdx = std::stoul(segment);

						Vertex vertex;
						vertex.Position = temp_positions[vIdx - 1];

						if (vtIdx > 0 && !temp_texCoords.empty()) {
							vertex.TexCoords = temp_texCoords[vtIdx - 1];
						}
						else {
							vertex.TexCoords = glm::vec2(0.0f, 0.0f);
						}

						vertex.Normal = temp_normals[vnIdx - 1];

						vertices.push_back(vertex);
						unsigned int newIndex = vertices.size() - 1;
						indices.push_back(newIndex);
						vertexCache[faceData] = newIndex;
					}
				}
			}
		}

		std::cout << "Loaded OBJ model: " << path << " with " << vertices.size() << " unique vertices." << std::endl;
		return std::make_unique<Mesh>(vertices, indices);
	}
}