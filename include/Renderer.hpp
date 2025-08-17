#pragma once

#include <vector>
#include <optional>
#include <string>
#include <cstdint>
#include <vulkan/vulkan.h>

struct GLFWwindow; // fwd

namespace rtvk {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Renderer {
public:
    Renderer(uint32_t width, uint32_t height, const std::string &title);
    ~Renderer();

    void mainLoop();

private:
    void initWindow(uint32_t width, uint32_t height, const std::string &title);
    void initVulkan();
    void createInstance();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void setupDebugMessenger();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void drawFrame();
    void cleanupSwapChain();
    void recreateSwapChain();

private:
    GLFWwindow* window_{};

    VkInstance instance_{};
    VkDebugUtilsMessengerEXT debugMessenger_{};
    VkSurfaceKHR surface_{};
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_{};

    VkQueue graphicsQueue_{};
    VkQueue presentQueue_{};

    VkSwapchainKHR swapChain_{};
    std::vector<VkImage> swapChainImages_;
    VkFormat swapChainImageFormat_{};
    VkExtent2D swapChainExtent_{};
    std::vector<VkImageView> swapChainImageViews_;

    VkRenderPass renderPass_{};
    VkPipelineLayout pipelineLayout_{};
    VkPipeline graphicsPipeline_{};

    std::vector<VkFramebuffer> swapChainFramebuffers_;

    VkCommandPool commandPool_{};
    std::vector<VkCommandBuffer> commandBuffers_;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    std::vector<VkFence> inFlightFences_;
    size_t currentFrame_ = 0;

    bool framebufferResized_ = false;
};

} // namespace rtvk
