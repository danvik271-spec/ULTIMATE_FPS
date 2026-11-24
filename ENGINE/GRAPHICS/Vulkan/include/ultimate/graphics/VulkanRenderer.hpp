#pragma once
#include <cstdint>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace ultimate::graphics {
struct RendererConfig { std::uint32_t width{1280}; std::uint32_t height{720}; std::string title{"ULTIMATE FPS - Vulkan Runtime"}; };
class VulkanRenderer {
public:
    VulkanRenderer() = default;
    ~VulkanRenderer();
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
    bool initialize(const RendererConfig& config, std::string* error = nullptr);
    void pollEvents() const;
    void drawFrame();
    [[nodiscard]] bool shouldClose() const;
    void shutdown() noexcept;
private:
    struct QueueFamilies { std::uint32_t graphics{UINT32_MAX}; std::uint32_t present{UINT32_MAX}; [[nodiscard]] bool complete() const noexcept { return graphics != UINT32_MAX && present != UINT32_MAX; } };
    struct SwapSupport { VkSurfaceCapabilitiesKHR capabilities{}; std::vector<VkSurfaceFormatKHR> formats; std::vector<VkPresentModeKHR> modes; };
    bool createInstance(std::string* error);
    bool createSurface(std::string* error);
    bool selectPhysicalDevice(std::string* error);
    bool createLogicalDevice(std::string* error);
    bool createSwapchain(std::string* error);
    bool createImageViews(std::string* error);
    bool createRenderPass(std::string* error);
    bool createPipeline(std::string* error);
    bool createFramebuffers(std::string* error);
    bool createCommandPool(std::string* error);
    bool createCommandBuffers(std::string* error);
    bool createSyncObjects(std::string* error);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, std::uint32_t imageIndex);
    void recreateSwapchain();
    void cleanupSwapchain() noexcept;
    [[nodiscard]] QueueFamilies findQueueFamilies(VkPhysicalDevice device) const;
    [[nodiscard]] SwapSupport querySwapSupport(VkPhysicalDevice device) const;
    [[nodiscard]] bool supportsRequiredExtensions(VkPhysicalDevice device) const;
    [[nodiscard]] VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const;
    [[nodiscard]] VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes) const;
    [[nodiscard]] VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& caps) const;
    static std::vector<char> readBinaryFile(const std::string& path);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* window_{nullptr};
    VkInstance instance_{VK_NULL_HANDLE};
    VkSurfaceKHR surface_{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE};
    VkQueue graphicsQueue_{VK_NULL_HANDLE};
    VkQueue presentQueue_{VK_NULL_HANDLE};
    VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> imageViews_;
    std::vector<VkFramebuffer> framebuffers_;
    VkFormat swapchainFormat_{VK_FORMAT_UNDEFINED};
    VkExtent2D swapchainExtent_{};
    VkRenderPass renderPass_{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout_{VK_NULL_HANDLE};
    VkPipeline graphicsPipeline_{VK_NULL_HANDLE};
    VkCommandPool commandPool_{VK_NULL_HANDLE};
    std::vector<VkCommandBuffer> commandBuffers_;
    static constexpr std::size_t FramesInFlight = 2;
    VkSemaphore imageAvailable_[FramesInFlight]{};
    VkSemaphore renderFinished_[FramesInFlight]{};
    VkFence inFlight_[FramesInFlight]{};
    std::size_t currentFrame_{0};
    bool framebufferResized_{false};
};
}
