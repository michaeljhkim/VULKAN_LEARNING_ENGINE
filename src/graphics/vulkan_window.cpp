#include "vulkan_window.hpp"

// std
#include <stdexcept>
#include <iostream>

//namespace lve {

VulkanWindow::VulkanWindow(int w, int h, const char *name) : width{w}, height{h}, windowName{name} {
    initWindow();
}

VulkanWindow::~VulkanWindow() {
    SDL_DestroyWindow(window);
    SDL_Quit(); 
}

void VulkanWindow::initWindow() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL failed to initialize: " << SDL_GetError() << std::endl;
        exit(1);
    }

    window = SDL_CreateWindow(windowName,    /* Title of the SDL window */
                    SDL_WINDOWPOS_UNDEFINED,    /* Position x of the window */
                    SDL_WINDOWPOS_UNDEFINED,    /* Position y of the window */
                    width, height,     /* Width and Height of the window in pixels */
                    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);     /* Additional flag(s) */

    //glfwSetWindowUserPointer(window, this);
    //glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void VulkanWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
    if (SDL_Vulkan_CreateSurface(window, instance, surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
}

void VulkanWindow::framebufferResizeCallback(SDL_Window *window, int width, int height) {
    /*
    auto lveWindow = reinterpret_cast<VulkanWindow *>(glfwGetWindowUserPointer(window));
    lveWindow->framebufferResized = true;
    lveWindow->width = width;
    lveWindow->height = height;
    */
}

//}  // namespace lve
