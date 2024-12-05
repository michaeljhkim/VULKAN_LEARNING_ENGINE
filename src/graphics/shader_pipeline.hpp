#pragma once

//#include "lve_camera.hpp"
#include "vulkan_device.hpp"
#include "vulkan_frame_info.hpp"
#include "vulkan_pipeline.hpp"
#include "game_object.hpp"


#include <glad/glad.h>

#include <string>
#include <sstream>
#include <iostream>

#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <filesystem>

/*
    class to represent shader program
*/

class ShaderPipline {
 public:
    // program ID
    unsigned int id;

    /*
        constructors
    */

    // default
    //ShaderPipline();

    // initialize with paths to vertex, fragment, and optional geometry shaders
    ShaderPipline::ShaderPipline(VulkanDevice& device, 
                            VkRenderPass renderPass, 
                            VkDescriptorSetLayout globalSetLayout,
                            bool includeDefaultHeader, 
                            const std::string& vertexShaderPath, 
                            const std::string& fragShaderPath, 
                            const std::string& geoShaderPath);

    /*
        process functions
    */
    //geoshader will usually be null, so figure something out later

    // activate shader
    void activate();

    // cleanup
    void cleanup();

    /*
        set uniform variables
    */

    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void set3Float(const std::string& name, float v1, float v2, float v3);
    void set3Float(const std::string& name, glm::vec3 v);
    void set4Float(const std::string& name, float v1, float v2, float v3, float v4);
    void set4Float(const std::string& name, aiColor4D color);
    void set4Float(const std::string& name, glm::vec4 v);
    void setMat3(const std::string& name, glm::mat3 val);
    void setMat4(const std::string& name, glm::mat4 val);

    /*
        static
    */

    // default directory
    static std::string defaultDirectory;

    // stream containing default header source
    static std::stringstream defaultHeaders;

    // load into default header
    static void loadIntoDefault(const std::string& filepath);

    // clear default header (after shader compilation)
    static void clearDefault();

    // load string from file
    static std::string ReadFile(const std::string& filePath);
    static std::vector<uint32_t> getOrCompileSPIRV(const std::string& glslFile, EShLanguage shaderType);
    

 private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(
        VkRenderPass renderPass);
    void createGraphicsPipeline(
        const PipelineConfigInfo& configInfo,
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const std::string& geoFilepath = "");

    VulkanDevice &vulkanDevice;
    static bool includeDefaultHeader;

    static std::string& vertexShaderPath;
    static std::string& fragShaderPath;
    static std::string& geoShaderPath;

    std::unique_ptr<VulkanPipeline> vulkanPipeline;
    VkPipelineLayout pipelineLayout;
    
};
