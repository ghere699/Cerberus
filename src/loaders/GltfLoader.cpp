#include "GltfLoader.h"
#include <iostream>
#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#include <gtc/type_ptr.hpp>

namespace Cerberus {

	std::unique_ptr<Mesh> GltfLoader::LoadModel(const std::string& path) {
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);

		if (!warn.empty()) std::cout << "GLTF Loader Warning: " << warn << std::endl;
		if (!err.empty()) {
			std::cerr << "GLTF Loader Error: " << err << std::endl;
			return nullptr;
		}
		if (!ret) {
			std::cerr << "Failed to load GLTF file: " << path << std::endl;
			return nullptr;
		}

		std::cout << "Successfully loaded GLB file: " << path << std::endl;
		return ProcessModel(model);
	}

	//Oh, I wanna stay, wanna walk out the door Oh no, right now, baby, I’m torn
	std::unique_ptr<Mesh> GltfLoader::LoadModelFromMemory(const unsigned char* data, size_t size) {
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadBinaryFromMemory(&model, &err, &warn, data, size);

		if (!warn.empty()) std::cout << "GLTF Loader Warning: " << warn << std::endl;
		if (!err.empty()) {
			std::cerr << "GLTF Loader Error: " << err << std::endl;
			return nullptr;
		}
		if (!ret) {
			std::cerr << "Failed to load GLTF from memory." << std::endl;
			return nullptr;
		}

		std::cout << "Loaded GLB from memory." << std::endl;
		return ProcessModel(model);
	}

	std::unique_ptr<Mesh> GltfLoader::ProcessModel(const tinygltf::Model& model) {
		if (model.meshes.empty()) {
			std::cerr << "GLTF model does not contain any meshes." << std::endl;
			return nullptr;
		}

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		const auto& mesh = model.meshes[0];
		for (const auto& primitive : mesh.primitives) {
			const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
			const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
			const auto& posBuffer = model.buffers[posBufferView.buffer];
			const float* positions = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

			const auto& normAccessor = model.accessors[primitive.attributes.at("NORMAL")];
			const auto& normBufferView = model.bufferViews[normAccessor.bufferView];
			const auto& normBuffer = model.buffers[normAccessor.bufferView];
			const float* normals = reinterpret_cast<const float*>(&normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset]);

			const float* texCoords = nullptr;
			if (primitive.attributes.count("TEXCOORD_0")) {
				const auto& uvAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
				const auto& uvBufferView = model.bufferViews[uvAccessor.bufferView];
				const auto& uvBuffer = model.buffers[uvBufferView.buffer];
				texCoords = reinterpret_cast<const float*>(&uvBuffer.data[uvBufferView.byteOffset + uvAccessor.byteOffset]);
			}

			vertices.reserve(vertices.size() + posAccessor.count);

			for (size_t i = 0; i < posAccessor.count; ++i) {
				Vertex vert;
				vert.Position = glm::make_vec3(positions + (i * 3));
				vert.Normal = glm::make_vec3(normals + (i * 3));
				if (texCoords) {
					vert.TexCoords = glm::make_vec2(texCoords + (i * 2));
				}
				else {
					vert.TexCoords = glm::vec2(0.0f);
				}
				vertices.push_back(vert);
			}

			const auto& indicesAccessor = model.accessors[primitive.indices];
			const auto& indicesBufferView = model.bufferViews[indicesAccessor.bufferView];
			const auto& indicesBuffer = model.buffers[indicesBufferView.buffer];
			const void* indicesData = &indicesBuffer.data[indicesBufferView.byteOffset + indicesAccessor.byteOffset];

			indices.reserve(indices.size() + indicesAccessor.count);

			switch (indicesAccessor.componentType) {
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
				for (size_t i = 0; i < indicesAccessor.count; i++) indices.push_back(static_cast<const uint8_t*>(indicesData)[i]);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				for (size_t i = 0; i < indicesAccessor.count; i++) indices.push_back(static_cast<const uint16_t*>(indicesData)[i]);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
				for (size_t i = 0; i < indicesAccessor.count; i++) indices.push_back(static_cast<const uint32_t*>(indicesData)[i]);
				break;
			default:
				std::cerr << "Unsupported idx type." << std::endl;
				return nullptr;
			}
		}

		std::cout << "Processed GLTF model with " << vertices.size() << " vertices and " << indices.size() << " indices." << std::endl;
		return std::make_unique<Mesh>(vertices, indices);
	}
}