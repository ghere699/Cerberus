#include "ObjLoader.h"
#include <iostream>
#include <vector>
#include <map>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace Cerberus {

    std::unique_ptr<Mesh> ProcessTinyObj(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes);

    std::unique_ptr<Mesh> ObjLoader::LoadModel(const std::string& path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
            std::cerr << "TinyObjLoader Error: " << err << std::endl;
            return nullptr;
        }

        if (!warn.empty()) {
            std::cout << "TinyObjLoader Warning: " << warn << std::endl;
        }

        return ProcessTinyObj(attrib, shapes);
    }

    std::unique_ptr<Mesh> ObjLoader::LoadModelFromMemory(const char* data, size_t size) {
        std::string content(data, size);
        std::stringstream data_stream(content);
        
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &data_stream)) {
            std::cerr << "TinyObjLoader Error (from memory): " << err << std::endl;
            return nullptr;
        }

        if (!warn.empty()) {
            std::cout << "TinyObjLoader Warning (from memory): " << warn << std::endl;
        }

        return ProcessTinyObj(attrib, shapes);
    }
    
    // Helper function to convert tinyobj data into our Mesh format
    std::unique_ptr<Mesh> ProcessTinyObj(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::map<std::tuple<int, int, int>, unsigned int> vertexCache;

        // Loop over shapes
        for (const auto& shape : shapes) {
            // Loop over faces(triangles)
            size_t index_offset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                int fv = shape.mesh.num_face_vertices[f];
                
                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                    auto vertex_tuple = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);

                    if (vertexCache.count(vertex_tuple)) {
                        indices.push_back(vertexCache[vertex_tuple]);
                    } else {
                        Vertex vertex;

                        // Position
                        vertex.Position = {
                            attrib.vertices[3 * idx.vertex_index + 0],
                            attrib.vertices[3 * idx.vertex_index + 1],
                            attrib.vertices[3 * idx.vertex_index + 2]
                        };

                        // Normal (check if it exists)
                        if (idx.normal_index >= 0) {
                            vertex.Normal = {
                                attrib.normals[3 * idx.normal_index + 0],
                                attrib.normals[3 * idx.normal_index + 1],
                                attrib.normals[3 * idx.normal_index + 2]
                            };
                        }

                        // Texture Coordinate (check if it exists)
                        if (idx.texcoord_index >= 0) {
                            vertex.TexCoords = {
                                attrib.texcoords[2 * idx.texcoord_index + 0],
                                1.0f - attrib.texcoords[2 * idx.texcoord_index + 1] // Flip Y-coord for OpenGL
                            };
                        }

                        vertices.push_back(vertex);
                        unsigned int newIndex = vertices.size() - 1;
                        indices.push_back(newIndex);
                        vertexCache[vertex_tuple] = newIndex;
                    }
                }
                index_offset += fv;
            }
        }
        
        std::cout << "Loaded OBJ model with " << vertices.size() << " vertices" << std::endl;
        return std::make_unique<Mesh>(vertices, indices);
    }
}