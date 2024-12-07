#include "shader_pipeline.hpp"

#include <stdio.h>
#include <fstream>

#include <string>
#include <vector>
#include <iostream>
#include <cstdint>

// std
#include <array>
#include <cassert>
#include <stdexcept>


struct SimplePushConstantData {
	glm::mat4 modelMatrix{1.f};
	glm::mat4 normalMatrix{1.f};
};

/*
    constructors
*/

// default
//ShaderPipline::ShaderPipline() {}

// initialize with paths to vertex and fragment shaders
ShaderPipline::ShaderPipline(VulkanDevice& device, 
                            VkRenderPass renderPass, 
                            VkDescriptorSetLayout globalSetLayout,
                            const std::string& vertexShaderPath, 
                            const std::string& fragShaderPath, 
                            const std::string& geoShaderPath = "") : vulkanDevice{device} {
    this->vertexShaderPath = vertexShaderPath;
    this->fragShaderPath = fragShaderPath;
    this->geoShaderPath = geoShaderPath;
	createPipelineLayout(globalSetLayout);
	createPipeline(renderPass);
}

ShaderPipline::ShaderPipline(VulkanDevice& device, 
                            VkRenderPass renderPass, 
                            VkDescriptorSetLayout globalSetLayout,
                            bool includeDefaultHeader, 
                            const std::string& vertexShaderPath, 
                            const std::string& fragShaderPath, 
                            const std::string& geoShaderPath = "") : vulkanDevice{device} {
    includeDefaultHeader = includeDefaultHeader;
    this->vertexShaderPath = vertexShaderPath;
    this->fragShaderPath = fragShaderPath;
    this->geoShaderPath = geoShaderPath;
	createPipelineLayout(globalSetLayout);
	createPipeline(renderPass);
}


void ShaderPipline::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
	VkPushConstantRange pushConstantRange{};
    //pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | (geoShaderPath.empty() ? 0 : VK_SHADER_STAGE_GEOMETRY_BIT);
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(vulkanDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

//CONSIDER HOW TO UPLOAD DATA TO SHADERS IN ORDER FOR SHADERS TO WORK
void ShaderPipline::createPipeline(VkRenderPass renderPass) {
	//assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
	assert(pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	shaderPipeline = std::make_unique<VulkanPipeline>(
			vulkanDevice,
			pipelineConfig,
            includeDefaultHeader,
			vertexShaderPath,
			fragShaderPath,
            geoShaderPath);
    
    /*
	vulkanPipeline = std::make_unique<VulkanPipeline>(
			vulkanDevice,
			pipelineConfig,
			vertexShaderPath,
			fragShaderPath,
            geoShaderPath);
    */
}



/*
    set uniform variables
*/

template<typename T>
void ShaderPipline::setData(const std::string& name, T value) {
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(T), &value);
}

void ShaderPipline::set3Float(const std::string& name, float v1, float v2, float v3) {
    glUniform3f(glGetUniformLocation(id, name.c_str()), v1, v2, v3);
}

void ShaderPipline::set4Float(const std::string& name, float v1, float v2, float v3, float v4) {
    glUniform4f(glGetUniformLocation(id, name.c_str()), v1, v2, v3, v4);
}



void ShaderPipline::renderGameObjects(FrameInfo& frameInfo) {
    shaderPipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr);

    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.model == nullptr) continue;
        //HERE IS WHERE PHYSICS TRANSFORMATIONS MUST BE
        SimplePushConstantData push{};
        push.modelMatrix = obj.transform.mat4();
        push.normalMatrix = obj.transform.normalMatrix();

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | (geoShaderPath.empty() ? 0 : VK_SHADER_STAGE_GEOMETRY_BIT),
            0,
            sizeof(SimplePushConstantData),
            &push);
        //obj.render(shader_pipeline, dt, frameInfo.commandBuffer);
        //obj.model->bind(frameInfo.commandBuffer);
        //obj.model->draw(frameInfo.commandBuffer);
    }
}


/*
void initializeGlslang() {
    glslang::InitializeProcess();
}

void finalizeGlslang() {
    glslang::FinalizeProcess();
}
*/

/*
    static
*/



