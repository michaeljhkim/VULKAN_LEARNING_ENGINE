#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "../objects/model.h"

class Sphere : public Model {
public:
    Sphere(unsigned int maxNumInstances)
        : Model("sphere", maxNumInstances, NO_TEX | DYNAMIC) {}

    void init() {
        loadModel("assets/models/sphere/scene.gltf");
    }
};

#endif