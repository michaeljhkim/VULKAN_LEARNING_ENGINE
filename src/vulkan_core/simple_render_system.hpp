#pragma once

#include "lve_camera.hpp"
#include "vulkan_device.hpp"
#include "vulkan_frame_info.hpp"
#include "lve_game_object.hpp"
#include "vulkan_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class SimpleRenderSystem {
 public:
  SimpleRenderSystem(
      VulkanDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void renderGameObjects(FrameInfo &frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  VulkanDevice &vulkanDevice;

  std::unique_ptr<VulkanPipeline> vulkanPipeline;
  VkPipelineLayout pipelineLayout;
};
}  // namespace lve
