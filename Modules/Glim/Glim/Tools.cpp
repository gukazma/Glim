#include "Tools.h"
#include <iostream>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

std::vector<std::string> getDeviceExtensions()
{
    return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

vk::SurfaceFormatKHR pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats)
{
    assert(!formats.empty());
    vk::SurfaceFormatKHR pickedFormat = formats[0];
    if (formats.size() == 1) {
        if (formats[0].format == vk::Format::eUndefined) {
            pickedFormat.format     = vk::Format::eB8G8R8A8Unorm;
            pickedFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        }
    }
    else {
        // request several formats, the first found will be used
        vk::Format        requestedFormats[]  = {vk::Format::eB8G8R8A8Unorm,
                                                 vk::Format::eR8G8B8A8Unorm,
                                                 vk::Format::eB8G8R8Unorm,
                                                 vk::Format::eR8G8B8Unorm};
        vk::ColorSpaceKHR requestedColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        for (size_t i = 0; i < sizeof(requestedFormats) / sizeof(requestedFormats[0]); i++) {
            vk::Format requestedFormat = requestedFormats[i];
            auto       it              = std::find_if(
                formats.begin(),
                formats.end(),
                [requestedFormat, requestedColorSpace](vk::SurfaceFormatKHR const& f) {
                    return (f.format == requestedFormat) && (f.colorSpace == requestedColorSpace);
                });
            if (it != formats.end()) {
                pickedFormat = *it;
                break;
            }
        }
    }
    assert(pickedFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
    return pickedFormat;
}

vk::PresentModeKHR pickPresentMode(std::vector<vk::PresentModeKHR> const& presentModes)
{
    vk::PresentModeKHR pickedMode = vk::PresentModeKHR::eFifo;
    for (const auto& presentMode : presentModes) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            pickedMode = presentMode;
            break;
        }

        if (presentMode == vk::PresentModeKHR::eImmediate) {
            pickedMode = presentMode;
        }
    }
    return pickedMode;
}

uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties,
                        uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask)
{
    uint32_t typeIndex = uint32_t(~0);
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeBits & 1) && ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) ==
                               requirementsMask)) {
            typeIndex = i;
            break;
        }
        typeBits >>= 1;
    }
    assert(typeIndex != uint32_t(~0));
    return typeIndex;
}

vk::DeviceMemory allocateDeviceMemory(vk::Device const&                         device,
                                      vk::PhysicalDeviceMemoryProperties const& memoryProperties,
                                      vk::MemoryRequirements const&             memoryRequirements,
                                      vk::MemoryPropertyFlags                   memoryPropertyFlags)
{
    uint32_t memoryTypeIndex =
        findMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

    return device.allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, memoryTypeIndex));
}

glm::mat4x4 createModelViewProjectionClipMatrix(vk::Extent2D const& extent)
{
    float fov = glm::radians(45.0f);
    if (extent.width > extent.height) {
        fov *= static_cast<float>(extent.height) / static_cast<float>(extent.width);
    }

    glm::mat4x4 model = glm::mat4x4(1.0f);
    glm::mat4x4 view  = glm::lookAt(
        glm::vec3(-5.0f, 3.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    glm::mat4x4 projection = glm::perspective(fov, 1.0f, 0.1f, 100.0f);
    // clang-format off
      glm::mat4x4 clip = glm::mat4x4( 1.0f,  0.0f, 0.0f, 0.0f,
                                      0.0f, -1.0f, 0.0f, 0.0f,
                                      0.0f,  0.0f, 0.5f, 0.0f,
                                      0.0f,  0.0f, 0.5f, 1.0f );  // vulkan clip space has inverted y and half z !
      // clang-format on 
      return clip * projection * view * model;
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

}   // namespace su
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
vk::raii::Device makeDevice(vk::raii::PhysicalDevice const& physicalDevice,
                            uint32_t queueFamilyIndex, std::vector<std::string> const& extensions,
                            vk::PhysicalDeviceFeatures const* physicalDeviceFeatures,
                            void const*                       pNext)
{
    std::vector<char const*> enabledExtensions;
    enabledExtensions.reserve(extensions.size());
    for (auto const& ext : extensions) {
        enabledExtensions.push_back(ext.data());
    }

    float                     queuePriority = 0.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo(
        vk::DeviceQueueCreateFlags(), queueFamilyIndex, 1, &queuePriority);
    vk::DeviceCreateInfo deviceCreateInfo(vk::DeviceCreateFlags(),
                                          deviceQueueCreateInfo,
                                          {},
                                          enabledExtensions,
                                          physicalDeviceFeatures,
                                          pNext);
    return vk::raii::Device(physicalDevice, deviceCreateInfo);
}
vk::raii::CommandBuffer makeCommandBuffer(vk::raii::Device const&      device,
                                          vk::raii::CommandPool const& commandPool)
{
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
        *commandPool, vk::CommandBufferLevel::ePrimary, 1);
    return std::move(vk::raii::CommandBuffers(device, commandBufferAllocateInfo).front());
}
SwapChainData::SwapChainData(vk::raii::PhysicalDevice const& physicalDevice,
                             vk::raii::Device const& device, vk::raii::SurfaceKHR const& surface,
                             vk::Extent2D const& extent, vk::ImageUsageFlags usage,
                             vk::raii::SwapchainKHR const* pOldSwapchain,
                             uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex)
{
    vk::SurfaceFormatKHR surfaceFormat =
        vk::su::pickSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface));
    colorFormat = surfaceFormat.format;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities =
        physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    vk::Extent2D swapchainExtent;
    if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
        // If the surface size is undefined, the size is set to the size of the images requested.
        swapchainExtent.width  = vk::su::clamp(extent.width,
                                              surfaceCapabilities.minImageExtent.width,
                                              surfaceCapabilities.maxImageExtent.width);
        swapchainExtent.height = vk::su::clamp(extent.height,
                                               surfaceCapabilities.minImageExtent.height,
                                               surfaceCapabilities.maxImageExtent.height);
    }
    else {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfaceCapabilities.currentExtent;
    }
    vk::SurfaceTransformFlagBitsKHR preTransform =
        (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
            ? vk::SurfaceTransformFlagBitsKHR::eIdentity
            : surfaceCapabilities.currentTransform;
    vk::CompositeAlphaFlagBitsKHR compositeAlpha =
        (surfaceCapabilities.supportedCompositeAlpha &
         vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
            ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
        : (surfaceCapabilities.supportedCompositeAlpha &
           vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
            ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
        : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
            ? vk::CompositeAlphaFlagBitsKHR::eInherit
            : vk::CompositeAlphaFlagBitsKHR::eOpaque;
    vk::PresentModeKHR presentMode =
        vk::su::pickPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface));

    vk::SwapchainCreateInfoKHR swapChainCreateInfo;
    swapChainCreateInfo.setSurface(*surface);
    swapChainCreateInfo.setMinImageCount(
        vk::su::clamp(3u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount));
    swapChainCreateInfo.setImageFormat(colorFormat);
    swapChainCreateInfo.setImageColorSpace(surfaceFormat.colorSpace);
    swapChainCreateInfo.setImageExtent(swapchainExtent);
    swapChainCreateInfo.setImageArrayLayers(1);
    swapChainCreateInfo.setImageUsage(usage);
    swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    swapChainCreateInfo.setPreTransform(preTransform);
    swapChainCreateInfo.setCompositeAlpha(compositeAlpha);
    swapChainCreateInfo.setPresentMode(presentMode);
    swapChainCreateInfo.setClipped(true);
    swapChainCreateInfo.setOldSwapchain(pOldSwapchain ? **pOldSwapchain : nullptr);

    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
        uint32_t queueFamilyIndices[2] = {graphicsQueueFamilyIndex, presentQueueFamilyIndex};
        // If the graphics and present queues are from different queue families, we either have to
        // explicitly transfer ownership of images between the queues, or we have to create the
        // swapchain with imageSharingMode as vk::SharingMode::eConcurrent
        swapChainCreateInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
    }
    swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);

    images = swapChain.getImages();

    imageViews.reserve(images.size());
    vk::ImageViewCreateInfo imageViewCreateInfo({},
                                                {},
                                                vk::ImageViewType::e2D,
                                                colorFormat,
                                                {},
                                                {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    for (auto image : images) {
        imageViewCreateInfo.image = image;
        imageViews.emplace_back(device, imageViewCreateInfo);
    }
}
vk::raii::DeviceMemory allocateDeviceMemory(
    vk::raii::Device const& device, vk::PhysicalDeviceMemoryProperties const& memoryProperties,
    vk::MemoryRequirements const& memoryRequirements, vk::MemoryPropertyFlags memoryPropertyFlags)
{
    uint32_t memoryTypeIndex = vk::su::findMemoryType(
        memoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags);
    vk::MemoryAllocateInfo memoryAllocateInfo(memoryRequirements.size, memoryTypeIndex);
    return vk::raii::DeviceMemory(device, memoryAllocateInfo);
}
ImageData::ImageData(vk::raii::PhysicalDevice const& physicalDevice, vk::raii::Device const& device,
                     vk::Format format_, vk::Extent2D const& extent, vk::ImageTiling tiling,
                     vk::ImageUsageFlags usage, vk::ImageLayout initialLayout,
                     vk::MemoryPropertyFlags memoryProperties, vk::ImageAspectFlags aspectMask)
    : format(format_)
    , image(device, {vk::ImageCreateFlags(),
                     vk::ImageType::e2D,
                     format,
                     vk::Extent3D(extent, 1),
                     1,
                     1,
                     vk::SampleCountFlagBits::e1,
                     tiling,
                     usage | vk::ImageUsageFlagBits::eSampled,
                     vk::SharingMode::eExclusive,
                     {},
                     initialLayout})
{
    deviceMemory = vk::raii::su::allocateDeviceMemory(device,
                                                      physicalDevice.getMemoryProperties(),
                                                      image.getMemoryRequirements(),
                                                      memoryProperties);
    image.bindMemory(*deviceMemory, 0);
    imageView = vk::raii::ImageView(
        device,
        vk::ImageViewCreateInfo(
            {}, *image, vk::ImageViewType::e2D, format, {}, {aspectMask, 0, 1, 0, 1}));
}

vk::raii::DescriptorSetLayout makeDescriptorSetLayout(
    vk::raii::Device const&                                                            device,
    std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> const& bindingData,
    vk::DescriptorSetLayoutCreateFlags                                                 flags)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings( bindingData.size() );
        for ( size_t i = 0; i < bindingData.size(); i++ )
        {
          bindings[i] = vk::DescriptorSetLayoutBinding(
            vk::su::checked_cast<uint32_t>( i ), std::get<0>( bindingData[i] ), std::get<1>( bindingData[i] ), std::get<2>( bindingData[i] ) );
        }
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo( flags, bindings );
        return vk::raii::DescriptorSetLayout( device, descriptorSetLayoutCreateInfo );
}
}   // namespace su
}   // namespace raii
}   // namespace vk