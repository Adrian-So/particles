#include "vulkan.h"

#include "particle.h"
#include "mvp.h"

#include <iostream>







struct PhysicalDeviceChoice {
    int rating;
    uint32_t queueFamilyIndex;
    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D extent;
    uint32_t imageCount;
    VkSurfaceTransformFlagBitsKHR preTransform;
};



static inline void _createWindow();
static inline void _createInstance();
static inline void _createSurface();
static inline void _choosePhysicalDevice();
static inline void _createDevice();
static inline void _createCommandPool();

static int _chooseDeviceExtensions(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);
static int _chooseDeviceQueueFamilies(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);
static int _chooseDevicePresentMode(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);
static int _chooseDeviceSurfaceFormat(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);
static int _chooseDeviceProperties(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);
static int _chooseDeviceSurfaceCapabilities(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice);



namespace Vulkan {

    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    VkDevice device;
    uint32_t queueFamilyIndex;
    VkQueue queue;
    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D extent;
    uint32_t imageCount;
    VkSurfaceTransformFlagBitsKHR preTransform;

    VkCommandPool commandPool;

}







#ifdef VALIDATION_LAYERS_ENABLED
static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}
static const VkDebugUtilsMessengerCreateInfoEXT _debugMessengerCreateInfo{
    .sType              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext              = nullptr,
    .flags              = 0,
    .messageSeverity    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType        = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback    = _debugCallback,
    .pUserData          = nullptr,
};
#endif







void Vulkan::initialise() {
    _createWindow();
    _createInstance();
    _createSurface();
    _choosePhysicalDevice();
    _createDevice();
    _createCommandPool();
}



void Vulkan::cleanup() {
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
#ifdef VALIDATION_LAYERS_ENABLED
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, nullptr);
    }
#endif
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}







void _createWindow() {

    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("Error: failed to initialise GLFW.");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    Vulkan::window = glfwCreateWindow(1280, 720, "Particles", nullptr, nullptr);
    if (Vulkan::window == nullptr) {
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
    for (auto validationLayer : Vulkan::VALIDATION_LAYERS) {
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
        .enabledLayerCount          = static_cast<uint32_t>(Vulkan::VALIDATION_LAYERS.size()),
        .ppEnabledLayerNames        = Vulkan::VALIDATION_LAYERS.data(),
        .enabledExtensionCount      = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames    = extensions.data(),
    };
    if (vkCreateInstance(&createInfo, nullptr, &Vulkan::instance) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create vulkan instance.");
    }

#ifdef VALIDATION_LAYERS_ENABLED
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Vulkan::instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == nullptr || func(Vulkan::instance, &_debugMessengerCreateInfo, nullptr, &Vulkan::debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Error: could not retrieve Vulkan debug messenger thing.");
    }
#endif

}



void _createSurface() {
    if (glfwCreateWindowSurface(Vulkan::instance, Vulkan::window, nullptr, &Vulkan::surface) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create surface.");
    }
}



void _choosePhysicalDevice() {

    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(Vulkan::instance, &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0) {
        throw std::runtime_error("Error: vulkan not supported by any device.");
    }
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(Vulkan::instance, &physicalDeviceCount, physicalDevices.data());

    Vulkan::physicalDevice = VK_NULL_HANDLE;
    PhysicalDeviceChoice highestRating{ .rating = 0 };
    int(*chooseFunctions[])(const VkPhysicalDevice, PhysicalDeviceChoice&) = {
        _chooseDeviceExtensions,
        _chooseDevicePresentMode,
        _chooseDeviceSurfaceFormat,
        _chooseDeviceQueueFamilies,
        _chooseDeviceProperties,
        _chooseDeviceSurfaceCapabilities,
    };
    for (VkPhysicalDevice physicalDevice : physicalDevices) {
        PhysicalDeviceChoice rating{ .rating = 0 };
        for (auto chooseFunction : chooseFunctions) {
            int result = chooseFunction(physicalDevice, rating);
            if (result > 0) {
                rating.rating += result;
            } else {
                rating.rating = -1;
                break;
            }
        }
        if (rating.rating > highestRating.rating) {
            highestRating = rating;
            Vulkan::physicalDevice = physicalDevice;
        }
    }
    if (Vulkan::physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Error: could not find suitable device.");
    }
    Vulkan::presentMode = highestRating.presentMode;
    Vulkan::surfaceFormat = highestRating.surfaceFormat;
    Vulkan::queueFamilyIndex = highestRating.queueFamilyIndex;
    Vulkan::extent = highestRating.extent;
    Vulkan::imageCount = highestRating.imageCount;
    Vulkan::preTransform = highestRating.preTransform;

    vkGetPhysicalDeviceMemoryProperties(Vulkan::physicalDevice, &Vulkan::memoryProperties);

}



void _createDevice() {

    float queuePriority = 1.0;
    VkDeviceQueueCreateInfo deviceQueueCreateInfo{
        .sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .queueFamilyIndex   = Vulkan::queueFamilyIndex,
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
        .enabledLayerCount          = static_cast<uint32_t>(Vulkan::VALIDATION_LAYERS.size()),
        .ppEnabledLayerNames        = Vulkan::VALIDATION_LAYERS.data(),
        .enabledExtensionCount      = static_cast<uint32_t>(Vulkan::REQUIRED_DEVICE_EXTENSIONS.size()),
        .ppEnabledExtensionNames    = Vulkan::REQUIRED_DEVICE_EXTENSIONS.data(),
        .pEnabledFeatures           = &deviceFeatures,
    };
    if (vkCreateDevice(Vulkan::physicalDevice, &deviceCreateInfo, nullptr, &Vulkan::device) != VK_SUCCESS) {
        throw std::runtime_error("Error: could not create device.");
    }
    vkGetDeviceQueue(Vulkan::device, Vulkan::queueFamilyIndex, 0, &Vulkan::queue);

}



void _createCommandPool() {

    VkCommandPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = Vulkan::queueFamilyIndex,
    };
    if (vkCreateCommandPool(Vulkan::device, &createInfo, nullptr, &Vulkan::commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create command pool.");
    }

}







int _chooseDeviceExtensions(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice) {

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    for (auto requiredExtension : Vulkan::REQUIRED_DEVICE_EXTENSIONS) {
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
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, Vulkan::surface, &surfaceSupport);
        if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && surfaceSupport) {
            choice.queueFamilyIndex = i;
            return 256;
        }
    }

    return -1;

}



int _chooseDevicePresentMode(const VkPhysicalDevice physicalDevice, PhysicalDeviceChoice& choice) {

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, Vulkan::surface, &presentModeCount, nullptr);
    if (presentModeCount == 0) {
        return -1;
    }
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, Vulkan::surface, &presentModeCount, presentModes.data());

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
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, Vulkan::surface, &formatCount, nullptr);
    if (formatCount == 0) {
        return -1;
    }
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, Vulkan::surface, &formatCount, formats.data());

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
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, Vulkan::surface, &capabilities);

    if (capabilities.currentExtent.width != UINT32_MAX) {
        choice.extent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(Vulkan::window, &width, &height);
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







void Vulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

    VkBufferCreateInfo bufferInfo{
        .sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size           = size,
        .usage          = usage,
        .sharingMode    = VK_SHARING_MODE_EXCLUSIVE,
    };
    if (vkCreateBuffer(Vulkan::device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create buffer.");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Vulkan::device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{
        .sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize     = memRequirements.size,
        .memoryTypeIndex    = findMemoryType(memRequirements.memoryTypeBits, properties),
    };
    if (vkAllocateMemory(Vulkan::device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to allocate memory.");
    }

    vkBindBufferMemory(Vulkan::device, buffer, bufferMemory, 0);

}



uint32_t Vulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Error: failed to find suitable memory type.");
}



void Vulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

    VkCommandBufferAllocateInfo allocInfo{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = commandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(Vulkan::device, &allocInfo, &commandBuffer);

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
    vkQueueSubmit(Vulkan::queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(Vulkan::queue);

    vkFreeCommandBuffers(Vulkan::device, commandPool, 1, &commandBuffer);

}