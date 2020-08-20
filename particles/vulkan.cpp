#include "vulkan.h"

#include "GLFW/glfw3.h"

#include "particle.h"

#include <iostream>
#include <fstream>



static const int MAX_FRAMES_IN_FLIGHT = 2;

static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}
static const VkDebugUtilsMessengerCreateInfoEXT _debugMessengerCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext = nullptr,
    .flags = 0,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = _debugCallback,
    .pUserData = nullptr,
};







static struct PhysicalDeviceChoice {
    int rating;
    uint32_t queueFamilyIndex;
    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D extent;
    uint32_t imageCount;
    VkSurfaceTransformFlagBitsKHR preTransform;
};



static GLFWwindow* _window;
static VkInstance _instance;
static VkDebugUtilsMessengerEXT _debugMessenger;
static VkSurfaceKHR _surface;

static VkPhysicalDevice _physicalDevice;
static VkDevice _device;
static uint32_t _queueFamilyIndex;
static VkQueue _queue;
static VkPresentModeKHR _presentMode;
static VkSurfaceFormatKHR _surfaceFormat;
static VkExtent2D _extent;
static uint32_t _imageCount;
static VkSurfaceTransformFlagBitsKHR _preTransform;

static VkRenderPass _renderPass;
static VkPipelineLayout _pipelineLayout;
static VkPipeline _graphicsPipeline;

static VkSwapchainKHR _swapchain;
static std::vector<VkImage> _swapchainImages;
static std::vector<VkImageView> _swapchainImageViews;
static std::vector<VkFramebuffer> _swapchainFramebuffers;

static VkCommandPool _commandPool;

static VkBuffer _particlesBuffer;
static VkDeviceMemory _particlesDeviceMemory;

static std::vector<VkCommandBuffer> _commandBuffers;

static std::vector<VkSemaphore> _imageAvailableSemaphores;
static std::vector<VkSemaphore> _renderFinishedSemaphores;
static std::vector<VkFence> _inFlightFences;
static std::vector<VkFence> _imagesInFlight;
static size_t _currentFrame = 0;



static void _createWindow();
static void _createInstance();
static void _createSurface();

static void _createDevice();
static PhysicalDeviceChoice _ratePhysicalDevice(const VkPhysicalDevice physicalDevice);
static int _chooseDeviceExtensions(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);
static int _chooseDeviceQueueFamilies(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);
static int _chooseDevicePresentMode(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);
static int _chooseDeviceSurfaceFormat(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);
static int _chooseDeviceProperties(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);
static int _chooseDeviceSurfaceCapabilities(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);

static void _createRenderPass();
static void _createGraphicsPipeline();
static VkShaderModule _createShaderModule(const std::string& fileName);

static void _createFramebuffers();

static void _createCommandPool();

static void _createSyncObjects();

static void _createParticlesBuffer();
static void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
static uint32_t _findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
static void _copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

static void _createCommandBuffers();

static void _draw();







void Vulkan::init() {
    _createWindow();
    _createInstance();
    _createSurface();
    _createDevice();
    _createRenderPass();
    _createGraphicsPipeline();
    _createFramebuffers();
    //createDescriptorSetLayout();
    _createCommandPool();
    _createParticlesBuffer();
    //createUniformBuffers();
    //createDescriptorPool();
    //createDescriptorSets();
    _createCommandBuffers();
    _createSyncObjects();
}



void Vulkan::run() {
    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
        _draw();
    }
    vkDeviceWaitIdle(_device);
}



void Vulkan::cleanup() {

    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(_device, _inFlightFences[i], nullptr);
    }

    for (auto framebuffer : _swapchainFramebuffers) {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }
    for (auto imageView : _swapchainImageViews) {
        vkDestroyImageView(_device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    vkFreeCommandBuffers(_device, _commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());

    vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
    vkDestroyRenderPass(_device, _renderPass, nullptr);

    vkDestroyBuffer(_device, _particlesBuffer, nullptr);
    vkFreeMemory(_device, _particlesDeviceMemory, nullptr);

    vkDestroyCommandPool(_device, _commandPool, nullptr);

    vkDestroyDevice(_device, nullptr);

    vkDestroySurfaceKHR(_instance, _surface, nullptr);

#ifdef VALIDATION_LAYERS_ENABLED
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(_instance, _debugMessenger, nullptr);
    }
#endif

    vkDestroyInstance(_instance, nullptr);

    glfwDestroyWindow(_window);
    glfwTerminate();

}







void _createWindow() {

    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("Error: failed to initialise GLFW.");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow(1280, 720, "Particles", nullptr, nullptr);
    if (_window == nullptr) {
        throw std::runtime_error("Error: GLFW failed to create window.");
    }

}



void _createInstance() {

    uint32_t glfwExtensionCount;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    void* pNext = nullptr;
#ifdef VALIDATION_LAYERS_ENABLED
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (auto validationLayer : Vulkan::validationLayers) {
        bool found = false;
        for (auto availableLayer : availableLayers) {
            if (strcmp(validationLayer, availableLayer.layerName)) {
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error("Error: validation layer requested but not found.");
        }
    }
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = _debugMessengerCreateInfo;
    pNext = &debugMessengerCreateInfo;
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkApplicationInfo appInfo{
        .sType                      = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext                      = nullptr,
        .pApplicationName           = "particles",
        .applicationVersion         = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName                = nullptr,
        .engineVersion              = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion                 = VK_API_VERSION_1_2,
    };
    VkInstanceCreateInfo createInfo{
        .sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                      = pNext,
        .flags                      = 0,
        .pApplicationInfo           = &appInfo,
        .enabledLayerCount          = static_cast<uint32_t>(Vulkan::validationLayers.size()),
        .ppEnabledLayerNames        = Vulkan::validationLayers.data(),
        .enabledExtensionCount      = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames    = extensions.data(),
    };
    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create vulkan instance.");
    }

#ifdef VALIDATION_LAYERS_ENABLED
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == nullptr || func(_instance, &_debugMessengerCreateInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Error: could not retrieve Vulkan debug messenger thing.");
    }
#endif

}



void _createSurface() {
    if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create surface.");
    }
}



void _createDevice() {

    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0) {
        throw std::runtime_error("Error: vulkan not supported by any device.");
    }
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, physicalDevices.data());

    int highestRating = -1;
    int bestPhysicalDeviceIndex = -1;
    std::vector<PhysicalDeviceChoice> ratings(physicalDeviceCount);
    for (uint32_t i = 0; i < physicalDeviceCount; i++) {
        ratings[i] = _ratePhysicalDevice(physicalDevices[i]);
        if (ratings[i].rating > highestRating) {
            highestRating = ratings[i].rating;
            bestPhysicalDeviceIndex = i;
        }
    }
    if (bestPhysicalDeviceIndex == -1) {
        throw std::runtime_error("Error: could not find suitable device.");
    }
    _physicalDevice = physicalDevices[bestPhysicalDeviceIndex];
    _presentMode = ratings[bestPhysicalDeviceIndex].presentMode;
    _surfaceFormat = ratings[bestPhysicalDeviceIndex].surfaceFormat;
    _queueFamilyIndex = ratings[bestPhysicalDeviceIndex].queueFamilyIndex;
    _extent = ratings[bestPhysicalDeviceIndex].extent;
    _imageCount = ratings[bestPhysicalDeviceIndex].imageCount;
    _preTransform = ratings[bestPhysicalDeviceIndex].preTransform;

    float queuePriority = 1.0;
    VkDeviceQueueCreateInfo deviceQueueCreateInfo{
        .sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .queueFamilyIndex   = _queueFamilyIndex,
        .queueCount         = 1,
        .pQueuePriorities   = &queuePriority,
    };
    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo deviceCreateInfo{
        .sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                      = nullptr,
        .flags                      = 0,
        .queueCreateInfoCount       = 1,
        .pQueueCreateInfos          = &deviceQueueCreateInfo,
        .enabledLayerCount          = static_cast<uint32_t>(Vulkan::validationLayers.size()),
        .ppEnabledLayerNames        = Vulkan::validationLayers.data(),
        .enabledExtensionCount      = static_cast<uint32_t>(Vulkan::requiredDeviceExtensions.size()),
        .ppEnabledExtensionNames    = Vulkan::requiredDeviceExtensions.data(),
        .pEnabledFeatures           = &deviceFeatures,
    };
    if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device) != VK_SUCCESS) {
        throw std::runtime_error("Error: could not create device.");
    }
    vkGetDeviceQueue(_device, _queueFamilyIndex, 0, &_queue);

}



PhysicalDeviceChoice _ratePhysicalDevice(const VkPhysicalDevice physicalDevice) {

    PhysicalDeviceChoice choice{ .rating = 0 };

    std::vector<int(*)(const VkPhysicalDevice, PhysicalDeviceChoice&)> chooseFunctions = {
        _chooseDeviceExtensions,
        _chooseDevicePresentMode,
        _chooseDeviceSurfaceFormat,
        _chooseDeviceQueueFamilies,
        _chooseDeviceProperties,
        _chooseDeviceSurfaceCapabilities,
    };

    for (auto chooseFunction : chooseFunctions) {
        int result = chooseFunction(physicalDevice, choice);
        if (result > 0) {
            choice.rating += result;
        } else {
            choice.rating = -1;
            return choice;
        }
    }

    return choice;

}



int _chooseDeviceExtensions(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice) {

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    for (auto requiredExtension : Vulkan::requiredDeviceExtensions) {
        bool found = false;
        for (auto availableExtension : availableExtensions) {
            if (strcmp(requiredExtension, availableExtension.extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return -1;
        }
    }

    return 1;

}



int _chooseDeviceQueueFamilies(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice) {

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        // Improve queue ratings: include option for queues to be 'specialised'
        VkBool32 surfaceSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surface, &surfaceSupport);
        if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && surfaceSupport) {
            choice.queueFamilyIndex = i;
            return 256;
        }
    }

    return -1;

}



int _chooseDevicePresentMode(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice) {

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, nullptr);
    if (presentModeCount == 0) {
        return -1;
    }
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, presentModes.data());

    for (auto mode : presentModes) {
        // Better rating of present modes needed
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            choice.presentMode = mode;
            return 64;
        }
    }

    choice.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    return 0;

}



int _chooseDeviceSurfaceFormat(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice) {

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, nullptr);
    if (formatCount == 0) {
        return -1;
    }
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, formats.data());

    for (auto format : formats) {
        // Better rating of formats needed
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            choice.surfaceFormat = format;
            return 64;
        }
    }

    choice.surfaceFormat = formats[0];
    return 0;

}



int _chooseDeviceProperties(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice) {

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    switch (properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return 1024;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return 512;
        default:
            return -1;
    }

}



int _chooseDeviceSurfaceCapabilities(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice) {

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, _surface, &capabilities);

    if (capabilities.currentExtent.width != UINT32_MAX) {
        choice.extent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);
        choice.extent.width = width > capabilities.minImageExtent.width ? width : capabilities.minImageExtent.width;
        choice.extent.width = choice.extent.width < capabilities.maxImageExtent.width ? choice.extent.width : capabilities.maxImageExtent.width;
        choice.extent.height = height > capabilities.minImageExtent.height ? height : capabilities.minImageExtent.height;
        choice.extent.height = choice.extent.height < capabilities.maxImageExtent.height ? choice.extent.height : capabilities.maxImageExtent.height;
    }

    choice.imageCount = 3;
    if (choice.imageCount < capabilities.minImageCount) {
        choice.imageCount = capabilities.minImageCount;
    }
    if (capabilities.maxImageCount > 0) {
        if (choice.imageCount > capabilities.maxImageCount) {
            choice.imageCount = capabilities.maxImageCount;
        }
    }

    choice.preTransform = capabilities.currentTransform;

    return 1;

}



void _createRenderPass() {

    VkAttachmentDescription attachment{
        .flags          = 0,
        .format         = _surfaceFormat.format,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference reference{
        .attachment = 0,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpass{
        .flags                      = 0,
        .pipelineBindPoint          = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount       = 0,
        .pInputAttachments          = nullptr,
        .colorAttachmentCount       = 1,
        .pColorAttachments          = &reference,
        .pResolveAttachments        = nullptr,
        .pDepthStencilAttachment    = nullptr,
        .preserveAttachmentCount    = 0,
        .pPreserveAttachments       = nullptr,
    };

    VkSubpassDependency dependency{
        .srcSubpass         = VK_SUBPASS_EXTERNAL,
        .dstSubpass         = 0,
        .srcStageMask       = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask       = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask      = 0,
        .dstAccessMask      = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags    = 0,
    };

    VkRenderPassCreateInfo createInfo{
        .sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .attachmentCount    = 1,
        .pAttachments       = &attachment,
        .subpassCount       = 1,
        .pSubpasses         = &subpass,
        .dependencyCount    = 1,
        .pDependencies      = &dependency,
    };

    if (vkCreateRenderPass(_device, &createInfo, nullptr, &_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Error: could not create render pass.");
    }

}



void _createFramebuffers() {

    VkSwapchainCreateInfoKHR swapchainCreateInfo{
        .sType                  = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext                  = nullptr,
        .flags                  = 0,
        .surface                = _surface,
        .minImageCount          = _imageCount,
        .imageFormat            = _surfaceFormat.format,
        .imageColorSpace        = _surfaceFormat.colorSpace,
        .imageExtent            = _extent,
        .imageArrayLayers       = 1,
        .imageUsage             = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode       = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount  = 0,
        .pQueueFamilyIndices    = nullptr,
        .preTransform           = _preTransform,
        .compositeAlpha         = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode            = _presentMode,
        .clipped                = VK_TRUE,
        .oldSwapchain           = VK_NULL_HANDLE,
    };
    if (vkCreateSwapchainKHR(_device, &swapchainCreateInfo, nullptr, &_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create swapchain.");
    }

    vkGetSwapchainImagesKHR(_device, _swapchain, &_imageCount, nullptr);
    _swapchainImages.resize(_imageCount);
    vkGetSwapchainImagesKHR(_device, _swapchain, &_imageCount, _swapchainImages.data());

    _swapchainImageViews.resize(_imageCount);
    VkImageViewCreateInfo imageViewCreateInfo{
        .sType              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .image              = VK_NULL_HANDLE,
        .viewType           = VK_IMAGE_VIEW_TYPE_2D,
        .format             = _surfaceFormat.format,
        .components         = {
            .r                  = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g                  = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b                  = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a                  = VK_COMPONENT_SWIZZLE_IDENTITY, },
        .subresourceRange   = {
            .aspectMask         = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel       = 0,
            .levelCount         = 1,
            .baseArrayLayer     = 0,
            .layerCount         = 1, },
    };
    for (size_t i = 0; i < _imageCount; i++) {
        imageViewCreateInfo.image = _swapchainImages[i];
        if (vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &_swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to create image view.");
        }
    }

    _swapchainFramebuffers.resize(_imageCount);
    VkFramebufferCreateInfo framebufferCreateInfo{
        .sType              = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .renderPass         = _renderPass,
        .attachmentCount    = 1,
        .pAttachments       = nullptr,
        .width              = _extent.width,
        .height             = _extent.height,
        .layers             = 1,
    };
    for (size_t i = 0; i < _imageCount; i++) {
        framebufferCreateInfo.pAttachments = &_swapchainImageViews[i];
        if (vkCreateFramebuffer(_device, &framebufferCreateInfo, nullptr, &_swapchainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to create framebuffer.");
        }
    }

}



void _createGraphicsPipeline() {
            
    VkShaderModule vertexShaderModule = _createShaderModule("..\\shaders\\vert.spv");
    VkShaderModule fragmentShaderModule = _createShaderModule("..\\shaders\\frag.spv");
    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[]{
        {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext                  = nullptr,
            .flags                  = 0,
            .stage                  = VK_SHADER_STAGE_VERTEX_BIT,
            .module                 = vertexShaderModule,
            .pName                  = "main",
            .pSpecializationInfo    = nullptr,
        }, {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext                  = nullptr,
            .flags                  = 0,
            .stage                  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module                 = fragmentShaderModule,
            .pName                  = "main",
            .pSpecializationInfo    = nullptr,
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
        .sType                              = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                              = nullptr,
        .flags                              = 0,
        .vertexBindingDescriptionCount      = 1,
        .pVertexBindingDescriptions         = &Particle::bindingDescription,
        .vertexAttributeDescriptionCount    = static_cast<uint32_t>(Particle::attributeDescription.size()),
        .pVertexAttributeDescriptions       = Particle::attributeDescription.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .topology               = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    /*VkPipelineTessellationStateCreateInfo*/

    VkViewport viewport{
        .x          = 0.0f,
        .y          = 0.0f,
        .width      = (float)_extent.width,
        .height     = (float)_extent.height,
        .minDepth   = 0.0f,
        .maxDepth   = 1.0f,
    };
    VkRect2D scissors{
        .offset     = {
            .x          = 0,
            .y          = 0, },
        .extent     = _extent,
    };
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext          = nullptr,
        .flags          = 0,
        .viewportCount  = 1,
        .pViewports     = &viewport,
        .scissorCount   = 1,
        .pScissors      = &scissors,
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
        .sType                      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext                      = nullptr,
        .flags                      = 0,
        .depthClampEnable           = VK_FALSE,
        .rasterizerDiscardEnable    = VK_FALSE,
        .polygonMode                = VK_POLYGON_MODE_FILL,
        .cullMode                   = VK_CULL_MODE_BACK_BIT,
        .frontFace                  = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable            = VK_FALSE,
        .depthBiasConstantFactor    = 0.0f,
        .depthBiasClamp             = 0.0f,
        .depthBiasSlopeFactor       = 0.0f,
        .lineWidth                  = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .rasterizationSamples   = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable    = VK_FALSE,
        .minSampleShading       = 1.0f,
        .pSampleMask            = nullptr,
        .alphaToCoverageEnable  = VK_FALSE,
        .alphaToOneEnable       = VK_FALSE,
    };

    /*VkPipelineDepthStencilStateCreateInfo*/

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
        .blendEnable            = VK_FALSE,
        .srcColorBlendFactor    = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp           = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp           = VK_BLEND_OP_ADD,
        .colorWriteMask         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .logicOpEnable      = VK_FALSE,
        .logicOp            = VK_LOGIC_OP_COPY,
        .attachmentCount    = 1,
        .pAttachments       = &colorBlendAttachmentState,
        .blendConstants     = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    /*VkDynamicState dynamicState{
    };*/
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .dynamicStateCount  = 0,
        .pDynamicStates     = nullptr,
    };

    VkPipelineLayoutCreateInfo layoutCreateInfo{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .setLayoutCount         = 0,
        .pSetLayouts            = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = nullptr,
    };
    if (vkCreatePipelineLayout(_device, &layoutCreateInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create pipeline layout.");
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
        .sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .stageCount             = 2,
        .pStages                = shaderStageCreateInfos,
        .pVertexInputState      = &vertexInputStateCreateInfo,
        .pInputAssemblyState    = &inputAssemblyStateCreateInfo,
        .pTessellationState     = nullptr,
        .pViewportState         = &viewportStateCreateInfo,
        .pRasterizationState    = &rasterizationStateCreateInfo,
        .pMultisampleState      = &multisampleStateCreateInfo,
        .pDepthStencilState     = nullptr,
        .pColorBlendState       = &colorBlendStateCreateInfo,
        .pDynamicState          = &dynamicStateCreateInfo,
        .layout                 = _pipelineLayout,
        .renderPass             = _renderPass,
        .subpass                = 0,
        .basePipelineHandle     = VK_NULL_HANDLE,
        .basePipelineIndex      = -1,
    };
    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create graphics pipeline.");
    }

    vkDestroyShaderModule(_device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(_device, fragmentShaderModule, nullptr);

}



VkShaderModule _createShaderModule(const std::string &fileName) {

    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error: failed to open shader module.");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo{
        .sType      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext      = nullptr,
        .flags      = 0,
        .codeSize   = fileSize,
        .pCode      = reinterpret_cast<const uint32_t*>(buffer.data()),
    };
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create shader module.");
    }

    return shaderModule;

}



void _createCommandPool() {

    VkCommandPoolCreateInfo createInfo{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .queueFamilyIndex   = _queueFamilyIndex,
    };
    if (vkCreateCommandPool(_device, &createInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create command pool.");
    }

}



void _createParticlesBuffer() {

    VkDeviceSize bufferSize = sizeof(Particle) * particles.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    _createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    if (vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data) != VK_SUCCESS) {
        throw std::runtime_error("Error: unable to map memory.");
    }
    memcpy(data, particles.data(), (size_t)bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    _createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _particlesBuffer, _particlesDeviceMemory);
    _copyBuffer(stagingBuffer, _particlesBuffer, bufferSize);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

}
        
        
        
void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

    VkBufferCreateInfo bufferInfo{
        .sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size           = size,
        .usage          = usage,
        .sharingMode    = VK_SHARING_MODE_EXCLUSIVE,
    };
    if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vertex buffer.");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{
        .sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize     = memRequirements.size,
        .memoryTypeIndex    = _findMemoryType(memRequirements.memoryTypeBits, properties),
    };
    if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate vertex buffer memory.");
    }

    vkBindBufferMemory(_device, buffer, bufferMemory, 0);

}



uint32_t _findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type.");
}



void _copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

    VkCommandBufferAllocateInfo allocInfo{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = _commandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkBufferCopy copyRegion{
        .srcOffset = 0,
        .dstOffset = 0,
        .size      = size,
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers    = &commandBuffer,
    };
    vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_queue);

    vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);

}



void _createCommandBuffers() {

    _commandBuffers.resize(_imageCount);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = _commandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = _imageCount,
    };
    if (vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, _commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to allocate command buffers.");
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .pInheritanceInfo   = nullptr,
    };

    VkClearValue clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
    VkRenderPassBeginInfo renderPassBeginInfo{
        .sType              = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext              = nullptr,
        .renderPass         = _renderPass,
        .framebuffer        = VK_NULL_HANDLE,
        .renderArea         = {
            .offset             = { 0, 0 },
            .extent             = _extent },
        .clearValueCount    = 1,
        .pClearValues       = &clearColour,
    };

    VkDeviceSize offset = 0;

    for (size_t i = 0; i < _imageCount; i++) {
        if (vkBeginCommandBuffer(_commandBuffers[i], &commandBufferBeginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to begin recording command buffer.");
        }
        renderPassBeginInfo.framebuffer = _swapchainFramebuffers[i];
        vkCmdBeginRenderPass(_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);
            vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, &_particlesBuffer, &offset);
            vkCmdDraw(_commandBuffers[i], static_cast<uint32_t>(particles.size()), 1, 0, 0);
        vkCmdEndRenderPass(_commandBuffers[i]);
        if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to record command buffer.");
        }
    }

}



void _createSyncObjects() {

    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    _imagesInFlight.resize(_imageCount, VK_NULL_HANDLE);

    const VkSemaphoreCreateInfo semaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    const VkFenceCreateInfo fenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(_device, &fenceCreateInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS
        ) {
            throw std::runtime_error("Error: failed to create synchronisation objects.");
        }
    }

}



void _draw() {

    vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    if (vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to acquire swapchain image.");
    }
    if (_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(_device, 1, &_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    _imagesInFlight[imageIndex] = _inFlightFences[_currentFrame];

    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{
        .sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                  = nullptr,
        .waitSemaphoreCount     = 1,
        .pWaitSemaphores        = &_imageAvailableSemaphores[_currentFrame],
        .pWaitDstStageMask      = &waitStages,
        .commandBufferCount     = 1,
        .pCommandBuffers        = &_commandBuffers[imageIndex],
        .signalSemaphoreCount   = 1,
        .pSignalSemaphores      = &_renderFinishedSemaphores[_currentFrame],
    };
    vkResetFences(_device, 1, &_imagesInFlight[_currentFrame]);
    if (vkQueueSubmit(_queue, 1, &submitInfo, _imagesInFlight[_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to submit draw command.");
    }

    VkPresentInfoKHR presentInfo{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext              = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &_renderFinishedSemaphores[_currentFrame],
        .swapchainCount     = 1,
        .pSwapchains        = &_swapchain,
        .pImageIndices      = &imageIndex,
        .pResults           = nullptr,
    };
    if (vkQueuePresentKHR(_queue, &presentInfo) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to present swapchain image.");
    }

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

}