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
size_t hashData(const unsigned char* data, size_t dataSize) {
	std::vector<unsigned char> texData(data, data + dataSize);
    std::hash<std::string> hash_fn;
    return hash_fn(std::string(texData.begin(), texData.end()));
}


// Save texture tiles to the file
void Texture::saveTextureTiles(unsigned char* pixels, int texWidth, int texHeight, std::ofstream& outFile) {
    // Write the header
    int tilesX = texWidth / TILE_WIDTH;
    int tilesY = texHeight / TILE_HEIGHT;
    int totalTiles = tilesX * tilesY;

    outFile.write(reinterpret_cast<const char*>(&totalTiles), sizeof(totalTiles));
    outFile.write(reinterpret_cast<const char*>(&TILE_WIDTH), sizeof(TILE_WIDTH));
    outFile.write(reinterpret_cast<const char*>(&TILE_HEIGHT), sizeof(TILE_HEIGHT));
    outFile.write(reinterpret_cast<const char*>(&textureHash), sizeof(textureHash));	// Calculate and write the texture hash

    std::streampos offsetTablePosition = outFile.tellp();								// Offset table placeholder
    outFile.write(reinterpret_cast<const char*>(&offsetTablePosition), sizeof(offsetTablePosition));

    // Process tiles
    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            Tile tile;
            tile.offset = static_cast<int>(outFile.tellp());

            // Extract tile pixels into tileData
            for (int y = 0; y < TILE_HEIGHT; ++y) {
                for (int x = 0; x < TILE_WIDTH; ++x) {
                    int srcX = tx * TILE_WIDTH + x;
                    int srcY = ty * TILE_HEIGHT + y;
                    int srcIdx = (srcY * texWidth + srcX) * 4;

                    std::copy_n(&pixels[srcIdx], 4, &tile.tile_pixels[(y * TILE_WIDTH + x) * 4]);
                }
            }

            // Compute hash and write tile data
            tile.hash = hashData(tile.tile_pixels.data(), TILE_SIZE);

            // Write the tile into the file
            outFile.write(reinterpret_cast<const char*>(tile.tile_pixels.data()), TILE_SIZE);
            outFile.write(reinterpret_cast<const char*>(&tile.offset), sizeof(tile.offset));
            outFile.write(reinterpret_cast<const char*>(&tile.hash), sizeof(tile.hash));
        }
    }

}



// Pack multiple textures into the output file
void Texture::packTextures(const std::vector<std::string>& texturePaths, const std::string& outputFile, bool flip) {
    //std::ofstream outFile(outputFile, std::ios::binary | std::ios::app);
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    // Global lookup table (load existing data)
    std::vector<TextureMetadata> globalLookupTable = loadGlobalLookupTable(outputFile);

    for (const auto& texturePath : texturePaths) {
        int texWidth, texHeight, texChannels;
        stbi_set_flip_vertically_on_load(flip);
        unsigned char* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        // Validate texture dimensions
        if (!pixels || (texWidth % TILE_WIDTH != 0 || texHeight % TILE_HEIGHT != 0) ) {
            std::cerr << "Failed to load texture: " << texturePath << std::endl;
            std::cerr << "Or Texture dimensions not divisible by tile size: " << texturePath << std::endl;
            stbi_image_free(pixels);
            continue;
        }
		
        // Check for duplicates
        auto it = std::find_if(globalLookupTable.begin(), globalLookupTable.end(),
								[](const TextureMetadata& metadata) {
									return metadata.textureHash == Texture::textureHash;
								});

        if (it != globalLookupTable.end()) {
            std::cout << "Texture already exists in the file, skipping: " << texturePath << std::endl;
            stbi_image_free(pixels);
            continue;
        }

        // Save texture and update metadata
        TextureMetadata metadata;
        metadata.textureHash = textureHash;
        metadata.offset = outFile.tellp();
        metadata.width = texWidth;
        metadata.height = texHeight;
        metadata.tileCount = (texWidth / TILE_WIDTH) * (texHeight / TILE_HEIGHT);

        saveTextureTiles(pixels, texWidth, texHeight, outFile);
        globalLookupTable.push_back(metadata);
        stbi_image_free(pixels);
    }

    // Save updated lookup table
    saveGlobalLookupTable(globalLookupTable, outFile);

    outFile.close();
    std::cout << "Textures packed successfully!" << std::endl;
}




void Texture::saveGlobalLookupTable(const std::vector<TextureMetadata>& globalLookupTable, std::ofstream& outFile) {
    // Write metadata for each texture
    for (const auto& metadata : globalLookupTable) {
        outFile.write(reinterpret_cast<const char*>(&metadata.textureHash), sizeof(metadata.textureHash));
        outFile.write(reinterpret_cast<const char*>(&metadata.offset), sizeof(metadata.offset));
        outFile.write(reinterpret_cast<const char*>(&metadata.width), sizeof(metadata.width));
        outFile.write(reinterpret_cast<const char*>(&metadata.height), sizeof(metadata.height));
        outFile.write(reinterpret_cast<const char*>(&metadata.tileCount), sizeof(metadata.tileCount));
    }

    // Write the number of textures in the lookup table
    int textureCount = globalLookupTable.size();
    outFile.write(reinterpret_cast<const char*>(&textureCount), sizeof(textureCount));
}


std::vector<TextureMetadata> loadGlobalLookupTable(const std::string& outputFile) {
    std::ifstream inFile(outputFile, std::ios::binary);
    std::vector<TextureMetadata> globalLookupTable;
    if (!inFile) {
        std::cerr << "Failed to open file for reading." << std::endl;
        return globalLookupTable; 		// Return an empty table
    }

    // Seek to the end of the file to check if the lookup table exists
    inFile.seekg(0, std::ios::end);
    std::streampos fileSize = inFile.tellg();

    if (fileSize < sizeof(int)) {
        std::cerr << "File is too small to contain a valid lookup table." << std::endl;
        return globalLookupTable;
    }

    // Seek back to where the texture count is stored and Read the number of textures in the table
    inFile.seekg(fileSize - static_cast<std::streamoff>(sizeof(int)), std::ios::beg);
    int textureCount = 0;
    inFile.read(reinterpret_cast<char*>(&textureCount), sizeof(textureCount));

    // Validate the table size
    if (textureCount <= 0) {
        return globalLookupTable;
    }

    // Seek to where the lookup table starts
    std::streampos tableStart = fileSize - static_cast<std::streamoff>( sizeof(int) - (textureCount * sizeof(TextureMetadata)) );
    inFile.seekg(tableStart, std::ios::beg);

    // Read all texture metadata
    for (int i = 0; i < textureCount; ++i) {
        TextureMetadata metadata;
        inFile.read(reinterpret_cast<char*>(&metadata.textureHash), sizeof(metadata.textureHash));
        inFile.read(reinterpret_cast<char*>(&metadata.offset), sizeof(metadata.offset));
        inFile.read(reinterpret_cast<char*>(&metadata.width), sizeof(metadata.width));
        inFile.read(reinterpret_cast<char*>(&metadata.height), sizeof(metadata.height));
        inFile.read(reinterpret_cast<char*>(&metadata.tileCount), sizeof(metadata.tileCount));
        globalLookupTable.push_back(metadata);
    }

    return globalLookupTable;
}


/*
std::vector<Tile> loadTilesForTexture(const std::string& outputFile, size_t textureHash) {
    std::ifstream inFile(outputFile, std::ios::binary);
    std::vector<Tile> tiles;

    if (!inFile) {
        std::cerr << "Failed to open file for reading." << std::endl;
        return tiles; // Return an empty list if the file can't be opened
    }

    // Load the global lookup table
    std::vector<TextureMetadata> globalLookupTable = loadGlobalLookupTable(outputFile);

    // Find the texture metadata for the given textureHash
    auto it = std::find_if(globalLookupTable.begin(), globalLookupTable.end(),
                           [textureHash](const TextureMetadata& metadata) {
                               return metadata.textureHash == textureHash;
                           });

    if (it == globalLookupTable.end()) {
        std::cerr << "Texture with hash " << textureHash << " not found." << std::endl;
        return tiles; // Return empty if texture not found
    }

    // Get the metadata for the texture
    TextureMetadata metadata = *it;

    // Seek to the position where the tiles start in the file
    inFile.seekg(metadata.offset, std::ios::beg);

    // Read the tiles
    for (int i = 0; i < metadata.tileCount; ++i) {
        TileData tile;

        // Assuming TileData has fields such as width, height, and data buffer (adjust according to actual format)
        inFile.read(reinterpret_cast<char*>(&tile.width), sizeof(tile.width));
        inFile.read(reinterpret_cast<char*>(&tile.height), sizeof(tile.height));
        inFile.read(reinterpret_cast<char*>(&tile.data), tile.width * tile.height * sizeof(char));  // Assuming 1 byte per pixel, adjust if necessary

        tiles.push_back(tile);
    }

    return tiles;
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