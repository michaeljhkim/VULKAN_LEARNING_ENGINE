#pragma once

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>
#include <vector>
#include <span>
#include <utility>

#include "vulkan_buffer.hpp"
#include "vulkan_device.hpp"
#include "vulkan_utils.hpp"


// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "model.hpp"
#include "shader_pipeline.hpp"

#include "models/box.hpp"
#include "../physics/collisionmodel.hpp"
#include "../physics/rigidbody.hpp"
#include "../algorithms/bounds.hpp"
#include "../algorithms/states.hpp"
//#include "../scene.hpp"



// model switches
#define DYNAMIC				(unsigned int)1 // 0b00000001
#define CONST_INSTANCES		(unsigned int)2 // 0b00000010
#define NO_TEX				(unsigned int)4	// 0b00000100


typedef enum modelTypeFlag {
    STATIC_INSTANCES = 1,			  // Model and instances are fully static, no changing after loading - consider combining with other models in a scene
    DYNAMIC_FIXED_INSTANCES = 2, 	  // Dynamic behavior but with a fixed number of instances
    DYNAMIC_RESIZABLE_INSTANCES = 3,  // Dynamic behavior with a variable (resizable) number of instances
};


struct PointLightComponent {
	float lightIntensity = 1.0f;
};

class Entity {
 public:
	using id_t = unsigned int;
	using Map = std::unordered_map<id_t, Entity>;

    // id of model in scene
    std::string model_name;

	/*
	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};
    std::vector<Vertex> combinedVertices;
    std::vector<uint32_t> combinedIndices;
    std::vector<stbi_uc> combinedTextures;
    std::vector<Texture> combinedTextures;		// Probably should handle texture stuff in Mesh - consider later
	std::vector<uint32_t> vertexOffsets;
	std::vector<uint32_t> indexOffsets;
	std::vector<std::unique_ptr<VkDrawIndexedIndirectCommand>> indirectCommands;
	*/

	// initialize with parameters
	//Entity(VulkanDevice &device, modelTypeFlag model_type);

	// Only for unique models
	static Entity createEntity(VulkanDevice &device, modelTypeFlag model_type) {
		static id_t currentId = 0;
		return Entity{device, currentId++};
	}

	//void attachModel(VulkanDevice &device, std::string name, unsigned int maxNumInstances, unsigned int flags = 0);
	void attachModel(std::string name, std::string filePath, unsigned int maxNumInstances, unsigned int flags = 0, modelTypeFlag model_type);

	static Entity makePointLight(
			float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

	Entity(const Entity &) = delete;
	Entity &operator=(const Entity &) = delete;
	Entity(Entity &&) = default;
	Entity &operator=(Entity &&) = default;

	id_t getId() { return id; }

	glm::vec3 color{};
	TransformComponent transform{};

	// Optional pointer components
	std::unique_ptr<Model> model{};
	std::unique_ptr<PointLightComponent> pointLight = nullptr;
	
	// pointer to the collision model
	std::unique_ptr<CollisionModel> collision;
	// list of bounding regions (1 for each mesh)
	std::vector<BoundingRegion> boundingRegions;
    // list of instances
    std::vector<RigidBody*> instances;
	// list of indexed indirect Commands (1 for each mesh)
	std::vector<std::unique_ptr<VkDrawIndexedIndirectCommand>> indirectCommands;

    // maximum number of instances and current number of instances
    unsigned int maxNumInstances;
    unsigned int currentNumInstances;

    // combination of switches above
    unsigned int switches;
	void initInstances();
    void removeInstance(unsigned int idx);
    void removeInstance(std::string instanceId);
    unsigned int getIdx(std::string id);
	void enableCollisionModel();

	RigidBody* generateInstance(glm::vec3 size, float mass, glm::vec3 pos, glm::vec3 rot);
	void render(ShaderPipline& shader_pipeline, float dt, VkCommandBuffer& commandBuffer);

 private:
	void createVertexBuffers();
	void createIndexBuffers();
	Entity(VulkanDevice &device, id_t objId ) : vulkanDevice{device}, id{objId} {}

	id_t id;
	modelTypeFlag model_type;

	VulkanDevice &vulkanDevice;

	std::unique_ptr<VulkanBuffer> instanceBuffer;
	std::unique_ptr<VulkanBuffer> normalInstanceBuffer;
	//std::unique_ptr<VulkanBuffer> indirectCommandBuffer;
	//uint32_t instanceCount;
	//uint32_t normalInstanceCount;
};
