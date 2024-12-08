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
#include "../physics/collisionmodel.hpp"
#include "../physics/rigidbody.hpp"
#include "../algorithms/bounds.hpp"
#include "../algorithms/states.hpp"
//#include "../scene.hpp"


// model switches
#define DYNAMIC				(unsigned int)1 // 0b00000001
#define CONST_INSTANCES		(unsigned int)2 // 0b00000010
#define NO_TEX				(unsigned int)4	// 0b00000100

//namespace lve {

class Model {
 public:
	
	// list of meshes
	static std::vector<std::unique_ptr<Mesh>> meshes;

	//std::vector<Vertex> vertices{};
	//std::vector<uint32_t> indices{};
    std::vector<Vertex> combinedVertices;
    std::vector<uint32_t> combinedIndices;
	std::vector<uint32_t> vertexOffsets;
	std::vector<uint32_t> indexOffsets;

	std::vector<std::unique_ptr<VkDrawIndexedIndirectCommand>> indirectCommands;

	//void render(Shader shader, float dt, Scene* scene);


    /*
        constructor
    */

    // initialize with parameters
    Model(VulkanDevice &vulkanDevice, unsigned int flags = 0);

	~Model();

	Model(const Model &) = delete;
	Model &operator=(const Model &) = delete;


    /*
        process functions
    */

    // initialize method (to be overriden)
    virtual void init();

	std::string directory;					// directory containing object file
	std::vector<Texture> textures_loaded;	// list of loaded textures
	unsigned int switches;					// combination of switches above

	void loadModel(const std::string filepath);
	void processNode(aiNode* node, const aiScene* scene);
	std::unique_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadTextures(aiMaterial* mat, aiTextureType type);


	// list of bounding regions (1 for each mesh)
	std::vector<BoundingRegion> boundingRegions;

	static std::unique_ptr<Model> createModelFromFile(VulkanDevice &device, const std::string &filepath);

	void bind(VkCommandBuffer commandBuffer, VkBuffer instanceBuffer, VkBuffer normalizedInstanceBuffer);
	void draw(VkCommandBuffer commandBuffer, VkBuffer indirectBuffer, uint32_t instanceCount);

 private:
	void createVertexBuffers();
	void createIndexBuffers();

	VulkanDevice &vulkanDevice;

	std::unique_ptr<VulkanBuffer> vertexBuffer;
	uint32_t vertexCount;

  	bool hasIndexBuffer = false;
	std::unique_ptr<VulkanBuffer> indexBuffer;
	uint32_t indexCount;
};

//}	// namespace lve
