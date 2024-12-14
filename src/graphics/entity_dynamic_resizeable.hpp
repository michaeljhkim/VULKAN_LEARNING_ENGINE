#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "../entity.hpp"

class ResizeableDynamicEntity : public Entity {
public:
    ResizeableDynamicEntity(VulkanDevice &device, modelTypeFlag model_type) : Entity(device, model_type) {
        for (const auto& current_mesh : model->meshes) {
            createVertexBuffers(current_mesh->vertices);
        }
    }

    void init() {
        loadModel("assets/models/sphere/scene.gltf");
    }
};

#endif