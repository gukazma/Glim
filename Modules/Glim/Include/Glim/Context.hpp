#pragma once
#include <cassert>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <optional>
#include "Tool.hpp"
#include "Swapchain.hpp"
namespace glim {
class Context final
{
public:
    static void Init(const std::vector<const char*>& extensions, CreateSurfaceFunc func);
    static void Quit();

    static Context& GetInstance()
    {
        assert(instance_);
        return *instance_;
    }

    ~Context();

    struct QueueFamliyIndices final {
        std::optional<uint32_t> graphicsQueue;
        std::optional<uint32_t> presentQueue;

        operator bool() const {
            return graphicsQueue.has_value() && presentQueue.has_value();
        }
    };

    vk::Instance               instance;
    vk::PhysicalDevice         phyDevice;
    vk::Device                 device;
    vk::Queue                  graphcisQueue;
    vk::Queue                  presentQueue;
    vk::SurfaceKHR             surface;
    std::unique_ptr<Swapchain> swapchain;
    QueueFamliyIndices         queueFamilyIndices;

    void InitSwapchain(int w, int h) { swapchain.reset(new Swapchain(w, h)); }

    void DestroySwapchain() { swapchain.reset(); }

private:
    static std::unique_ptr<Context> instance_;

    Context(const std::vector<const char*>& extensions, CreateSurfaceFunc func);

    void createInstance(const std::vector<const char*>& extensions);
    void pickupPhyiscalDevice();
    void createDevice();
    void queryQueueFamilyIndices();
    void getQueues();
};
}   // namespace glim