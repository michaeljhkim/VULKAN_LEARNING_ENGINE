#pragma once

#include "vulkan_device.hpp"
#include <vulkan/vulkan.hpp>

// std
#include <string>
#include <vector>

#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <filesystem>



struct PipelineConfigInfo {
	PipelineConfigInfo() = default;
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector<VkDynamicState> dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	//VkPipelineLayout pipelineLayout = nullptr;
	//VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class VulkanPipeline {
 public:
	VulkanPipeline(
			VulkanDevice& device,
			const PipelineConfigInfo& configInfo,
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const std::string& geoFilepath = "");

	VulkanPipeline(
			VulkanDevice& device,
			const PipelineConfigInfo& configInfo,
            bool includeDefaultHeader,
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const std::string& geoFilepath = "");
	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline&) = delete;
	VulkanPipeline& operator=(const VulkanPipeline&) = delete;

	void bind(VkCommandBuffer commandBuffer);

	static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
	static void enableAlphaBlending(PipelineConfigInfo& configInfo);


    static std::string defaultDirectory;						// default directory
    static std::stringstream defaultHeaders;					// stream containing default header source
    static void loadIntoDefault(const std::string& filepath);	// load into default header
    static void clearDefault();									// clear default header (after shader compilation)
    static std::vector<char> getOrCompileSPIRV(const std::string& glslFile, EShLanguage shaderType);

 private:
	static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(
        const PipelineConfigInfo& configInfo,
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const std::string& geoFilepath = "");

	void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
    static const bool includeDefaultHeader = false;

	VulkanDevice& vulkanDevice;
	VkPipeline graphicsPipeline;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	static VkShaderModule geoShaderModule;
};

