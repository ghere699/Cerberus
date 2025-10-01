#pragma once

#include "../graphics/Mesh.h"
#include <string>
#include <vector>
#include <memory>

struct TagComponent {
    std::string name = "Object";
    std::string filePath = "Internal";
};

struct TransformComponent {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 initialPosition = glm::vec3(0.0f); 
};

struct MaterialComponent {
    glm::vec3 color = glm::vec3(1.0f);
    float shininess = 32.0f;
    float specularStrength = 1.0f;
    float ambientStrength = 0.1f;
};

struct MeshComponent {
    std::unique_ptr<Cerberus::Mesh> mesh = nullptr;
};

using Entity = size_t;

class Scene {
public:
    Entity CreateEntity(const std::string& name = "New Object");
    void DestroyEntity(Entity entity);

    template<typename T>
    T& GetComponent(Entity entity);

    std::vector<TagComponent> tags;
    std::vector<TransformComponent> transforms;
    std::vector<MaterialComponent> materials;
    std::vector<MeshComponent> meshes;

    Entity GetEntityCount() const { return tags.size(); }
};

template<typename T>
T& Scene::GetComponent(Entity entity) {
    // this is a placeholder bc c++ doesn't allow switching on its type
    if constexpr (std::is_same_v<T, TagComponent>) return tags[entity];
    if constexpr (std::is_same_v<T, TransformComponent>) return transforms[entity];
    if constexpr (std::is_same_v<T, MaterialComponent>) return materials[entity];
    if constexpr (std::is_same_v<T, MeshComponent>) return meshes[entity];
}