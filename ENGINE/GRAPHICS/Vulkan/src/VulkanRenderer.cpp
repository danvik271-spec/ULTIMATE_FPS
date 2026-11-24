#include "ultimate/graphics/VulkanRenderer.hpp"
#include "ultimate/core/Log.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <limits>
#include <set>
#include <stdexcept>

namespace ultimate::graphics {
namespace {
constexpr const char* DeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
std::string vkMessage(const char* what, VkResult result) { return std::string(what) + " (VkResult=" + std::to_string(static_cast<int>(result)) + ")"; }
bool fail(std::string* error, std::string message) { if (error) *error = std::move(message); return false; }
}
VulkanRenderer::~VulkanRenderer() { shutdown(); }

bool VulkanRenderer::initialize(const RendererConfig& config, std::string* error) {
    if (!glfwInit()) return fail(error, "GLFW initialization failed");
    if (glfwVulkanSupported() != GLFW_TRUE) return fail(error, "GLFW reports that Vulkan is unavailable");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window_ = glfwCreateWindow(static_cast<int>(config.width), static_cast<int>(config.height), config.title.c_str(), nullptr, nullptr);
    if (!window_) return fail(error, "failed to create GLFW window");
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    if (!createInstance(error) || !createSurface(error) || !selectPhysicalDevice(error) ||
        !createLogicalDevice(error) || !createSwapchain(error) || !createImageViews(error) ||
        !createRenderPass(error) || !createPipeline(error) || !createFramebuffers(error) ||
        !createCommandPool(error) || !createCommandBuffers(error) || !createSyncObjects(error)) {
        shutdown(); return false;
    }
    return true;
}

bool VulkanRenderer::createInstance(std::string* error) {
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "ULTIMATE FPS"; app.applicationVersion = VK_MAKE_VERSION(0,1,0);
    app.pEngineName = "ULTIMATE Custom Engine"; app.engineVersion = VK_MAKE_VERSION(0,1,0);
    app.apiVersion = VK_API_VERSION_1_1;
    std::uint32_t extensionCount = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    if (!extensions || extensionCount == 0) return fail(error, "GLFW returned no Vulkan surface extensions");
    VkInstanceCreateInfo create{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create.pApplicationInfo = &app; create.enabledExtensionCount = extensionCount; create.ppEnabledExtensionNames = extensions;
#ifndef NDEBUG
    const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    std::uint32_t layerCount = 0; vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount); vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    const bool layerAvailable = std::any_of(layers.begin(), layers.end(), [&](const auto& layer) { return std::strcmp(layer.layerName, validationLayer) == 0; });
    if (layerAvailable) { create.enabledLayerCount = 1; create.ppEnabledLayerNames = &validationLayer; }
#endif
    const VkResult result = vkCreateInstance(&create, nullptr, &instance_);
    if (result != VK_SUCCESS) return fail(error, vkMessage("vkCreateInstance failed", result));
    return true;
}
bool VulkanRenderer::createSurface(std::string* error) {
    const VkResult result = glfwCreateWindowSurface(instance_, window_, nullptr, &surface_);
    return result == VK_SUCCESS || fail(error, vkMessage("glfwCreateWindowSurface failed", result));
}
VulkanRenderer::QueueFamilies VulkanRenderer::findQueueFamilies(VkPhysicalDevice device) const {
    QueueFamilies result;
    std::uint32_t count = 0; vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count); vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());
    for (std::uint32_t i = 0; i < count; ++i) {
        if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) result.graphics = i;
        VkBool32 present = VK_FALSE; vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present);
        if (present == VK_TRUE) result.present = i;
        if (result.complete()) break;
    }
    return result;
}
bool VulkanRenderer::supportsRequiredExtensions(VkPhysicalDevice device) const {
    std::uint32_t count = 0; vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count); vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data());
    for (const char* required : DeviceExtensions) {
        const bool found = std::any_of(available.begin(), available.end(), [&](const auto& e) { return std::strcmp(e.extensionName, required) == 0; });
        if (!found) return false;
    }
    return true;
}
VulkanRenderer::SwapSupport VulkanRenderer::querySwapSupport(VkPhysicalDevice device) const {
    SwapSupport s; vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &s.capabilities);
    std::uint32_t count = 0; vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &count, nullptr);
    if (count) { s.formats.resize(count); vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &count, s.formats.data()); }
    count = 0; vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &count, nullptr);
    if (count) { s.modes.resize(count); vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &count, s.modes.data()); }
    return s;
}
bool VulkanRenderer::selectPhysicalDevice(std::string* error) {
    std::uint32_t count = 0; vkEnumeratePhysicalDevices(instance_, &count, nullptr);
    if (!count) return fail(error, "no Vulkan physical devices found");
    std::vector<VkPhysicalDevice> devices(count); vkEnumeratePhysicalDevices(instance_, &count, devices.data());
    for (const auto candidate : devices) {
        const auto queues = findQueueFamilies(candidate);
        if (!queues.complete() || !supportsRequiredExtensions(candidate)) continue;
        const auto swap = querySwapSupport(candidate);
        if (swap.formats.empty() || swap.modes.empty()) continue;
        VkPhysicalDeviceFeatures features{}; vkGetPhysicalDeviceFeatures(candidate, &features);
        physicalDevice_ = candidate; return true;
    }
    return fail(error, "no suitable Vulkan device with graphics, presentation and swapchain support");
}
bool VulkanRenderer::createLogicalDevice(std::string* error) {
    const auto queues = findQueueFamilies(physicalDevice_);
    const std::set<std::uint32_t> unique{queues.graphics, queues.present};
    const float priority = 1.0F;
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    for (const auto index : unique) {
        VkDeviceQueueCreateInfo q{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO}; q.queueFamilyIndex = index; q.queueCount = 1; q.pQueuePriorities = &priority; queueInfos.push_back(q);
    }
    VkPhysicalDeviceFeatures features{};
    VkDeviceCreateInfo create{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    create.queueCreateInfoCount = static_cast<std::uint32_t>(queueInfos.size()); create.pQueueCreateInfos = queueInfos.data();
    create.pEnabledFeatures = &features; create.enabledExtensionCount = 1; create.ppEnabledExtensionNames = DeviceExtensions;
    const VkResult result = vkCreateDevice(physicalDevice_, &create, nullptr, &device_);
    if (result != VK_SUCCESS) return fail(error, vkMessage("vkCreateDevice failed", result));
    vkGetDeviceQueue(device_, queues.graphics, 0, &graphicsQueue_); vkGetDeviceQueue(device_, queues.present, 0, &presentQueue_);
    return true;
}
VkSurfaceFormatKHR VulkanRenderer::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const {
    for (const auto& f : formats) if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return f;
    return formats.front();
}
VkPresentModeKHR VulkanRenderer::choosePresentMode(const std::vector<VkPresentModeKHR>& modes) const {
    for (const auto m : modes) if (m == VK_PRESENT_MODE_MAILBOX_KHR) return m;
    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D VulkanRenderer::chooseExtent(const VkSurfaceCapabilitiesKHR& caps) const {
    if (caps.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) return caps.currentExtent;
    int width = 0, height = 0; glfwGetFramebufferSize(window_, &width, &height);
    VkExtent2D e{static_cast<std::uint32_t>(std::max(width,0)), static_cast<std::uint32_t>(std::max(height,0))};
    e.width = std::clamp(e.width, caps.minImageExtent.width, caps.maxImageExtent.width);
    e.height = std::clamp(e.height, caps.minImageExtent.height, caps.maxImageExtent.height);
    return e;
}
bool VulkanRenderer::createSwapchain(std::string* error) {
    const auto support = querySwapSupport(physicalDevice_);
    const auto format = chooseSurfaceFormat(support.formats); const auto mode = choosePresentMode(support.modes);
    const auto extent = chooseExtent(support.capabilities);
    std::uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount && imageCount > support.capabilities.maxImageCount) imageCount = support.capabilities.maxImageCount;
    const auto queues = findQueueFamilies(physicalDevice_);
    const std::uint32_t indices[] = {queues.graphics, queues.present};
    VkSwapchainCreateInfoKHR create{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    create.surface = surface_; create.minImageCount = imageCount; create.imageFormat = format.format; create.imageColorSpace = format.colorSpace;
    create.imageExtent = extent; create.imageArrayLayers = 1; create.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (queues.graphics != queues.present) { create.imageSharingMode = VK_SHARING_MODE_CONCURRENT; create.queueFamilyIndexCount = 2; create.pQueueFamilyIndices = indices; }
    else create.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create.preTransform = support.capabilities.currentTransform; create.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create.presentMode = mode; create.clipped = VK_TRUE;
    const VkResult result = vkCreateSwapchainKHR(device_, &create, nullptr, &swapchain_);
    if (result != VK_SUCCESS) return fail(error, vkMessage("vkCreateSwapchainKHR failed", result));
    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr); swapchainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, swapchainImages_.data());
    swapchainFormat_ = format.format; swapchainExtent_ = extent; return true;
}
bool VulkanRenderer::createImageViews(std::string* error) {
    imageViews_.resize(swapchainImages_.size());
    for (std::size_t i = 0; i < swapchainImages_.size(); ++i) {
        VkImageViewCreateInfo create{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO}; create.image = swapchainImages_[i]; create.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create.format = swapchainFormat_; create.components = {VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY};
        create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; create.subresourceRange.baseMipLevel = 0; create.subresourceRange.levelCount = 1; create.subresourceRange.baseArrayLayer = 0; create.subresourceRange.layerCount = 1;
        const VkResult result = vkCreateImageView(device_, &create, nullptr, &imageViews_[i]);
        if (result != VK_SUCCESS) return fail(error, vkMessage("vkCreateImageView failed", result));
    }
    return true;
}
bool VulkanRenderer::createRenderPass(std::string* error) {
    VkAttachmentDescription color{}; color.format = swapchainFormat_; color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference reference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass{}; subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; subpass.colorAttachmentCount = 1; subpass.pColorAttachments = &reference;
    VkSubpassDependency dependency{}; dependency.srcSubpass = VK_SUBPASS_EXTERNAL; dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo create{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO}; create.attachmentCount = 1; create.pAttachments = &color;
    create.subpassCount = 1; create.pSubpasses = &subpass; create.dependencyCount = 1; create.pDependencies = &dependency;
    const VkResult result = vkCreateRenderPass(device_, &create, nullptr, &renderPass_);
    return result == VK_SUCCESS || fail(error, vkMessage("vkCreateRenderPass failed", result));
}
std::vector<char> VulkanRenderer::readBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file) throw std::runtime_error("cannot open SPIR-V shader: " + path);
    const auto size = file.tellg(); if (size <= 0) throw std::runtime_error("empty SPIR-V shader: " + path);
    std::vector<char> bytes(static_cast<std::size_t>(size)); file.seekg(0); file.read(bytes.data(), size);
    if (!file) throw std::runtime_error("cannot read SPIR-V shader: " + path);
    return bytes;
}
bool VulkanRenderer::createPipeline(std::string* error) {
    VkShaderModule vert = VK_NULL_HANDLE, frag = VK_NULL_HANDLE;
    try {
        const auto vertCode = readBinaryFile(std::string(ULTIMATE_SHADER_DIR) + "/scene.vert.spv");
        const auto fragCode = readBinaryFile(std::string(ULTIMATE_SHADER_DIR) + "/scene.frag.spv");
        VkShaderModuleCreateInfo module{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO}; module.codeSize = vertCode.size(); module.pCode = reinterpret_cast<const std::uint32_t*>(vertCode.data());
        VkResult result = vkCreateShaderModule(device_, &module, nullptr, &vert);
        if (result != VK_SUCCESS) return fail(error, vkMessage("vertex shader module failed", result));
        module.codeSize = fragCode.size(); module.pCode = reinterpret_cast<const std::uint32_t*>(fragCode.data());
        result = vkCreateShaderModule(device_, &module, nullptr, &frag);
        if (result != VK_SUCCESS) { vkDestroyShaderModule(device_, vert, nullptr); return fail(error, vkMessage("fragment shader module failed", result)); }
        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT; stages[0].module = vert; stages[0].pName = "main";
        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; stages[1].module = frag; stages[1].pName = "main";
        VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO}; assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkViewport viewport{0.0F,0.0F,static_cast<float>(swapchainExtent_.width),static_cast<float>(swapchainExtent_.height),0.0F,1.0F};
        VkRect2D scissor{{0,0},swapchainExtent_};
        VkPipelineViewportStateCreateInfo viewportState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO}; viewportState.viewportCount = 1; viewportState.pViewports = &viewport; viewportState.scissorCount = 1; viewportState.pScissors = &scissor;
        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO}; raster.depthClampEnable = VK_FALSE; raster.rasterizerDiscardEnable = VK_FALSE; raster.polygonMode = VK_POLYGON_MODE_FILL; raster.lineWidth = 1.0F; raster.cullMode = VK_CULL_MODE_NONE; raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO}; multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPipelineColorBlendAttachmentState blendAttachment{}; blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO}; blend.attachmentCount = 1; blend.pAttachments = &blendAttachment;
        VkPipelineLayoutCreateInfo layout{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        result = vkCreatePipelineLayout(device_, &layout, nullptr, &pipelineLayout_);
        if (result != VK_SUCCESS) { vkDestroyShaderModule(device_, frag, nullptr); vkDestroyShaderModule(device_, vert, nullptr); return fail(error, vkMessage("vkCreatePipelineLayout failed", result)); }
        VkGraphicsPipelineCreateInfo create{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        create.stageCount = 2; create.pStages = stages; create.pVertexInputState = &vertexInput; create.pInputAssemblyState = &assembly;
        create.pViewportState = &viewportState; create.pRasterizationState = &raster; create.pMultisampleState = &multisample; create.pColorBlendState = &blend;
        create.layout = pipelineLayout_; create.renderPass = renderPass_; create.subpass = 0;
        result = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &create, nullptr, &graphicsPipeline_);
        vkDestroyShaderModule(device_, frag, nullptr); vkDestroyShaderModule(device_, vert, nullptr);
        return result == VK_SUCCESS || fail(error, vkMessage("vkCreateGraphicsPipelines failed", result));
    } catch (const std::exception& e) {
        if (frag) vkDestroyShaderModule(device_, frag, nullptr); if (vert) vkDestroyShaderModule(device_, vert, nullptr);
        return fail(error, e.what());
    }
}
bool VulkanRenderer::createFramebuffers(std::string* error) {
    framebuffers_.resize(imageViews_.size());
    for (std::size_t i = 0; i < imageViews_.size(); ++i) {
        VkFramebufferCreateInfo create{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO}; create.renderPass = renderPass_; create.attachmentCount = 1; create.pAttachments = &imageViews_[i];
        create.width = swapchainExtent_.width; create.height = swapchainExtent_.height; create.layers = 1;
        const VkResult result = vkCreateFramebuffer(device_, &create, nullptr, &framebuffers_[i]);
        if (result != VK_SUCCESS) return fail(error, vkMessage("vkCreateFramebuffer failed", result));
    }
    return true;
}
bool VulkanRenderer::createCommandPool(std::string* error) {
    const auto queues = findQueueFamilies(physicalDevice_);
    VkCommandPoolCreateInfo create{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO}; create.queueFamilyIndex = queues.graphics; create.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    const VkResult result = vkCreateCommandPool(device_, &create, nullptr, &commandPool_);
    return result == VK_SUCCESS || fail(error, vkMessage("vkCreateCommandPool failed", result));
}
bool VulkanRenderer::createCommandBuffers(std::string* error) {
    commandBuffers_.resize(FramesInFlight);
    VkCommandBufferAllocateInfo allocate{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO}; allocate.commandPool = commandPool_; allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; allocate.commandBufferCount = static_cast<std::uint32_t>(commandBuffers_.size());
    const VkResult result = vkAllocateCommandBuffers(device_, &allocate, commandBuffers_.data());
    return result == VK_SUCCESS || fail(error, vkMessage("vkAllocateCommandBuffers failed", result));
}
bool VulkanRenderer::createSyncObjects(std::string* error) {
    VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fence{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO}; fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (std::size_t i = 0; i < FramesInFlight; ++i) {
        if (vkCreateSemaphore(device_, &semaphore, nullptr, &imageAvailable_[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device_, &semaphore, nullptr, &renderFinished_[i]) != VK_SUCCESS ||
            vkCreateFence(device_, &fence, nullptr, &inFlight_[i]) != VK_SUCCESS) return fail(error, "failed to create per-frame synchronization objects");
    }
    return true;
}
void VulkanRenderer::recordCommandBuffer(const VkCommandBuffer command, const std::uint32_t imageIndex) {
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    if (vkBeginCommandBuffer(command, &begin) != VK_SUCCESS) throw std::runtime_error("vkBeginCommandBuffer failed");
    VkClearValue clear{}; clear.color = {{0.025F,0.04F,0.065F,1.0F}};
    VkRenderPassBeginInfo render{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO}; render.renderPass = renderPass_; render.framebuffer = framebuffers_[imageIndex];
    render.renderArea = {{0,0},swapchainExtent_}; render.clearValueCount = 1; render.pClearValues = &clear;
    vkCmdBeginRenderPass(command, &render, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);
    vkCmdDraw(command, 3, 1, 0, 0);
    vkCmdEndRenderPass(command);
    if (vkEndCommandBuffer(command) != VK_SUCCESS) throw std::runtime_error("vkEndCommandBuffer failed");
}
void VulkanRenderer::drawFrame() {
    vkWaitForFences(device_, 1, &inFlight_[currentFrame_], VK_TRUE, UINT64_MAX);
    std::uint32_t imageIndex = 0;
    const VkResult acquire = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, imageAvailable_[currentFrame_], VK_NULL_HANDLE, &imageIndex);
    if (acquire == VK_ERROR_OUT_OF_DATE_KHR) { recreateSwapchain(); return; }
    if (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR) throw std::runtime_error(vkMessage("vkAcquireNextImageKHR failed", acquire));
    vkResetFences(device_, 1, &inFlight_[currentFrame_]);
    vkResetCommandBuffer(commandBuffers_[currentFrame_], 0);
    recordCommandBuffer(commandBuffers_[currentFrame_], imageIndex);
    const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO}; submit.waitSemaphoreCount = 1; submit.pWaitSemaphores = &imageAvailable_[currentFrame_]; submit.pWaitDstStageMask = &waitStage;
    submit.commandBufferCount = 1; submit.pCommandBuffers = &commandBuffers_[currentFrame_]; submit.signalSemaphoreCount = 1; submit.pSignalSemaphores = &renderFinished_[currentFrame_];
    const VkResult submitted = vkQueueSubmit(graphicsQueue_, 1, &submit, inFlight_[currentFrame_]);
    if (submitted != VK_SUCCESS) throw std::runtime_error(vkMessage("vkQueueSubmit failed", submitted));
    VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR}; present.waitSemaphoreCount = 1; present.pWaitSemaphores = &renderFinished_[currentFrame_];
    present.swapchainCount = 1; present.pSwapchains = &swapchain_; present.pImageIndices = &imageIndex;
    const VkResult shown = vkQueuePresentKHR(presentQueue_, &present);
    if (shown == VK_ERROR_OUT_OF_DATE_KHR || shown == VK_SUBOPTIMAL_KHR || framebufferResized_) { framebufferResized_ = false; recreateSwapchain(); }
    else if (shown != VK_SUCCESS) throw std::runtime_error(vkMessage("vkQueuePresentKHR failed", shown));
    currentFrame_ = (currentFrame_ + 1) % FramesInFlight;
}
void VulkanRenderer::recreateSwapchain() {
    int w = 0, h = 0; glfwGetFramebufferSize(window_, &w, &h);
    while (w == 0 || h == 0) { glfwWaitEvents(); glfwGetFramebufferSize(window_, &w, &h); }
    vkDeviceWaitIdle(device_); cleanupSwapchain();
    std::string error;
    if (!createSwapchain(&error) || !createImageViews(&error) || !createRenderPass(&error) || !createPipeline(&error) || !createFramebuffers(&error))
        throw std::runtime_error("swapchain recreation failed: " + error);
}
void VulkanRenderer::cleanupSwapchain() noexcept {
    if (device_) {
        for (const auto fb : framebuffers_) if (fb) vkDestroyFramebuffer(device_, fb, nullptr);
        framebuffers_.clear();
        if (graphicsPipeline_) vkDestroyPipeline(device_, graphicsPipeline_, nullptr); graphicsPipeline_ = VK_NULL_HANDLE;
        if (pipelineLayout_) vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr); pipelineLayout_ = VK_NULL_HANDLE;
        if (renderPass_) vkDestroyRenderPass(device_, renderPass_, nullptr); renderPass_ = VK_NULL_HANDLE;
        for (const auto view : imageViews_) if (view) vkDestroyImageView(device_, view, nullptr);
        imageViews_.clear();
        if (swapchain_) vkDestroySwapchainKHR(device_, swapchain_, nullptr); swapchain_ = VK_NULL_HANDLE;
        swapchainImages_.clear();
    }
}
void VulkanRenderer::pollEvents() const { glfwPollEvents(); }
bool VulkanRenderer::shouldClose() const { return !window_ || glfwWindowShouldClose(window_) == GLFW_TRUE; }
void VulkanRenderer::framebufferSizeCallback(GLFWwindow* window, int, int) {
    auto* self = static_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window)); if (self) self->framebufferResized_ = true;
}
void VulkanRenderer::shutdown() noexcept {
    if (device_) {
        vkDeviceWaitIdle(device_); cleanupSwapchain();
        for (std::size_t i = 0; i < FramesInFlight; ++i) {
            if (imageAvailable_[i]) vkDestroySemaphore(device_, imageAvailable_[i], nullptr);
            if (renderFinished_[i]) vkDestroySemaphore(device_, renderFinished_[i], nullptr);
            if (inFlight_[i]) vkDestroyFence(device_, inFlight_[i], nullptr);
        }
        if (commandPool_) vkDestroyCommandPool(device_, commandPool_, nullptr);
        vkDestroyDevice(device_, nullptr); device_ = VK_NULL_HANDLE;
    }
    if (surface_ && instance_) vkDestroySurfaceKHR(instance_, surface_, nullptr); surface_ = VK_NULL_HANDLE;
    if (instance_) vkDestroyInstance(instance_, nullptr); instance_ = VK_NULL_HANDLE;
    if (window_) glfwDestroyWindow(window_); window_ = nullptr;
    glfwTerminate();
}
}
