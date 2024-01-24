#include "SDL.h"
#include "SDL_vulkan.h"
#include <iostream>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <Glim/Tools.h>
#include <glm/glm.hpp>
static char const* AppName    = "05_InitSwapchainRAII";
static char const* EngineName = "Vulkan.hpp";

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    uint32_t         width = 1024, height = 720;
    SDL_Window* window = SDL_CreateWindow("sandbox",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          width,
                                          height,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
    if (!window) {
        SDL_Log("create window failed");
        exit(2);
    }
    bool      shouldClose = false;
    SDL_Event event;

    unsigned int count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> extensions_sdl(count);
    std::vector<std::string> extensions;
    SDL_Vulkan_GetInstanceExtensions(window, &count, extensions_sdl.data());
    for (size_t i = 0; i < extensions_sdl.size(); i++) {
        extensions.push_back(std::string(extensions_sdl[i]));
    }

    vk::raii::Context  context;
    vk::raii::Instance instance =
        vk::raii::su::makeInstance(context, AppName, EngineName, {}, extensions);
#if !defined(NDEBUG)
    vk::raii::DebugUtilsMessengerEXT debugUtilsMessenger(
        instance, vk::su::makeDebugUtilsMessengerCreateInfoEXT());
#endif
    vk::raii::PhysicalDevice physicalDevice = vk::raii::PhysicalDevices(instance).front();
    VkSurfaceKHR             _surface;
    if (!SDL_Vulkan_CreateSurface(window, *instance, &_surface)) {
        throw std::runtime_error("can't create surface");
    }
    vk::raii::SurfaceKHR surface(instance, _surface);

    std::pair<uint32_t, uint32_t> graphicsAndPresentQueueFamilyIndex =
        vk::raii::su::findGraphicsAndPresentQueueFamilyIndex(physicalDevice, surface);
    vk::raii::Device device = vk::raii::su::makeDevice(
        physicalDevice, graphicsAndPresentQueueFamilyIndex.first, vk::su::getDeviceExtensions());
    vk::raii::CommandPool commandPool =
        vk::raii::CommandPool(device, {{}, graphicsAndPresentQueueFamilyIndex.first});
    vk::raii::CommandBuffer commandBuffer = vk::raii::su::makeCommandBuffer(device, commandPool);
    vk::raii::Queue         graphicsQueue(device, graphicsAndPresentQueueFamilyIndex.first, 0);
    vk::raii::Queue         presentQueue(device, graphicsAndPresentQueueFamilyIndex.second, 0);
    vk::Extent2D                extent = {width, height};
    vk::raii::su::SwapChainData swapChainData(
        physicalDevice,
        device,
        surface,
        extent,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
        {},
        graphicsAndPresentQueueFamilyIndex.first,
        graphicsAndPresentQueueFamilyIndex.second);

    vk::raii::su::DepthBufferData depthBufferData(
        physicalDevice, device, vk::Format::eD16Unorm, extent);
    vk::raii::su::BufferData uniformBufferData(
        physicalDevice, device, sizeof(glm::mat4x4), vk::BufferUsageFlagBits::eUniformBuffer);

    glm::mat4x4 mvpcMatrix = vk::su::createModelViewProjectionClipMatrix(extent);
    vk::raii::su::copyToDevice(uniformBufferData.deviceMemory, mvpcMatrix);
    while (!shouldClose) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldClose = true;
            }
        }
    }


    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
