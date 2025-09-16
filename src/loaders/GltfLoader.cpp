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

        if (!warn.empty()) {
            std::cout << "GLTF Loader Warning: " << warn << std::endl;
        }
        if (!err.empty()) {
            std::cerr << "GLTF Loader Error: " << err << std::endl;
            return nullptr;
        }
        if (!ret) {
            std::cerr << "Failed to load GLTF file: " << path << std::endl;
            return nullptr;
        }

        if (model.meshes.empty()) {
            std::cerr << "GLTF file does not contain any meshes." << std::endl;
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
            const auto& normBuffer = model.buffers[normBufferView.buffer];
            const float* normals = reinterpret_cast<const float*>(&normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset]);

            vertices.reserve(vertices.size() + posAccessor.count);

            for (size_t i = 0; i < posAccessor.count; ++i) {
                Vertex vert;
                vert.Position = glm::make_vec3(positions + (i * 3));
                vert.Normal = glm::make_vec3(normals + (i * 3));
                vertices.push_back(vert);
            }

            const auto& indicesAccessor = model.accessors[primitive.indices];
            const auto& indicesBufferView = model.bufferViews[indicesAccessor.bufferView];
            const auto& indicesBuffer = model.buffers[indicesBufferView.buffer];
            const void* indicesData = &indicesBuffer.data[indicesBufferView.byteOffset + indicesAccessor.byteOffset];

            indices.reserve(indices.size() + indicesAccessor.count);

            switch (indicesAccessor.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                const auto* buf = static_cast<const uint8_t*>(indicesData);
                for (size_t i = 0; i < indicesAccessor.count; i++) indices.push_back(buf[i]);
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                const auto* buf = static_cast<const uint16_t*>(indicesData);
                for (size_t i = 0; i < indicesAccessor.count; i++) indices.push_back(buf[i]);
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                const auto* buf = static_cast<const uint32_t*>(indicesData);
                for (size_t i = 0; i < indicesAccessor.count; i++) indices.push_back(buf[i]);
                break;
            }
            default:
                std::cerr << "Unsupported index type." << std::endl;
                return nullptr;
            }
        }

        std::cout << "Loaded GLB model: " << path << " with " << vertices.size() << " vertices and " << indices.size() << " indices." << std::endl;
        return std::make_unique<Mesh>(vertices, indices);
    }
}