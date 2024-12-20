#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include <string>
//namespace lve {

class VulkanWindow {
	public:
		VulkanWindow(int w, int h, const char *name);
		~VulkanWindow();

		VulkanWindow(const VulkanWindow &) = delete;
		VulkanWindow &operator=(const VulkanWindow &) = delete;

		//bool shouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
		SDL_Window *getSDL_Window() const { return window; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

	private:
		static void framebufferResizeCallback(SDL_Window *window, int width, int height);
		void initWindow();

		int width;
		int height;
		bool framebufferResized = false;

		const char *windowName;
		SDL_Window *window;
};

//}  // namespace lve
