#pragma once

#include "mesh.hpp"

#include "vulkan_buffer.hpp"
#include "vulkan_device.hpp"


// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <vector>
#include <span>
#include <utility>


#include "models/box.hpp"
#include "../physics/collisionmodel.h"
#include "../physics/rigidbody.h"
#include "../algorithms/bounds.h"
#include "../algorithms/states.hpp"
#include "../scene.hpp"


// model switches
#define DYNAMIC				(unsigned int)1 // 0b00000001
#define CONST_INSTANCES		(unsigned int)2 // 0b00000010
#define NO_TEX				(unsigned int)4	// 0b00000100

//namespace lve {

class Model {
 public:
    // id of model in scene
    std::string id;
	
	// list of meshes
	static std::vector<std::unique_ptr<Mesh>> meshes;

    // list of instances
    std::vector<RigidBody*> instances;

    // maximum number of instances
    unsigned int maxNumInstances;
    // current number of instances
    unsigned int currentNumInstances;

    // combination of switches above
    unsigned int switches;
	void initInstances();

	RigidBody* generateInstance(glm::vec3 size, float mass, glm::vec3 pos, glm::vec3 rot);
	void render(Shader shader, float dt, Scene* scene);


    /*
        constructor
    */

    // initialize with parameters
    Model(VulkanDevice &device, std::string id, unsigned int maxNumInstances, unsigned int flags = 0);

	~Model();

	Model(const Model &) = delete;
	Model &operator=(const Model &) = delete;


    /*
        process functions
    */

    // initialize method (to be overriden)
    virtual void init();

	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};
	
	// pointer to the collision model
	CollisionModel* collision;
	// list of bounding regions (1 for each mesh)
	std::vector<BoundingRegion> boundingRegions;

	// directory containing object file
	std::string directory;
	// list of loaded textures
	std::vector<Texture> textures_loaded;
	// combination of switches above
	unsigned int switches;

	void loadModel(const std::string filepath);
	void processNode(aiNode* node, const aiScene* scene);
	std::unique_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadTextures(aiMaterial* mat, aiTextureType type);



	static std::unique_ptr<Model> createModelFromFile(
			VulkanDevice &device, const std::string &filepath);


 private:
	VulkanDevice &vulkanDevice;

	std::unique_ptr<VulkanBuffer> instanceBuffer;
	//uint32_t instanceCount;
	std::unique_ptr<VulkanBuffer> normalInstanceBuffer;
	//uint32_t normalInstanceCount;
};

//}	// namespace lve
