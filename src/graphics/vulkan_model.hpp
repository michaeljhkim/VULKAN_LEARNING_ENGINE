#pragma once

#include "vulkan_buffer.hpp"
#include "vulkan_device.hpp"

#include "mesh.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <vector>


#include "models/box.hpp"
#include "../physics/collisionmodel.h"
#include "../physics/rigidbody.h"
#include "../algorithms/bounds.h"
#include "../algorithms/states.hpp"

// model switches
#define DYNAMIC				(unsigned int)1 // 0b00000001
#define CONST_INSTANCES		(unsigned int)2 // 0b00000010
#define NO_TEX				(unsigned int)4	// 0b00000100

//namespace lve {

class VulkanModel {
 public:
	struct Builder {
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		
		// list of meshes
		std::vector<Mesh> meshes;
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

		void loadModel(const std::string &filepath);
		void loadModel_test(const std::string &filepath);
		void processNode(aiNode* node, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> loadTextures(aiMaterial* mat, aiTextureType type);
	};

	VulkanModel(VulkanDevice &device, const VulkanModel::Builder &builder);
	~VulkanModel();

	VulkanModel(const VulkanModel &) = delete;
	VulkanModel &operator=(const VulkanModel &) = delete;

	static std::unique_ptr<VulkanModel> createModelFromFile(
			VulkanDevice &device, const std::string &filepath);

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

 private:
	void createVertexBuffers(const std::vector<Vertex> &vertices);
	void createIndexBuffers(const std::vector<uint32_t> &indices);

	VulkanDevice &vulkanDevice;

	std::unique_ptr<VulkanBuffer> vertexBuffer;
	uint32_t vertexCount;

	bool hasIndexBuffer = false;
	std::unique_ptr<VulkanBuffer> indexBuffer;
	uint32_t indexCount;
};

//}	// namespace lve
