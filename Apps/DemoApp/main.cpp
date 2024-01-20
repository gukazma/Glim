#include "SDL.h"
#include "SDL_vulkan.h"
#include <iostream>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <Glim/Tools.h>

static char const* AppName    = "05_InitSwapchainRAII";
static char const* EngineName = "Vulkan.hpp";

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("sandbox",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          1024,
                                          720,
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
   /* glim::Init(
        extensions,
        [&](vk::Instance instance) {
            VkSurfaceKHR surface;
            if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
                throw std::runtime_error("can't create surface");
            }
            return surface;
        },
        1024,
        720);*/

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
