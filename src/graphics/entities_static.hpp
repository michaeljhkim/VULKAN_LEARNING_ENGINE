#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "../entity.hpp"

class StaticEntity : public Entity {
public:
    StaticEntity(unsigned int maxNumInstances)
        : Entity("sphere", maxNumInstances, NO_TEX | DYNAMIC) {}

    void init() {
        loadModel("assets/models/sphere/scene.gltf");
    }
};

#endif