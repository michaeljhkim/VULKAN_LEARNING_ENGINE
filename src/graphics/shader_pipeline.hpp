#pragma once

#include "vulkan_device.hpp"
#include "vulkan_frame_info.hpp"
#include "vulkan_pipeline.hpp"



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
                            const std::string& vertexShaderPath, 
                            const std::string& fragShaderPath, 
                            const std::string& geoShaderPath = "");

    ShaderPipline::ShaderPipline(VulkanDevice& device, 
                            VkRenderPass renderPass, 
                            VkDescriptorSetLayout globalSetLayout,
                            bool includeDefaultHeader, 
                            const std::string& vertexShaderPath, 
                            const std::string& fragShaderPath, 
                            const std::string& geoShaderPath = "");

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
    template<typename T>
    void ShaderPipline::setData(const std::string& name, T value);
    void set3Float(const std::string& name, float v1, float v2, float v3);
    void set4Float(const std::string& name, float v1, float v2, float v3, float v4);

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
    void renderGameObjects(FrameInfo& frameInfo);

    VkPipelineLayout getPipelineLayout() { return pipelineLayout; };

 private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    VulkanDevice &vulkanDevice;
    static const bool includeDefaultHeader = false;

    static std::string& vertexShaderPath;
    static std::string& fragShaderPath;
    static std::string& geoShaderPath;

    std::unique_ptr<VulkanPipeline> shaderPipeline;
    VkPipelineLayout pipelineLayout;
    
};
