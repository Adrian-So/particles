#pragma once



#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>



#ifndef NDEBUG
#define VALIDATION_LAYERS_ENABLED
#endif







namespace Vulkan {



    const int MAX_FRAMES_IN_FLIGHT = 2;

    const std::vector<const char*> VALIDATION_LAYERS = {
#ifdef VALIDATION_LAYERS_ENABLED
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };



    void initialise();
    void cleanup();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);



    extern GLFWwindow* window;

    extern VkInstance instance;
    extern VkDebugUtilsMessengerEXT debugMessenger;
    extern VkSurfaceKHR surface;

    extern VkPhysicalDevice physicalDevice;
    extern VkPhysicalDeviceMemoryProperties memoryProperties;

    extern VkDevice device;
    extern uint32_t queueFamilyIndex;
    extern VkQueue queue;
    extern VkPresentModeKHR presentMode;
    extern VkSurfaceFormatKHR surfaceFormat;
    extern VkExtent2D extent;
    extern uint32_t imageCount;
    extern VkSurfaceTransformFlagBitsKHR preTransform;

    extern VkCommandPool commandPool;



};