#include "StlLoader.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <gtc/type_ptr.hpp>

namespace Cerberus {

	std::unique_ptr<Mesh> StlLoader::LoadModel(const std::string& path) {
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if (!file) {
			std::cerr << "Could not open STL file: " << path << std::endl;
			return nullptr;
		}

		size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return LoadModelFromMemory(buffer.data(), buffer.size());
	}

	// core parsing dispatcher
	std::unique_ptr<Mesh> StlLoader::LoadModelFromMemory(const char* data, size_t size) {
		if (size < 5) return nullptr;

		// check for "solid" keyword
		std::string header(data, 5);
		std::string content(data, size);
		std::stringstream stream(content);

		if (header == "solid") {
			std::cout << "Parsing ASCII STL from memory..." << std::endl;
			return LoadASCII(stream);
		}
		else {
			std::cout << "Parsing Binary STL from memory..." << std::endl;
			return LoadBinary(stream);
		}
	}

	std::unique_ptr<Mesh> StlLoader::LoadASCII(std::stringstream& stream) {
		std::vector<Vertex> vertices;
		std::string line;
		glm::vec3 currentNormal(0.0f);

		while (std::getline(stream, line)) {
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

		std::vector<unsigned int> indices(vertices.size());
		for (unsigned int i = 0; i < vertices.size(); ++i) indices[i] = i;

		std::cout << "Loaded STL model with " << vertices.size() << " vertices." << std::endl;
		return std::make_unique<Mesh>(vertices, indices);
	}

	std::unique_ptr<Mesh> StlLoader::LoadBinary(std::stringstream& stream) {
		std::vector<Vertex> vertices;

		stream.seekg(80, std::ios::beg);

		uint32_t numTriangles = 0;
		stream.read(reinterpret_cast<char*>(&numTriangles), sizeof(uint32_t));
		vertices.reserve(numTriangles * 3);

		for (uint32_t i = 0; i < numTriangles; ++i) {
			float normal[3], v1[3], v2[3], v3[3];
			uint16_t attributeByteCount;

			stream.read(reinterpret_cast<char*>(normal), 3 * sizeof(float));
			stream.read(reinterpret_cast<char*>(v1), 3 * sizeof(float));
			stream.read(reinterpret_cast<char*>(v2), 3 * sizeof(float));
			stream.read(reinterpret_cast<char*>(v3), 3 * sizeof(float));
			stream.read(reinterpret_cast<char*>(&attributeByteCount), sizeof(uint16_t));

			glm::vec3 n = glm::make_vec3(normal);
			vertices.push_back({ glm::make_vec3(v1), n });
			vertices.push_back({ glm::make_vec3(v2), n });
			vertices.push_back({ glm::make_vec3(v3), n });
		}

		std::vector<unsigned int> indices(vertices.size());
		for (unsigned int i = 0; i < vertices.size(); ++i) indices[i] = i;

		std::cout << "Loaded STL model with " << vertices.size() << " vertices." << std::endl;
		return std::make_unique<Mesh>(vertices, indices);
	}

}