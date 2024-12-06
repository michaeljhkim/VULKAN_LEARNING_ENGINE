#include <glm/glm.hpp>

#include "bounds.hpp"
#include "../physics/collisionmesh.hpp"
#include "../physics/rigidbody.hpp"

class Ray {
public:
	glm::vec3 origin;
	glm::vec3 dir;
	glm::vec3 invdir;

	Ray(glm::vec3 origin, glm::vec3 dir);

	bool intersectsBoundingRegion(BoundingRegion br, float &tmin, float &tmax);
	bool intersectsMesh(CollisionMesh* mesh, RigidBody* rb, float &t);
};
