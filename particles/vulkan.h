#pragma once



#include <vulkan/vulkan.hpp>



#ifndef NDEBUG
#define VALIDATION_LAYERS_ENABLED
#endif







namespace Vulkan {

    const std::vector<const char*> validationLayers = {
#ifdef VALIDATION_LAYERS_ENABLED
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    const std::vector<const char*> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    void init();
    void run();
    void cleanup();

};