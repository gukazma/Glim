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
}
}
}