#ifndef COLLISIONMODEL_H
#define COLLISIONMODEL_H

#include "collisionmesh.hpp"

#include <vector>

// forward declaration
class Model;

class CollisionModel {
public:
	Model* model;

	std::vector<CollisionMesh> meshes;

	CollisionModel(Model* model);
};

#endif