#include "game_object.hpp"


struct SimplePushConstantData {
	Material material;
    bool noNormalMap;
    bool TexExists;
};

// initialize with parameters
/*
GameObject::GameObject(VulkanDevice &device, std::string name, unsigned int maxNumInstances, unsigned int flags)
: vulkanDevice{device}, model_name(name), switches(flags), currentNumInstances(0), maxNumInstances(maxNumInstances), instances(maxNumInstances), collision(nullptr) {}
*/
GameObject::GameObject(VulkanDevice &device): vulkanDevice{device} {}

void GameObject::attachModel(std::string name, std::string filePath, unsigned int maxNumInstances, unsigned int flags = 0) {
    model = std::make_unique<Model>(vulkanDevice, flags);
    model->loadModel(filePath);

    this->model_name = name;
    this->maxNumInstances = maxNumInstances;
    this->currentNumInstances = 0;
    this->instances.reserve(maxNumInstances);
    this->collision = nullptr;
    this->indirectCommands = std::move(model->indirectCommands);
}

// enable a collision model
void GameObject::enableCollisionModel() {
    if (!this->collision) {
        this->collision = std::make_unique<CollisionModel>(model);
    }
}


// render instance(s)
void GameObject::render(ShaderPipline& shader_pipeline, float dt, VkCommandBuffer& commandBuffer) {
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
                // update Rigid Body and activate moved switch
                instances[i]->update(dt);
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
            instanceBuffer->writeToBuffer((void*)models.data() );

            normalInstanceBuffer->map();
            normalInstanceBuffer->writeToBuffer((void*)normalModels.data() );

            for (unsigned int i = 0, numMeshes = model->meshes.size(); i < numMeshes; i++) {
                model->bind(commandBuffer, instanceBuffer->getBuffer(), normalInstanceBuffer->getBuffer());
                model->draw(commandBuffer, indirectCommandBuffer->getBuffer(), currentNumInstances);
            }
        }
    }

    // set shininess
    //shader_pipeline.setFloat("material.shininess", 0.5f);
    SimplePushConstantData push{};
    push.material.shininess = 0.5f;     //Temp value for testing - change later
    vkCmdPushConstants(
            commandBuffer,
            shader_pipeline.getPipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push);

    // render each mesh
    for (unsigned int i = 0, numMeshes = model->meshes.size(); i < numMeshes; i++) {
        model->meshes[i]->render(shader_pipeline, currentNumInstances);
    }
}


//CONSIDER MOVING BOUNDING REGION ARRAY HERE

// generate instance with parameters
RigidBody* GameObject::generateInstance(glm::vec3 size, float mass, glm::vec3 pos, glm::vec3 rot) {
    // all slots filled
    if (currentNumInstances >= maxNumInstances) {
        return nullptr; 
    }
    instances[currentNumInstances] = new RigidBody(model_name, size, mass, pos, rot);   // instantiate new instance

    // Optimize this later if possible
    for (unsigned int i = 0; i < model->meshes.size(); i++) {
        //indirectCommands[currentNumInstances]->indexCount = model->meshes[currentNumInstances]->indices.size();   //Already defined
        indirectCommands[i]->instanceCount = currentNumInstances;       // Dynamic count based on the frame
    }
    return instances[currentNumInstances++];
}


//Later, MAYBE make it so that it only binds and draws if the instances exists IDK I gotta do some more digging
void GameObject::initInstances() {
    // default values
    std::unique_ptr<glm::mat4> posData = nullptr;
    std::unique_ptr<glm::mat3> normalData = nullptr;

    std::vector<glm::mat4> positions(currentNumInstances);
    std::vector<glm::mat3> normals(currentNumInstances);

    if (States::isActive(&switches, CONST_INSTANCES)) {
        // instances won't change, set data pointers

        for (unsigned int i = 0; i < currentNumInstances; i++) {
            positions[i] = instances[i]->model;
            normals[i] = instances[i]->normalModel;
        }

        if (currentNumInstances) {
            posData = std::make_unique<glm::mat4>(&positions[0]);
            normalData = std::make_unique<glm::mat3>(&normals[0]);
        }
    }

    uint32_t max_instances = static_cast<uint32_t>(maxNumInstances);
	assert(max_instances >= 1 && "Max instance count must be at least 1");

	uint32_t dataSize = sizeof(posData);
	instanceBuffer = std::make_unique<VulkanBuffer>(
			vulkanDevice,
			dataSize,
			max_instances,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	uint32_t normalDataSize = sizeof(normalData);
    normalInstanceBuffer = std::make_unique<VulkanBuffer>(
			vulkanDevice,
			normalDataSize,
			max_instances,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	uint32_t indirectCommandSize = sizeof(VkDrawIndexedIndirectCommand);
    indirectCommandBuffer = std::make_unique<VulkanBuffer>(
			vulkanDevice,
			indirectCommandSize,
			max_instances,
			VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}



// remove instance at idx
void GameObject::removeInstance(unsigned int idx) {
    if (idx < maxNumInstances) {
        // shift down
        for (unsigned int i = idx + 1; i < currentNumInstances; i++) {
            instances[i - 1] = instances[i];
        }
        currentNumInstances--;
        for (unsigned int i = 0; i < model->meshes.size(); i++) {
            indirectCommands[i]->instanceCount = currentNumInstances;       // Dynamic count based on the frame
        }
    }
}

// remove instance with id
void GameObject::removeInstance(std::string instanceId) {
    int idx = getIdx(instanceId);
    if (idx != -1) {
        removeInstance(idx);
    }
}

// get index of instance with id
unsigned int GameObject::getIdx(std::string id) {
    // test each instance
    for (int i = 0; i < currentNumInstances; i++) {
        if (instances[i]->instanceId == id) {
            return i;
        }
    }
    return -1;
}


/*
GameObject GameObject::makePointLight(float intensity, float radius, glm::vec3 color) {
  GameObject gameObj = GameObject::createGameObject();
  gameObj.color = color;
  gameObj.transform.scale.x = radius;
  gameObj.pointLight = std::make_unique<PointLightComponent>();
  gameObj.pointLight->lightIntensity = intensity;
  return gameObj;
}
*/
