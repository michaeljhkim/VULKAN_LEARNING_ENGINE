//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

#include "vulkan_buffer.hpp"
#include <vulkan/vulkan.hpp>

#include <assimp/scene.h>
#include <stb/stb_image.h>



struct Tile {
    int offset;    // File offset of the tile data
    size_t hash;   // Hash of the tile data
};


struct TextureMetadata {
    size_t textureHash;   // Hash of the entire texture
    int offset;           // Offset in the file where the texture data starts
    int width;            // Texture width
    int height;           // Texture height
    int tileCount;        // Total number of tiles for this texture
};


/*
    class to represent texture
*/

class Texture {
public:
    // Tile dimensions
    const int TILE_WIDTH = 128;
    const int TILE_HEIGHT = 128;
    const int TILE_SIZE = TILE_WIDTH * TILE_HEIGHT * 4; // RGBA (4 bytes per pixel)

    // texture hash
    static size_t textureHash;

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
    void saveTextureTiles(unsigned char* pixels, int texWidth, int texHeight, std::ofstream& outFile);
    void packTextures(const std::vector<std::string>& texturePaths, const std::string& outputFile, bool flip);
    void saveGlobalLookupTable(const std::vector<TextureMetadata>& globalLookupTable, std::ofstream& outFile);
    std::vector<TextureMetadata> loadGlobalLookupTable(const std::string& outputFile);

    void allocate(enum format, unsigned int width, unsigned int height, enum type);

    /*
    static void setParams(enum texMinFilter = GL_NEAREST,
        enum texMagFilter = GL_NEAREST,
        enum wrapS = GL_REPEAT,
        enum wrapT = GL_REPEAT);
    */

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

	VkDescriptorImageInfo descriptorImageInfo(VkSampler textureSampler, VkImageView textureImageView);
};
