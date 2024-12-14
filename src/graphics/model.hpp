#pragma once

#include "mesh.hpp"
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











class Model {
 public:
    // id of model in scene
    std::string id;

	// list of meshes
	static std::vector<std::unique_ptr<Mesh>> meshes;

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

 private:
    void createTextureBuffers();

	VulkanDevice &vulkanDevice;
};

//}	// namespace lve
