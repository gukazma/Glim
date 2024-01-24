#include <vulkan/vulkan_raii.hpp>

namespace vk
{
namespace su
{

std::vector<char const*> gatherLayers(std::vector<std::string> const& layers
#if !defined(NDEBUG)
                                      ,
                                      std::vector<vk::LayerProperties> const& layerProperties
#endif
);

std::vector<char const*> gatherExtensions(
    std::vector<std::string> const& extensions
#if !defined(NDEBUG)
    ,
    std::vector<vk::ExtensionProperties> const& extensionProperties
#endif
);
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void* /*pUserData*/);
vk::DebugUtilsMessengerCreateInfoEXT makeDebugUtilsMessengerCreateInfoEXT();
uint32_t findGraphicsQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const& queueFamilyProperties);
std::vector<std::string> getDeviceExtensions();
vk::SurfaceFormatKHR     pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats);
vk::PresentModeKHR       pickPresentMode(std::vector<vk::PresentModeKHR> const& presentModes);
template<class T> VULKAN_HPP_INLINE constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
    return v < lo ? lo : hi < v ? hi : v;
}
}
}

namespace vk
{
namespace raii
{
namespace su
{
vk::raii::Instance makeInstance(vk::raii::Context const& context, std::string const& appName,
                                std::string const&              engineName,
                                std::vector<std::string> const& layers     = {},
                                std::vector<std::string> const& extensions = {},
                                uint32_t                        apiVersion = VK_API_VERSION_1_0);
std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(
    vk::raii::PhysicalDevice const& physicalDevice, vk::raii::SurfaceKHR const& surface);
vk::raii::Device makeDevice(vk::raii::PhysicalDevice const&   physicalDevice,
                            uint32_t                          queueFamilyIndex,
                            std::vector<std::string> const&   extensions             = {},
                            vk::PhysicalDeviceFeatures const* physicalDeviceFeatures = nullptr,
                            void const*                       pNext                  = nullptr);
vk::raii::CommandBuffer makeCommandBuffer(vk::raii::Device const&      device,
                                          vk::raii::CommandPool const& commandPool);
struct SwapChainData
{
    SwapChainData(vk::raii::PhysicalDevice const& physicalDevice, vk::raii::Device const& device,
                  vk::raii::SurfaceKHR const& surface, vk::Extent2D const& extent,
                  vk::ImageUsageFlags usage, vk::raii::SwapchainKHR const* pOldSwapchain,
                  uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex);

    vk::Format                       colorFormat;
    vk::raii::SwapchainKHR           swapChain = nullptr;
    std::vector<vk::Image>           images;
    std::vector<vk::raii::ImageView> imageViews;
};
}
}
}