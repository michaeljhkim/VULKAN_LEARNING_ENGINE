#pragma once

//#include "mesh.hpp"
#include "shader_pipeline.hpp"
#include "rendering/texture.hpp"
#include "rendering/material.hpp"

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
struct SimplePushConstantData {
	Material material;
    bool noNormalMap;
    bool TexExists;
};

struct Mesh {
    BoundingRegion meshBoundingRegion;
    std::unique_ptr<CollisionMesh> collision;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // materialAssets
    bool noTextures;
    bool noNormalMap;
    std::vector<Texture> textures;
	//Material material;
    aiColor4D diffuse;
    aiColor4D specular;
};

struct Vertex {
    /*
        vertex values
    */
    glm::vec3 position{};   // position
    glm::vec3 color{};      // color
    glm::vec3 normal{};     // normal vector
    glm::vec2 texCoord{};   // texture coordinate (also known as UV or uv)
    glm::vec3 tangent;      // tangent vector

    // generate list of vertices
    static std::vector<Vertex> genList(float* vertices, int numVertices);
    // calculate tangent vectors for each face
    static void calcTanVectors(std::vector<Vertex>& list, std::vector<unsigned int>& indices);

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
            {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX},           // Binding for vertex data
            {1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE},      // Binding for instance data
            {2, sizeof(glm::mat3), VK_VERTEX_INPUT_RATE_INSTANCE}       // Binding for normalized instance data (e.g., orientation)
        };
        return bindingDescriptions;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
            // Per-vertex attributes
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},
            {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
            {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)},

            // Per-instance attributes (model matrix columns)
            //{4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0 * sizeof(glm::vec4)}, // First column
            //{5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 1 * sizeof(glm::vec4)}, // Second column
            {4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0 },
            {5, 1, VK_FORMAT_R32G32B32A32_SFLOAT,     sizeof(glm::vec4)},
            {6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 2 * sizeof(glm::vec4)}, // Third column
            {7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 3 * sizeof(glm::vec4)}, // Fourth column

            // Per-instance attributes (normalized matrix columns)
            //{ 8, 2, VK_FORMAT_R32G32B32_SFLOAT, 0 * sizeof(glm::vec3)},  // First column of glm::mat3
            //{ 9, 2, VK_FORMAT_R32G32B32_SFLOAT, 1 * sizeof(glm::vec3)},  // Second column of glm::mat3
            { 8, 2, VK_FORMAT_R32G32B32_SFLOAT, 0},
            { 9, 2, VK_FORMAT_R32G32B32_SFLOAT,     sizeof(glm::vec3)}, 
            {10, 2, VK_FORMAT_R32G32B32_SFLOAT, 2 * sizeof(glm::vec3)}   // Third column of glm::mat3
        };

        return attributeDescriptions;
    }

    bool operator==(const Vertex &other) const {
        return (position == other.position) && (color == other.color) && (normal == other.normal) && (texCoord == other.texCoord);
    }
};








class Model {
 public:
    // id of model in scene
    std::string id;
	
	// list of meshes
	static std::vector<std::unique_ptr<Mesh>> meshes;

	//std::vector<Vertex> vertices{};
	//std::vector<uint32_t> indices{};
    std::vector<Vertex> combinedVertices;
    std::vector<uint32_t> combinedIndices;
    //std::vector<stbi_uc> combinedTextures;
    std::vector<Texture> combinedTextures;
	std::vector<uint32_t> vertexOffsets;
	std::vector<uint32_t> indexOffsets;

	std::vector<std::unique_ptr<VkDrawIndexedIndirectCommand>> indirectCommands;

	std::string directory;					// directory containing object file
	std::vector<Texture> textures_loaded;	// list of loaded textures
	unsigned int switches;					// combination of switches above

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

	void loadModel(const std::string filepath);
	void processNode(aiNode* node, const aiScene* scene);
	std::unique_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<stbi_uc*> Model::loadTexturesAsPixels(aiMaterial* mat, aiTextureType type);
	std::vector<Texture> Model::loadTextures(aiMaterial* mat, aiTextureType type);


	// list of bounding regions (1 for each mesh)
	std::vector<BoundingRegion> boundingRegions;

	static std::unique_ptr<Model> createModelFromFile(VulkanDevice &device, const std::string &filepath);

	void bind(VkCommandBuffer commandBuffer, VkBuffer instanceBuffer, VkBuffer normalizedInstanceBuffer);
	void draw(VkCommandBuffer commandBuffer, VkBuffer indirectBuffer, uint32_t instanceCount);

 private:
	void createVertexBuffers();
	void createIndexBuffers();
    void createTextureBuffers();

	VulkanDevice &vulkanDevice;

	std::unique_ptr<VulkanBuffer> vertexBuffer;
	uint32_t vertexCount;

  	bool hasIndexBuffer = false;
	std::unique_ptr<VulkanBuffer> indexBuffer;
	uint32_t indexCount;
};

//}	// namespace lve
