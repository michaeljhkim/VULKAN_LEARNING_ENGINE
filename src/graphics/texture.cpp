#include "texture.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <unordered_set>

/*
    constructor
*/

Texture::Texture(VulkanDevice &vulkanDevice, std::string name) : 
                                        vulkanDevice(vulkanDevice), name(name), type(aiTextureType_NONE) {
    generate();
}

// initialize with image path and type
Texture::Texture(VulkanDevice &vulkanDevice, std::string dir, std::string path, aiTextureType type) : 
                                        vulkanDevice(vulkanDevice), dir(dir), path(path), type(type) {
    generate();
}

// generate texture hash
void Texture::generate() {
    textureHash = hashData(pixels, texWidth * texHeight * 4);
}







// Hash data (e.g., tile or texture data)
std::string hashData(const unsigned char* data, size_t dataSize) {
    std::string dataString(reinterpret_cast<const char*>(data), dataSize);
    std::hash<std::string> hash_fn;
    return std::to_string(hash_fn(dataString));
}

// Function to split texture into tiles and save as KTX2
void Texture::splitTextureAndSaveAsKTX2(const std::string& inputFile, const std::string& outputFile) {
    int width, height, channels;
    unsigned char* image = stbi_load(inputFile.c_str(), &width, &height, &channels, 0);
    if (!image) {
        std::cerr << "Failed to load image: " << inputFile << std::endl;
        return;
    }

    size_t texSize = width * height * channels;
    if (width % TILE_WIDTH != 0 || height % TILE_HEIGHT != 0) {
        std::cerr << "Image dimensions are not evenly divisible by tile size!" << std::endl;
        stbi_image_free(image);
        return;
    }
    int numTilesX = width / TILE_WIDTH;
    int numTilesY = height / TILE_HEIGHT;
    int numTiles = numTilesX * numTilesY;

    // Prepare a vector to store all tile data
    std::vector<unsigned char> allTilesData(TILE_WIDTH * TILE_HEIGHT * channels * numTiles);
    std::string textureHash = hashData(image, texSize); // Create a unique hash for this texture
    ktxHashList* tileHashList = nullptr;    // Create a hash list for this texture's metadata (tiles)
    ktxHashList_Create(&tileHashList);
    
    for (int y = 0; y < numTilesY; ++y) {
        for (int x = 0; x < numTilesX; ++x) {
            int tileIndex = y * numTilesX + x;
            size_t tileSize = TILE_WIDTH * TILE_HEIGHT * channels;
            unsigned char* tile = &allTilesData[tileIndex * tileSize];

            // Extract the tile data from the image
            for (int row = 0; row < TILE_HEIGHT; ++row) {
                std::memcpy(
                    tile + row * TILE_WIDTH * channels,
                    image + ((y * TILE_HEIGHT + row) * width + x * TILE_WIDTH) * channels,
                    TILE_WIDTH * channels
                );
            }

            std::string tileKey = textureHash + std::to_string(tileIndex);      // Create a unique key for each tile using textureHash and tile index

            KTX_error_code result = ktxHashList_AddKVPair(tileHashList, tileKey.c_str(), tileSize, tile);       // Add tile data to hash list
            if (result != KTX_SUCCESS) {
                std::cerr << "Error adding tile metadata to KTX2 file" << std::endl;
                stbi_image_free(image);
                return;
            }
        }
    }

    stbi_image_free(image); // Free the image data after extracting the tiles

    // Create the KTX2 texture
    ktxTexture2* ktxFileTexture = nullptr;
    ktxTextureCreateInfo createInfo{};
    createInfo.vkFormat = VK_FORMAT_R8G8B8_UNORM; // Adjust for your texture's format
    createInfo.baseWidth = TILE_WIDTH;
    createInfo.baseHeight = TILE_HEIGHT;
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 2;
    createInfo.numLevels = 1; // Only one level of detail (no mipmaps)
    createInfo.numLayers = numTiles; // Each tile is a layer
    createInfo.numFaces = 1;

    ktx_error_code_e result = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &ktxFileTexture);
    if (result != KTX_SUCCESS) {
        std::cerr << "Failed to create KTX2 texture: " << ktxErrorString(result) << std::endl;
        return;
    }

    // Upload the tile data to the KTX2 texture
    for (int tileIndex = 0; tileIndex < numTiles; ++tileIndex) {
        size_t tileSize = TILE_WIDTH * TILE_HEIGHT * channels;
        ktx_size_t offset = tileIndex * tileSize;
        result = ktxTexture2_SetImageFromMemory(ktxFileTexture, 0, tileIndex, 0, &allTilesData[offset], tileSize);
        if (result != KTX_SUCCESS) {
            std::cerr << "Failed to set image data for tile " << tileIndex << ": " << ktxErrorString(result) << std::endl;
            ktxTexture2_Destroy(ktxFileTexture);
            return;
        }
    }

    // Add the texture hash metadata to the KTX2 file and then Write the KTX2 texture to a file
    ktxHashList_AddKVPair(&ktxFileTexture->kvDataHead, "TextureHash", textureHash.size(), textureHash.c_str());
    result = ktxTexture2_WriteToNamedFile(ktxFileTexture, outputFile.c_str());

    if (result != KTX_SUCCESS) {
        std::cerr << "Failed to write KTX2 file: " << ktxErrorString(result) << std::endl;
    } else {
        std::cout << "KTX2 file saved as " << outputFile << std::endl;
    }

    // Clean up
    ktxTexture2_Destroy(ktxFileTexture);
}


/*
// Function to retrieve a texture based on a texture hash in the KTX2 file
ktxTexture2* getTextureFromKTX2(ktxFile* ktxFile, const std::string& textureHash) {
    ktxHashList* hashList = nullptr;
    KTX_error_code result = ktxFile->kvDataHead; // Access the main metadata of the KTX2 file

    ktxHashListIterator* iterator = nullptr;
    result = ktxHashList_GetIterator(hashList, &iterator); // Create an iterator for the hash list
    if (result != KTX_SUCCESS) {
        std::cerr << "Error creating hash list iterator" << std::endl;
        return nullptr;
    }

    // Iterate through the hash list to find the texture key
    while (ktxHashListIterator_GetNext(iterator) == KTX_SUCCESS) {
        const char* currentKey;
        const char* value;
        size_t keyLength, valueLength;

        ktxHashListIterator_GetKey(iterator, &currentKey, &keyLength);
        ktxHashListIterator_GetValue(iterator, &value, &valueLength);

        // Check if the current key matches the texture hash
        if (strncmp(currentKey, textureHash.c_str(), keyLength) == 0) {
            // Once the texture is found, return the corresponding texture data
            return reinterpret_cast<ktxTexture2*>(value);
        }
    }

    // If no match is found, return nullptr
    std::cerr << "Texture not found in KTX2 file" << std::endl;
    return nullptr;
}


// Function to retrieve a tile for a texture based on its tile index
const void* getTileFromTexture(ktxTexture2* texture, uint32_t tileIndex) {
    ktxHashList* hashList = texture->kvDataHead;  // Access the hash list for this texture
    ktxHashListIterator* iterator = nullptr;

    // Create an iterator for the hash list
    KTX_error_code result = ktxHashList_GetIterator(hashList, &iterator);
    if (result != KTX_SUCCESS) {
        std::cerr << "Error creating hash list iterator" << std::endl;
        return nullptr;
    }

    // Generate the key for the tile based on texture hash and tile index
    std::string tileKey = std::to_string(tileIndex);  // You may use texture hash + tile index as key
    const char* key = tileKey.c_str();

    // Iterate through the hash list to find the matching key for the tile
    while (ktxHashListIterator_GetNext(iterator) == KTX_SUCCESS) {
        const char* currentKey;
        const char* value;
        size_t keyLength, valueLength;

        ktxHashListIterator_GetKey(iterator, &currentKey, &keyLength);
        ktxHashListIterator_GetValue(iterator, &value, &valueLength);

        // Check if the current key matches the tile key
        if (strncmp(currentKey, key, keyLength) == 0) {
            // Return the corresponding tile data
            return reinterpret_cast<const void*>(value);
        }
    }

    // If no match is found, return nullptr
    std::cerr << "Tile not found for the specified texture" << std::endl;
    return nullptr;
}
*/





























// I will use this for single big texture atlases (smaller projects)



// load texture from path
void Texture::load(VulkanDevice &vulkanDevice, bool flip) {
    stbi_set_flip_vertically_on_load(flip);
    pixels = stbi_load((dir + "/" + path).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;


    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vulkanDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                            stagingBuffer, stagingBufferMemory);
    
    
    void* data;
    vkMapMemory(vulkanDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(vulkanDevice.device(), stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(vulkanDevice.device(), stagingBuffer, nullptr);
    vkFreeMemory(vulkanDevice.device(), stagingBufferMemory, nullptr);

    //we now have a textureImage
}







void Texture::createTextureImageView() {
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

void Texture::createTextureSampler() {
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(vulkanDevice.physical(), &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(vulkanDevice.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

// maybe the big texture array holds both textureSampler and the imageView
// Perhaps put this inside of sampler, and make this take in a texture array of somekind
// loop over all discriptor sets
// still need to figure out how to incorporate this with the overall vulkan descriptors class
// part I want this to interact with is writeImage in VulkanDescriptorWriter

void Texture::updateTextureSampler(VkDescriptorSet descriptorSet) {
    /*
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(vulkanDevice.physical(), &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(vulkanDevice.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
    */
   
    // For updating, only new VkWriteDescriptorSets need to be added to update function
    for (size_t i = 0; i < textureArray.size(); ++i) {
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.sampler = textureArray[i].textureSampler;
        imageInfo.imageView = textureArray[i].textureImageView;

        VkWriteDescriptorSet descriptorWrite = {};  // "write" instead of "descriptorWrite" in the descriptors cpp file
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = static_cast<uint32_t>(i);
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(vulkanDevice.device(), 1, &descriptorWrite, 0, nullptr);
    }

}




VkImageView Texture::createImageView(VkImage image, VkFormat format) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(vulkanDevice.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view!");
	}

	return imageView;
}

void Texture::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
				VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(vulkanDevice.device(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(vulkanDevice.device(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = vulkanDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(vulkanDevice.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(vulkanDevice.device(), image, imageMemory, 0);
}

void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = vulkanDevice.beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	vulkanDevice.endSingleTimeCommands(commandBuffer);
}

void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = vulkanDevice.beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	vulkanDevice.endSingleTimeCommands(commandBuffer);
}