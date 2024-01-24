#include "Tools.h"
#include <iostream>
namespace vk {
namespace su {

std::vector<char const*> gatherLayers(std::vector<std::string> const& layers
#if !defined(NDEBUG)
                                      ,
                                      std::vector<vk::LayerProperties> const& layerProperties
#endif
)
{
    std::vector<char const*> enabledLayers;
    enabledLayers.reserve(layers.size());
    for (auto const& layer : layers) {
        assert(std::any_of(
            layerProperties.begin(), layerProperties.end(), [layer](vk::LayerProperties const& lp) {
                return layer == lp.layerName;
            }));
        enabledLayers.push_back(layer.data());
    }
#if !defined(NDEBUG)
    // Enable standard validation layer to find as much errors as possible!
    if (std::none_of(
            layers.begin(),
            layers.end(),
            [](std::string const& layer) { return layer == "VK_LAYER_KHRONOS_validation"; }) &&
        std::any_of(
            layerProperties.begin(), layerProperties.end(), [](vk::LayerProperties const& lp) {
                return (strcmp("VK_LAYER_KHRONOS_validation", lp.layerName) == 0);
            })) {
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
#endif
    return enabledLayers;
}

std::vector<char const*> gatherExtensions(
    std::vector<std::string> const& extensions
#if !defined(NDEBUG)
    ,
    std::vector<vk::ExtensionProperties> const& extensionProperties
#endif
)
{
    std::vector<char const*> enabledExtensions;
    enabledExtensions.reserve(extensions.size());
    for (auto const& ext : extensions) {
        assert(std::any_of(
            extensionProperties.begin(),
            extensionProperties.end(),
            [ext](vk::ExtensionProperties const& ep) { return ext == ep.extensionName; }));
        enabledExtensions.push_back(ext.data());
    }
#if !defined(NDEBUG)
    if (std::none_of(extensions.begin(),
                     extensions.end(),
                     [](std::string const& extension) {
                         return extension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
                     }) &&
        std::any_of(extensionProperties.begin(),
                    extensionProperties.end(),
                    [](vk::ExtensionProperties const& ep) {
                        return (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, ep.extensionName) == 0);
                    })) {
        enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#endif
    return enabledExtensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void* /*pUserData*/)
{
#if !defined(NDEBUG)
    if (static_cast<uint32_t>(pCallbackData->messageIdNumber) == 0x822806fa) {
        // Validation Warning: vkCreateInstance(): to enable extension VK_EXT_debug_utils, but this
        // extension is intended to support use by applications when debugging and it is strongly
        // recommended that it be otherwise avoided.
        return vk::False;
    }
    else if (static_cast<uint32_t>(pCallbackData->messageIdNumber) == 0xe8d1a9fe) {
        // Validation Performance Warning: Using debug builds of the validation layers *will*
        // adversely affect performance.
        return vk::False;
    }
#endif

    std::cerr << vk::to_string(
                     static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity))
              << ": " << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes))
              << ":\n";
    std::cerr << std::string("\t") << "messageIDName   = <" << pCallbackData->pMessageIdName
              << ">\n";
    std::cerr << std::string("\t") << "messageIdNumber = " << pCallbackData->messageIdNumber
              << "\n";
    std::cerr << std::string("\t") << "message         = <" << pCallbackData->pMessage << ">\n";
    if (0 < pCallbackData->queueLabelCount) {
        std::cerr << std::string("\t") << "Queue Labels:\n";
        for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++) {
            std::cerr << std::string("\t\t") << "labelName = <"
                      << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
        }
    }
    if (0 < pCallbackData->cmdBufLabelCount) {
        std::cerr << std::string("\t") << "CommandBuffer Labels:\n";
        for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
            std::cerr << std::string("\t\t") << "labelName = <"
                      << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
        }
    }
    if (0 < pCallbackData->objectCount) {
        std::cerr << std::string("\t") << "Objects:\n";
        for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
            std::cerr << std::string("\t\t") << "Object " << i << "\n";
            std::cerr << std::string("\t\t\t") << "objectType   = "
                      << vk::to_string(
                             static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType))
                      << "\n";
            std::cerr << std::string("\t\t\t")
                      << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
            if (pCallbackData->pObjects[i].pObjectName) {
                std::cerr << std::string("\t\t\t") << "objectName   = <"
                          << pCallbackData->pObjects[i].pObjectName << ">\n";
            }
        }
    }
    return vk::False;
}

vk::DebugUtilsMessengerCreateInfoEXT makeDebugUtilsMessengerCreateInfoEXT()
{
    return {{},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
            &vk::su::debugUtilsMessengerCallback};
}

uint32_t findGraphicsQueueFamilyIndex(
    std::vector<vk::QueueFamilyProperties> const& queueFamilyProperties)
{
    // get the first index into queueFamiliyProperties which supports graphics
    std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty =
        std::find_if(queueFamilyProperties.begin(),
                     queueFamilyProperties.end(),
                     [](vk::QueueFamilyProperties const& qfp) {
                         return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
                     });
    assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
    return static_cast<uint32_t>(
        std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
}


#if defined(NDEBUG)
vk::StructureChain<vk::InstanceCreateInfo>
#else
vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
#endif
makeInstanceCreateInfoChain(vk::ApplicationInfo const&      applicationInfo,
                            std::vector<char const*> const& layers,
                            std::vector<char const*> const& extensions)
{
#if defined(NDEBUG)
    // in non-debug mode just use the InstanceCreateInfo for instance creation
    vk::StructureChain<vk::InstanceCreateInfo> instanceCreateInfo(
        {{}, &applicationInfo, layers, extensions});
#else
    // in debug mode, addionally use the debugUtilsMessengerCallback in instance creation!
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
        instanceCreateInfo(
            {{}, &applicationInfo, layers, extensions},
            {{}, severityFlags, messageTypeFlags, &vk::su::debugUtilsMessengerCallback});
#endif
    return instanceCreateInfo;
}
}
}   // namespace vk


namespace vk {
namespace raii {
namespace su {
vk::raii::Instance makeInstance(vk::raii::Context const& context, std::string const& appName,
    std::string const& engineName,
    std::vector<std::string> const& layers,
    std::vector<std::string> const& extensions,
    uint32_t                        apiVersion)
{
    vk::ApplicationInfo      applicationInfo(appName.c_str(), 1, engineName.c_str(), 1, apiVersion);
    std::vector<char const*> enabledLayers =
        vk::su::gatherLayers(layers
#if !defined(NDEBUG)
                             ,
                             context.enumerateInstanceLayerProperties()
#endif
        );
    std::vector<char const*> enabledExtensions =
        vk::su::gatherExtensions(extensions
#if !defined(NDEBUG)
                                 ,
                                 context.enumerateInstanceExtensionProperties()
#endif
        );
#if defined(NDEBUG)
    vk::StructureChain<vk::InstanceCreateInfo>
#else
    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
#endif
        instanceCreateInfoChain =
            vk::su::makeInstanceCreateInfoChain(applicationInfo, enabledLayers, enabledExtensions);

    return vk::raii::Instance(context, instanceCreateInfoChain.get<vk::InstanceCreateInfo>());
}
std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(
    vk::raii::PhysicalDevice const& physicalDevice, vk::raii::SurfaceKHR const& surface)
{
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
        physicalDevice.getQueueFamilyProperties();
    assert(queueFamilyProperties.size() < std::numeric_limits<uint32_t>::max());

    uint32_t graphicsQueueFamilyIndex = vk::su::findGraphicsQueueFamilyIndex(queueFamilyProperties);
    if (physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex, *surface)) {
        return std::make_pair(graphicsQueueFamilyIndex,
                              graphicsQueueFamilyIndex);   // the first graphicsQueueFamilyIndex
                                                           // does also support presents
    }

    // the graphicsQueueFamilyIndex doesn't support present -> look for an other family index that
    // supports both graphics and present
    for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
        if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
            physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface)) {
            return std::make_pair(static_cast<uint32_t>(i), static_cast<uint32_t>(i));
        }
    }

    // there's nothing like a single family index that supports both graphics and present -> look
    // for an other family index that supports present
    for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
        if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface)) {
            return std::make_pair(graphicsQueueFamilyIndex, static_cast<uint32_t>(i));
        }
    }

    throw std::runtime_error("Could not find queues for both graphics or present -> terminating");
}
}
}   // namespace raii
}   // namespace vk