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
                            bool includeDefaultHeader, 
                            const std::string& vertexShaderPath, 
                            const std::string& fragShaderPath, 
                            const std::string& geoShaderPath = "") : vulkanDevice{device} {

    this->includeDefaultHeader = includeDefaultHeader;
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

void ShaderPipline::createPipeline(VkRenderPass renderPass) {
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	vulkanPipeline = std::make_unique<VulkanPipeline>(
			vulkanDevice,
			vertexShaderPath,
			fragShaderPath,
			pipelineConfig);
}



/*
    set uniform variables
*/

void ShaderPipline::setBool(const std::string& name, bool value) {
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void ShaderPipline::setInt(const std::string& name, int value) {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void ShaderPipline::setFloat(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void ShaderPipline::set3Float(const std::string& name, float v1, float v2, float v3) {
    glUniform3f(glGetUniformLocation(id, name.c_str()), v1, v2, v3);
}

void ShaderPipline::set3Float(const std::string& name, glm::vec3 v) {
    glUniform3f(glGetUniformLocation(id, name.c_str()), v.x, v.y, v.z);
}

void ShaderPipline::set4Float(const std::string& name, float v1, float v2, float v3, float v4) {
    glUniform4f(glGetUniformLocation(id, name.c_str()), v1, v2, v3, v4);
}

void ShaderPipline::set4Float(const std::string& name, aiColor4D color) {
    glUniform4f(glGetUniformLocation(id, name.c_str()), color.r, color.g, color.b, color.a);
}

void ShaderPipline::set4Float(const std::string& name, glm::vec4 v) {
    glUniform4f(glGetUniformLocation(id, name.c_str()), v.x, v.y, v.z, v.w);
}

void ShaderPipline::setMat3(const std::string& name, glm::mat3 val) {
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}

void ShaderPipline::setMat4(const std::string& name, glm::mat4 val) {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
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



