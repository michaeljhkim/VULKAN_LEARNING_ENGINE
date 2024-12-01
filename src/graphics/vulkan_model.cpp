#include "vulkan_model.hpp"
#include "vulkan_utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


// std
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <iostream>
#include <limits>
#include <span>


#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace std {
template <>
struct hash<Vertex> {
	size_t operator()(Vertex const &vertex) const {
		size_t seed = 0;
		hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.texCoord);
		return seed;
	}
};
}	// namespace std

//namespace lve {

VulkanModel::VulkanModel(VulkanDevice &device, const VulkanModel::Builder &builder) : vulkanDevice{device} {
	createVertexBuffers(builder.vertices);
	createIndexBuffers(builder.indices);
}

VulkanModel::~VulkanModel() {}

std::unique_ptr<VulkanModel> VulkanModel::createModelFromFile(VulkanDevice &device, const std::string &filepath) {
	Builder builder{};
	builder.loadModel(ENGINE_DIR + filepath);

	//THIS IS THE LINE WHERE THE VERTEX AND INDEX BUFFERS ARE MADE
	return std::make_unique<VulkanModel>(device, builder);
}

void VulkanModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
	vertexCount = static_cast<uint32_t>(vertices.size());
	assert(vertexCount >= 3 && "Vertex count must be at least 3");
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
	uint32_t vertexSize = sizeof(vertices[0]);

	VulkanBuffer stagingBuffer{
			vulkanDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	stagingBuffer.map();
	stagingBuffer.writeToBuffer((void *)vertices.data());

	vertexBuffer = std::make_unique<VulkanBuffer>(
			vulkanDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vulkanDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
}

void VulkanModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
	indexCount = static_cast<uint32_t>(indices.size());
	hasIndexBuffer = indexCount > 0;

	if (!hasIndexBuffer) {
		return;
	}

	VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
	uint32_t indexSize = sizeof(indices[0]);

	VulkanBuffer stagingBuffer{
			vulkanDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	};

	stagingBuffer.map();
	stagingBuffer.writeToBuffer((void *)indices.data());

	indexBuffer = std::make_unique<VulkanBuffer>(
			vulkanDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vulkanDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}

void VulkanModel::draw(VkCommandBuffer commandBuffer) {
	if (hasIndexBuffer) {
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	} else {
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}
}

void VulkanModel::bind(VkCommandBuffer commandBuffer) {
	VkBuffer buffers[] = {vertexBuffer->getBuffer()};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	if (hasIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}
}

std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions() {
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

	attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
	attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
	attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
	attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)});

	return attributeDescriptions;
}

void VulkanModel::Builder::loadModel(const std::string &filepath) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	vertices.clear();
	indices.clear();

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	for (const auto &shape : shapes) {
		for (const auto &index : shape.mesh.indices) {
			Vertex vertex{};

			if (index.vertex_index >= 0) {
				vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
				};

				vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2],
				};
			}

			if (index.normal_index >= 0) {
				vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2],
				};
			}

			if (index.texcoord_index >= 0) {
				vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1],
				};
			}

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);
		}
	}
}







// load model from path
void VulkanModel::Builder::loadModel_test(const std::string& filepath) {
    Assimp::Importer import;

    // Load the model with ASSIMP
    const aiScene* scene = import.ReadFile(filepath, 
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    // Check for errors
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        std::cerr << "Failed to load model: " << filepath << "\n" << import.GetErrorString() << std::endl;
        return;
    }

    // Parse directory from filepath
    directory = filepath.substr(0, filepath.find_last_of('/'));

    // Process the root node
    processNode(scene->mRootNode, scene);
}

void VulkanModel::Builder::processNode(aiNode* node, const aiScene* scene) {
    // Process all meshes in the current node
    for (unsigned int meshIndex : std::span(node->mMeshes, node->mNumMeshes)) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        meshes.push_back(processMesh(mesh, scene));
        boundingRegions.push_back(meshes.back().br); // Push bounding region
    }

    // Recursively process child nodes
    for (aiNode* child : std::span(node->mChildren, node->mNumChildren)) {
        processNode(child, scene);
    }
}


// process mesh in object file
Mesh VulkanModel::Builder::processMesh(aiMesh* mesh, const aiScene* scene) {
    // Reserve space to minimize reallocations
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    //vertices.reserve(mesh->mNumVertices);
    //indices.reserve(3 * mesh->mNumFaces);
    std::vector<Texture> textures;

    // Setup bounding region and initial values
    BoundingRegion br(BoundTypes::SPHERE);
    glm::vec3 min(std::numeric_limits<float>::max());
    glm::vec3 max(std::numeric_limits<float>::lowest());

    // Process vertices
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex{};

        glm::vec3 position(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        // Update min and max bounds - goal is to find the absolutes of the model to generate Bounding Region
        min = glm::min(min, position);
        max = glm::max(max, position);

        // Populate vertex attributes
        vertex.position = position;
        vertex.normal = glm::vec3(
            mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z
        );
        vertex.texCoord = mesh->mTextureCoords[0]
            ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
            : glm::vec2(0.0f);
        vertex.tangent = glm::vec3(
            mesh->mTangents[i].x,
            mesh->mTangents[i].y,
            mesh->mTangents[i].z
        );

		//We do this so we only account for unique vectors. Less processing power needed
		if (uniqueVertices.count(vertex) == 0) {
			uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
			vertices.push_back(vertex);
		}
		indices.push_back(uniqueVertices[vertex]);
    }

    // Compute bounding region
    br.center = (min + max) / 2.0f;
    br.ogCenter = br.center;
    br.radius = 0.0f;
    br.collisionMesh = nullptr;

    for (const auto& vertex : vertices) {
        br.radius = glm::max(br.radius, glm::length(vertex.position - br.center));
    }
    br.ogRadius = br.radius;


    // Process material
    Mesh ret;
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        if (States::isActive<unsigned int>(&switches, NO_TEX)) {
            // Use material colors
            aiColor4D diff(1.0f), spec(1.0f);
            aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diff);
            aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &spec);
            ret = Mesh(br, diff, spec);
        } else {
            // Load textures
            auto loadAndInsert = [&](aiTextureType type) {
                auto maps = loadTextures(material, type);
                textures.insert(textures.end(), maps.begin(), maps.end());
            };
            loadAndInsert(aiTextureType_DIFFUSE);
            loadAndInsert(aiTextureType_SPECULAR);
            loadAndInsert(aiTextureType_NORMALS); // Use HEIGHT for .obj files if needed
            ret = Mesh(br, textures);
        }
    }

    // Load vertex and index data
    ret.loadData(vertices, indices);
    return ret;
}



// load list of textures
std::vector<Texture> VulkanModel::Builder::loadTextures(aiMaterial* mat, aiTextureType type) {
    std::vector<Texture> textures;
    std::unordered_map<std::string, Texture> textureMap;

    // Build a map of already loaded textures for fast lookup
    for (const auto& loadedTex : textures_loaded) {
        textureMap.emplace(loadedTex.path, loadedTex);
    }

    // Load new textures
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string texturePath = str.C_Str();

        // Check if texture is already loaded
        if (auto it = textureMap.find(texturePath); it != textureMap.end()) {
            textures.push_back(it->second);
        } 
		else {
            // Texture not loaded yet
            Texture tex(directory, texturePath, type);
            tex.load(false);
            textures.push_back(tex);
            textures_loaded.push_back(tex); // Add to loaded textures
            textureMap.emplace(texturePath, tex); // Update the map
        }
    }

    return textures;
}



//}	// namespace lve
