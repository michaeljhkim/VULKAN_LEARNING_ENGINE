#pragma once

#include "vulkan_device.hpp"
#include "vulkan_buffer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//namespace lve {

class VulkanImage{
 public:
	VulkanImage(
			VulkanDevice& device,
			VkDeviceSize instanceSize,
			uint32_t instanceCount,
			VkBufferUsageFlags usageFlags,
			VkMemoryPropertyFlags memoryPropertyFlags,
			VkDeviceSize minOffsetAlignment = 1);
	~VulkanImage();

	VulkanBuffer(const VulkanBuffer&) = delete;
	VulkanBuffer& operator=(const VulkanBuffer&) = delete;

	void UpdateInstanceBuffer(const std::vector<glm::mat4>& instances);
	VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	void unmap();

	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	VkImageView createImageView(VkImage image, VkFormat format);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
				VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	void writeToIndex(void* data, int index);
	VkResult flushIndex(int index);
	VkDescriptorBufferInfo descriptorInfoForIndex(int index);
	VkResult invalidateIndex(int index);

	VkBuffer getBuffer() const { return buffer; }
	void* getMappedMemory() const { return mapped; }
	uint32_t getInstanceCount() const { return instanceCount; }
	VkDeviceSize getInstanceSize() const { return instanceSize; }
	VkDeviceSize getAlignmentSize() const { return instanceSize; }
	VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
	VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
	VkDeviceSize getBufferSize() const { return bufferSize; }
	VkDeviceMemory getMemory() const { return memory; }

 private:
	static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

	VulkanDevice& vulkanDevice;
	void* mapped = nullptr;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkDeviceSize bufferSize;
	uint32_t instanceCount;
	VkDeviceSize instanceSize;
	VkDeviceSize alignmentSize;
	VkBufferUsageFlags usageFlags;
	VkMemoryPropertyFlags memoryPropertyFlags;
};

//}	// namespace lve
