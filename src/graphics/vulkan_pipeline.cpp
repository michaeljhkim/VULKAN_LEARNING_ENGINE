#include "vulkan_pipeline.hpp"
#include "TBuiltInResource_default.hpp"

#include "model.hpp"

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


#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

//namespace lve {

VulkanPipeline::VulkanPipeline(
		VulkanDevice& device,
		const PipelineConfigInfo& configInfo,
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const std::string& geoFilepath)
		: vulkanDevice{device} {
	createGraphicsPipeline(configInfo, vertFilepath, fragFilepath, geoFilepath);
}

VulkanPipeline::VulkanPipeline(
		VulkanDevice& device,
		const PipelineConfigInfo& configInfo,
		bool includeDefaultHeader,
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const std::string& geoFilepath = "")
		: vulkanDevice{device} {
    includeDefaultHeader = includeDefaultHeader;
	createGraphicsPipeline(configInfo, vertFilepath, fragFilepath, geoFilepath);
}


VulkanPipeline::~VulkanPipeline() {
	vkDestroyShaderModule(vulkanDevice.device(), vertShaderModule, nullptr);
	vkDestroyShaderModule(vulkanDevice.device(), fragShaderModule, nullptr);
	vkDestroyShaderModule(vulkanDevice.device(), geoShaderModule, nullptr);
	vkDestroyPipeline(vulkanDevice.device(), graphicsPipeline, nullptr);
}

std::vector<char> VulkanPipeline::readFile(const std::string& filepath) {
	std::string enginePath = ENGINE_DIR + filepath;
	std::ifstream file{enginePath, std::ios::ate | std::ios::binary};

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file: " + enginePath);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}


		
void VulkanPipeline::createGraphicsPipeline(
        const PipelineConfigInfo& configInfo,
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const std::string& geoFilepath = "") {
	assert( configInfo.pipelineLayout != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
	assert( configInfo.renderPass != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline: no renderPass provided in configInfo");

    auto vertSpirv = getOrCompileSPIRV(vertFilepath, EShLangVertex);
    auto fragSpirv = getOrCompileSPIRV(fragFilepath, EShLangFragment);
	createShaderModule(vertSpirv, &vertShaderModule);
	createShaderModule(fragSpirv, &fragShaderModule);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    // Vertex Shader Stage
    VkPipelineShaderStageCreateInfo vertShaderStage{};
    vertShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStage.module = vertShaderModule;
    vertShaderStage.pName = "main";
    vertShaderStage.flags = 0;
    vertShaderStage.pNext = nullptr;
    vertShaderStage.pSpecializationInfo = nullptr;
    shaderStages.push_back(vertShaderStage);

    // Fragment Shader Stage
    VkPipelineShaderStageCreateInfo fragShaderStage{};
    fragShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStage.module = fragShaderModule;
    fragShaderStage.pName = "main";
    fragShaderStage.flags = 0;
    fragShaderStage.pNext = nullptr;
    fragShaderStage.pSpecializationInfo = nullptr;
    shaderStages.push_back(fragShaderStage);

    // We add Geomatry Shader stage only if we are procided one
    if ( !geoFilepath.empty() ) {
        auto geoSpirv = getOrCompileSPIRV(geoFilepath, EShLangGeometry);
	    createShaderModule(geoSpirv, &geoShaderModule);

        // Geometry Shader Stage
        VkPipelineShaderStageCreateInfo geoShaderStage{};
        geoShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        geoShaderStage.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        geoShaderStage.module = geoShaderModule;
        geoShaderStage.pName = "main";
        geoShaderStage.flags = 0;
        geoShaderStage.pNext = nullptr;
        geoShaderStage.pSpecializationInfo = nullptr;
        shaderStages.push_back(geoShaderStage);
    }


	auto& bindingDescriptions = configInfo.bindingDescriptions;
	auto& attributeDescriptions = configInfo.attributeDescriptions;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount =
			static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
	pipelineInfo.pViewportState = &configInfo.viewportInfo;
	pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
	pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
	pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
	pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
	pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

	pipelineInfo.layout = configInfo.pipelineLayout;
	pipelineInfo.renderPass = configInfo.renderPass;
	pipelineInfo.subpass = configInfo.subpass;

	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(
					vulkanDevice.device(),
					VK_NULL_HANDLE,
					1,
					&pipelineInfo,
					nullptr,
					&graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline");
	}
}


void VulkanPipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(vulkanDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module"); }
}

void VulkanPipeline::bind(VkCommandBuffer commandBuffer) {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

void VulkanPipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {
	configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.viewportInfo.viewportCount = 1;
	configInfo.viewportInfo.pViewports = nullptr;
	configInfo.viewportInfo.scissorCount = 1;
	configInfo.viewportInfo.pScissors = nullptr;

	configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
	configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.rasterizationInfo.lineWidth = 1.0f;
	configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
	configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;	// Optional
	configInfo.rasterizationInfo.depthBiasClamp = 0.0f;					 // Optional
	configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;		 // Optional

	configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
	configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	configInfo.multisampleInfo.minSampleShading = 1.0f;					 // Optional
	configInfo.multisampleInfo.pSampleMask = nullptr;						 // Optional
	configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;	// Optional
	configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;			 // Optional

	configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
	configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;	 // Optional
	configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;	// Optional
	configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;							// Optional
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;	 // Optional
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;	// Optional
	configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;							// Optional

	configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;	// Optional
	configInfo.colorBlendInfo.attachmentCount = 1;
	configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
	configInfo.colorBlendInfo.blendConstants[0] = 0.0f;	// Optional
	configInfo.colorBlendInfo.blendConstants[1] = 0.0f;	// Optional
	configInfo.colorBlendInfo.blendConstants[2] = 0.0f;	// Optional
	configInfo.colorBlendInfo.blendConstants[3] = 0.0f;	// Optional

	configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.minDepthBounds = 0.0f;	// Optional
	configInfo.depthStencilInfo.maxDepthBounds = 1.0f;	// Optional
	configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.front = {};	// Optional
	configInfo.depthStencilInfo.back = {};	 // Optional

	configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
	configInfo.dynamicStateInfo.dynamicStateCount =
			static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
	configInfo.dynamicStateInfo.flags = 0;

	configInfo.bindingDescriptions = Vertex::getBindingDescriptions();
	configInfo.attributeDescriptions = Vertex::getAttributeDescriptions();
}

void VulkanPipeline::enableAlphaBlending(PipelineConfigInfo& configInfo) {
	configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
	configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}





// stream containing default headers
std::stringstream VulkanPipeline::defaultHeaders;

// load into default header
void VulkanPipeline::loadIntoDefault(const std::string& filepath) {
    // Load shader source as a string (no need for manual memory management)
    std::string fileContents = readFile(filepath).data();
    // Append file contents directly to the default headers
    VulkanPipeline::defaultHeaders << fileContents;
}

// clear default header (after shader compilation)
void VulkanPipeline::clearDefault() {
    VulkanPipeline::defaultHeaders.clear();
}



/*
    process functions
*/


// load char from file
std::vector<char> VulkanPipeline::readFile(const std::string& filepath) {
    // Open the file in binary mode
	//std::string enginePath = ENGINE_DIR + filepath;
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath); }

    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg); // Move back to the start of the file

    // Handle files with default headers (non-SPIR-V)
    if (includeDefaultHeader && (std::filesystem::path(filepath).extension().string() != ".spv") ) {

        const std::string& headers = VulkanPipeline::defaultHeaders.str();
        size_t totalSize = fileSize + headers.size() + 1;
        std::vector<char> buffer(totalSize, '\0');	// Allocate buffer with space for headers and a null terminator

        std::memcpy(buffer.data(), headers.data(), headers.size());		// Copy default headers into the buffer
        file.read(buffer.data() + headers.size(), fileSize);			// Read file content after headers
        file.close();
        return buffer;
    }

    // Handle SPIR-V or files without default headers
    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}



std::vector<char> VulkanPipeline::getOrCompileSPIRV(const std::string& filePath, EShLanguage shaderType) {
    std::string spirvFilePath = VulkanPipeline::defaultDirectory + '/' + filePath + ".spv";

    // Check if SPIR-V file already exists
    if (std::filesystem::exists(spirvFilePath)) {
		return readFile(filePath); }

    // Initialize GLSLang shader
    glslang::TShader shader(shaderType);		// Read GLSL source code
    const char* sourceCStr = readFile(filePath).data();
    shader.setStrings(&sourceCStr, 1);
    TBuiltInResource defaultResource = InitResources();		//Defined in TBuiltInResource_default

    // Parse GLSL
    if (!shader.parse(&defaultResource, 100, false, EShMsgDefault)) {
        throw std::runtime_error("GLSL Parsing Failed: " + std::string(shader.getInfoLog())); }

    // Link into a program
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(EShMsgDefault)) {
        throw std::runtime_error("GLSL Linking Failed: " + std::string(program.getInfoLog())); }

    // Convert to SPIR-V
    std::vector<uint32_t> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(shaderType), spirv);

    // Convert the uint32_t vector into a char vector (byte-by-byte)
    std::vector<char> spirvCharData(reinterpret_cast<const char*>(spirv.data()), 
                                    reinterpret_cast<const char*>(spirv.data()) + spirv.size() * sizeof(uint32_t));

    // Save SPIR-V binary to a file
    std::ofstream spirvFile(spirvFilePath, std::ios::binary);
    if (!spirvFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + spirvFilePath); }
    spirvFile.write(spirvCharData.data(), spirvCharData.size());

    return spirvCharData;
}



