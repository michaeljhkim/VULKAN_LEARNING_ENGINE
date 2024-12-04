#include "model.hpp"
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

#include "../scene.hpp"


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


Model::~Model() {}
/*
// free up memory
void Model::cleanup() {
    // free all instances
    for (unsigned int i = 0, len = instances.size(); i < len; i++) {
        if (instances[i]) {
            //free(instances[i]);
        }
    }
    instances.clear();

    // cleanup each mesh
    for (unsigned int i = 0, len = instances.size(); i < len; i++) {
        meshes[i].cleanup();
    }

    // free up memory for position and size VBOs
    modelVBO.cleanup();
    normalModelVBO.cleanup();
}
*/


/*
    constructor
*/

// initialize with parameters
Model::Model(VulkanDevice &device, std::string id, unsigned int maxNumInstances, unsigned int flags)
    : vulkanDevice{device}, id(id), switches(flags), currentNumInstances(0), maxNumInstances(maxNumInstances), instances(maxNumInstances), collision(nullptr) {}

/*
    process functions
*/

// initialize method (to be overriden)
void Model::init() {}

// render instance(s)
void Model::render(Shader shader, float dt, Scene* scene) {
    if (!States::isActive(&switches, CONST_INSTANCES)) {
        // dynamic instances - update VBO data

        // create list of each
        std::vector<glm::mat4> models(currentNumInstances);
        std::vector<glm::mat3> normalModels(currentNumInstances);

        // determine if instances are moving
        bool doUpdate = States::isActive(&switches, DYNAMIC);

        // iterate through each instance
        for (int i = 0; i < currentNumInstances; i++) {
            if (doUpdate) {
                // update Rigid Body
                instances[i]->update(dt);
                // activate moved switch
                States::activate(&instances[i]->state, INSTANCE_MOVED);
            }
            else {
                // deactivate moved switch
                States::deactivate(&instances[i]->state, INSTANCE_MOVED);
            }

            // add updated matrices
            models[i] = instances[i]->model;
            normalModels[i] = instances[i]->normalModel;
        }

        if (currentNumInstances) {
            // set transformation data
            instanceBuffer->map();
            instanceBuffer->writeToBuffer((void*) (sizeof(glm::mat4) * currentNumInstances) );

            normalInstanceBuffer->map();
            normalInstanceBuffer->writeToBuffer((void*) (sizeof(glm::mat3) * currentNumInstances) );

            // set attribute pointers for each mesh
            for (unsigned int i = 0; i < meshes.size(); i++) {
                //meshes[i]->bind(commandBuffer, instanceBuffer, normalInstanceBuffer)
                //meshes[i]->draw(commandBuffer, currentNumInstances)
            }
        }
    }

	/*
    // set shininess
    shader.setFloat("material.shininess", 0.5f);

    // render each mesh
    for (unsigned int i = 0, noMeshes = meshes.size(); i < noMeshes; i++) {
        meshes[i].render(shader, currentNumInstances);
    }
	*/
}
// generate instance with parameters
RigidBody* Model::generateInstance(glm::vec3 size, float mass, glm::vec3 pos, glm::vec3 rot) {
    // all slots filled
    if (currentNumInstances >= maxNumInstances) {
        return nullptr;
    }

    // instantiate new instance
    instances[currentNumInstances] = new RigidBody(id, size, mass, pos, rot);
    return instances[currentNumInstances++];
}


//Later, MAYBE make it so that it only binds and draws if the instances exists IDK I gotta do some more digging
void Model::initInstances() {
    // default values
    std::unique_ptr<glm::mat4> modelData = nullptr;
    std::unique_ptr<glm::mat3> normalModelData = nullptr;

    std::vector<glm::mat4> models(currentNumInstances);
    std::vector<glm::mat3> normalModels(currentNumInstances);

    if (States::isActive(&switches, CONST_INSTANCES)) {
        // instances won't change, set data pointers

        for (unsigned int i = 0; i < currentNumInstances; i++) {
            models[i] = instances[i]->model;
            normalModels[i] = instances[i]->normalModel;
        }

        if (currentNumInstances) {
            modelData = std::make_unique<glm::mat4>(&models[0]);
            normalModelData = std::make_unique<glm::mat3>(&normalModels[0]);
        }
    }

    uint32_t max_instances = static_cast<uint32_t>(maxNumInstances);
	assert(max_instances >= 1 && "Max instance count must be at least 1");

	uint32_t dataSize = sizeof(modelData);
	instanceBuffer = std::make_unique<VulkanBuffer>(
			vulkanDevice,
			dataSize,
			max_instances,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	uint32_t normalDataSize = sizeof(normalModelData);
    normalInstanceBuffer = std::make_unique<VulkanBuffer>(
			vulkanDevice,
			normalDataSize,
			max_instances,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}




// load model from path
void Model::loadModel(const std::string filepath) {
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

void Model::processNode(aiNode* node, const aiScene* scene) {
    // Process all meshes in the current node
    for (unsigned int meshIndex : std::span(node->mMeshes, node->mNumMeshes)) {
        aiMesh* mesh = scene->mMeshes[meshIndex];

		//we add the mesh to the meshes list
        meshes.push_back(processMesh(mesh, scene));
		//meshes.back() is the mesh we just processed the line above
        boundingRegions.push_back(meshes.back()->br); // Push bounding region
    }

    // Recursively process child nodes
    for (aiNode* child : std::span(node->mChildren, node->mNumChildren)) {
        processNode(child, scene);
    }
}


// process mesh in object file
std::unique_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    // Reserve space to minimize reallocations
    std::vector<Vertex> vertices(mesh->mNumVertices);
    std::vector<uint32_t> indices(3 * mesh->mNumFaces);

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
    std::unique_ptr<Mesh> ret;
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        if (States::isActive<unsigned int>(&switches, NO_TEX)) {
            // Use material colors
            aiColor4D diff(1.0f), spec(1.0f);
            aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diff);
            aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &spec);
            ret = std::make_unique<Mesh>(vulkanDevice, br, diff, spec);
        } else {
            // Load textures
            auto loadAndInsert = [&](aiTextureType type) {
                auto maps = loadTextures(material, type);
                textures.insert(textures.end(), maps.begin(), maps.end());
            };
            loadAndInsert(aiTextureType_DIFFUSE);
            loadAndInsert(aiTextureType_SPECULAR);
            loadAndInsert(aiTextureType_NORMALS); // Use HEIGHT for .obj files if needed
            ret = std::make_unique<Mesh>(vulkanDevice, br, textures);
        }
    }
    // Load vertex and index data
    ret->loadData(vertices, indices);

    return std::move(ret);
}



// load list of textures
std::vector<Texture> Model::loadTextures(aiMaterial* mat, aiTextureType type) {
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
