#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "vulkan_buffer.hpp"
#include <vulkan/vulkan.hpp>

#include <assimp/scene.h>
#include <stb/stb_image.h>

/*
    class to represent texture
*/

class Texture {
public:
    /*
        constructor
    */
    // initialize with name
    Texture(VulkanDevice &vulkanDevice, std::string name);

    // initialize with image path and type
    Texture(VulkanDevice &vulkanDevice, std::string dir, std::string path, aiTextureType type);

    // generate texture id
    void generate();

    // load texture from path
    void load(VulkanDevice &vulkanDevice, bool flip = true);

    void allocate(GLenum format, GLuint width, GLuint height, GLenum type);

    static void setParams(GLenum texMinFilter = GL_NEAREST,
        GLenum texMagFilter = GL_NEAREST,
        GLenum wrapS = GL_REPEAT,
        GLenum wrapT = GL_REPEAT);

    // bind texture id
    void bind();

    void cleanup();
    

    /*
        texture object values
    */
    
    // texture id
    unsigned int id;
    // texture type
    aiTextureType type;
    // name
    std::string name;
    // directory of image
    std::string dir;
    // name of image
    std::string path;


	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels;
    VkDeviceSize imageSize;

    void createTextureImageView();
    void createTextureSampler();
    VkImageView createImageView(VkImage image, VkFormat format);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
				VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void updateTextureSampler(VkDescriptorSet descriptorSet);

	VulkanDevice &vulkanDevice;
};
