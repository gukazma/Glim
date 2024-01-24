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
uint32_t                 findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties,
                                        uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask);
vk::DeviceMemory         allocateDeviceMemory(vk::Device const&                         device,
                                              vk::PhysicalDeviceMemoryProperties const& memoryProperties,
                                              vk::MemoryRequirements const&             memoryRequirements,
                                              vk::MemoryPropertyFlags memoryPropertyFlags);

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

template<typename Func>
void oneTimeSubmit(vk::raii::Device const& device, vk::raii::CommandPool const& commandPool,
                   vk::raii::Queue const& queue, Func const& func)
{
    vk::raii::CommandBuffer commandBuffer = std::move(
        vk::raii::CommandBuffers(device, {*commandPool, vk::CommandBufferLevel::ePrimary, 1})
            .front());
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    func(commandBuffer);
    commandBuffer.end();
    vk::SubmitInfo submitInfo(nullptr, nullptr, *commandBuffer);
    queue.submit(submitInfo, nullptr);
    queue.waitIdle();
}

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
vk::raii::DeviceMemory allocateDeviceMemory(
    vk::raii::Device const& device, vk::PhysicalDeviceMemoryProperties const& memoryProperties,
    vk::MemoryRequirements const& memoryRequirements, vk::MemoryPropertyFlags memoryPropertyFlags);
struct BufferData
{
    BufferData(vk::raii::PhysicalDevice const& physicalDevice, vk::raii::Device const& device,
               vk::DeviceSize size, vk::BufferUsageFlags usage,
               vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                       vk::MemoryPropertyFlagBits::eHostCoherent)
        : buffer(device, vk::BufferCreateInfo({}, size, usage))
#if !defined(NDEBUG)
        , m_size(size)
        , m_usage(usage)
        , m_propertyFlags(propertyFlags)
#endif
    {
        deviceMemory = vk::raii::su::allocateDeviceMemory(device,
                                                          physicalDevice.getMemoryProperties(),
                                                          buffer.getMemoryRequirements(),
                                                          propertyFlags);
        buffer.bindMemory(*deviceMemory, 0);
    }

    BufferData(std::nullptr_t) {}

    template<typename DataType> void upload(DataType const& data) const
    {
        assert((m_propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent) &&
               (m_propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible));
        assert(sizeof(DataType) <= m_size);

        void* dataPtr = deviceMemory.mapMemory(0, sizeof(DataType));
        memcpy(dataPtr, &data, sizeof(DataType));
        deviceMemory.unmapMemory();
    }

    template<typename DataType>
    void upload(std::vector<DataType> const& data, size_t stride = 0) const
    {
        assert(m_propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible);

        size_t elementSize = stride ? stride : sizeof(DataType);
        assert(sizeof(DataType) <= elementSize);

        copyToDevice(deviceMemory, data.data(), data.size(), elementSize);
    }

    template<typename DataType>
    void upload(vk::raii::PhysicalDevice const& physicalDevice, vk::raii::Device const& device,
                vk::raii::CommandPool const& commandPool, vk::raii::Queue const& queue,
                std::vector<DataType> const& data, size_t stride) const
    {
        assert(m_usage & vk::BufferUsageFlagBits::eTransferDst);
        assert(m_propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal);

        size_t elementSize = stride ? stride : sizeof(DataType);
        assert(sizeof(DataType) <= elementSize);

        size_t dataSize = data.size() * elementSize;
        assert(dataSize <= m_size);

        vk::raii::su::BufferData stagingBuffer(
            physicalDevice, device, dataSize, vk::BufferUsageFlagBits::eTransferSrc);
        copyToDevice(stagingBuffer.deviceMemory, data.data(), data.size(), elementSize);

        vk::raii::su::oneTimeSubmit(
            device, commandPool, queue, [&](vk::raii::CommandBuffer const& commandBuffer) {
                commandBuffer.copyBuffer(
                    *stagingBuffer.buffer, *this->buffer, vk::BufferCopy(0, 0, dataSize));
            });
    }

    // the DeviceMemory should be destroyed before the Buffer it is bound to; to get that order with
    // the standard destructor of the BufferData, the order of DeviceMemory and Buffer here matters
    vk::raii::DeviceMemory deviceMemory = nullptr;
    vk::raii::Buffer       buffer       = nullptr;
#if !defined(NDEBUG)
private:
    vk::DeviceSize          m_size;
    vk::BufferUsageFlags    m_usage;
    vk::MemoryPropertyFlags m_propertyFlags;
#endif
};
struct ImageData
{
    ImageData(vk::raii::PhysicalDevice const& physicalDevice, vk::raii::Device const& device,
              vk::Format format_, vk::Extent2D const& extent, vk::ImageTiling tiling,
              vk::ImageUsageFlags usage, vk::ImageLayout initialLayout,
              vk::MemoryPropertyFlags memoryProperties, vk::ImageAspectFlags aspectMask);

    ImageData(std::nullptr_t) {}

    // the DeviceMemory should be destroyed before the Image it is bound to; to get that order with
    // the standard destructor of the ImageData, the order of DeviceMemory and Image here matters
    vk::Format             format;
    vk::raii::DeviceMemory deviceMemory = nullptr;
    vk::raii::Image        image        = nullptr;
    vk::raii::ImageView    imageView    = nullptr;
};

struct DepthBufferData : public ImageData
{
    DepthBufferData(vk::raii::PhysicalDevice const& physicalDevice, vk::raii::Device const& device,
                    vk::Format format, vk::Extent2D const& extent)
        : ImageData(physicalDevice, device, format, extent, vk::ImageTiling::eOptimal,
                    vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eUndefined,
                    vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eDepth)
    {}
};

}
}
}