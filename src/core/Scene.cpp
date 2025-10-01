#include "Scene.h"

Entity Scene::CreateEntity(const std::string& name) {
    std::string entityName = (tags.empty()) ? "[Scene]" : name;

    tags.emplace_back(TagComponent{ name });
    transforms.emplace_back(TransformComponent{});
    materials.emplace_back(MaterialComponent{});
    meshes.emplace_back(MeshComponent{});

    return tags.size() - 1;
}

void Scene::DestroyEntity(Entity entity) {
    if (entity >= tags.size()) return;
    tags[entity] = tags.back();
    transforms[entity] = transforms.back();
    materials[entity] = materials.back();
    meshes[entity] = std::move(meshes.back());

    tags.pop_back();
    transforms.pop_back();
    materials.pop_back();
    meshes.pop_back();
}