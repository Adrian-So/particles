#include "vulkan.h"

#include "GLFW/glfw3.h"

#include <iostream>







namespace Vulkan {



    namespace {

        struct PhysicalDeviceRating {
            int rating;
            uint32_t queueFamilyIndex;
            VkPresentModeKHR presentMode;
            VkSurfaceFormatKHR surfaceFormat;
            VkSurfaceCapabilitiesKHR surfaceCapabilities;
        };

        GLFWwindow* _window;
        VkInstance _instance;
        VkDebugUtilsMessengerEXT _debugMessenger;

        VkSurfaceKHR _surface;

        VkPhysicalDevice _physicalDevice;

        VkDevice _device;
        VkQueue _queue;
        VkPresentModeKHR _presentMode;
        VkSurfaceFormatKHR _surfaceFormat;

        VkRenderPass _renderPass;

        VkSwapchainKHR _swapchain;
        std::vector<VkImage> _swapchainImages;
        std::vector<VkImageView> _swapchainImageViews;
        std::vector<VkFramebuffer> _swapchainFramebuffers;

        void _createWindow();
        void _createInstance();
        void _createSurface();

        void _createDevice();
        PhysicalDeviceRating _ratePhysicalDevice(const VkPhysicalDevice physicalDevice);
        int _rateDeviceExtensions(const VkPhysicalDevice physicalDevice);
        int _rateDeviceQueueFamilies(const VkPhysicalDevice physicalDevice, uint32_t& queueFamilyIndex);
        int _rateDevicePresentMode(const VkPhysicalDevice physicalDevice, VkPresentModeKHR &presentMode);
        int _rateDeviceSurfaceFormat(const VkPhysicalDevice physicalDevice, VkSurfaceFormatKHR &surfaceFormat);
        int _rateDeviceProperties(const VkPhysicalDevice physicalDevice);

        void _createRenderPass();

        void _createFrameBuffers();

        static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
            return VK_FALSE;
        }
        const VkDebugUtilsMessengerCreateInfoEXT _debugMessengerCreateInfo{
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, //sType
            nullptr, //pNext
            0, //flags
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, //messageSeverity
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, //messageType
            _debugCallback, //pfnUserCallback
            nullptr//pUserData
        };

    }




    
    
    
    void init() {

        _createWindow();
        _createInstance();
        _createSurface();
        _createDevice();
        _createRenderPass();
        _createFrameBuffers();
        //createDescriptorSetLayout();
        //createGraphicsPipeline();
        //createCommandPool();
        //createVertexBuffer();
        //createIndexBuffer();
        //createUniformBuffers();
        //createDescriptorPool();
        //createDescriptorSets();
        //createCommandBuffers();
        //createSyncObjects();
    }



    void cleanup() {

        for (auto framebuffer : _swapchainFramebuffers) {
            vkDestroyFramebuffer(_device, framebuffer, nullptr);
        }
        for (auto imageView : _swapchainImageViews) {
            vkDestroyImageView(_device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);

        vkDestroyRenderPass(_device, _renderPass, nullptr);

        vkDestroyDevice(_device, nullptr);

        vkDestroySurfaceKHR(_instance, _surface, nullptr);

#ifdef ENABLE_VALIDATION_LAYERS
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(_instance, _debugMessenger, nullptr);
        }
#endif

        vkDestroyInstance(_instance, nullptr);

        glfwDestroyWindow(_window);
        glfwTerminate();

    }







    namespace {



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
#ifdef ENABLE_VALIDATION_LAYERS
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
            for (auto validationLayer : validationLayers) {
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
                VK_STRUCTURE_TYPE_APPLICATION_INFO, //sType
                nullptr, //pNext
                "particles", //pApplicationName
                VK_MAKE_VERSION(1, 0, 0), //applicationVersion
                nullptr, //pEngineName
                VK_MAKE_VERSION(1, 0, 0), //engineVersion
                VK_API_VERSION_1_2 //apiVersion
            };
            VkInstanceCreateInfo createInfo{
                VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, //sType
                pNext, //pNext
                0, //flags
                &appInfo, //pApplicationInfo
                static_cast<uint32_t>(validationLayers.size()), //enabledLayerCount
                validationLayers.data(), //ppEnabledLayerNames
                static_cast<uint32_t>(extensions.size()), //enabledExtensionCount
                extensions.data() //ppEnabledExtensionNames
            };
            if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
                throw std::runtime_error("Error: failed to create vulkan instance.");
            }

#ifdef ENABLE_VALIDATION_LAYERS
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
            std::vector<PhysicalDeviceRating> ratings(physicalDeviceCount);
            for (int i = 0; i < physicalDeviceCount; i++) {
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
            uint32_t queueFamilyIndex = ratings[bestPhysicalDeviceIndex].queueFamilyIndex;

            float queuePriority = 1.0;
            VkDeviceQueueCreateInfo deviceQueueCreateInfo{
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                queueFamilyIndex, //queueFamilyIndex
                1, //queueCount
                &queuePriority //pQueuePriorities
            };
            VkPhysicalDeviceFeatures deviceFeatures{};
            VkDeviceCreateInfo deviceCreateInfo{
                VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                1, //queueCreateInfoCount
                &deviceQueueCreateInfo, //pQueueCreateInfos
                static_cast<uint32_t>(validationLayers.size()), //enabledLayerCount
                validationLayers.data(), //ppEnabledLayerNames
                static_cast<uint32_t>(requiredDeviceExtensions.size()), //enabledExtensionCount
                requiredDeviceExtensions.data(),
                &deviceFeatures //pEnabledFeatures
            };
            if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device) != VK_SUCCESS) {
                throw std::runtime_error("Error: could not create device.");
            }
            vkGetDeviceQueue(_device, queueFamilyIndex, 0, &_queue);

        }



        PhysicalDeviceRating _ratePhysicalDevice(const VkPhysicalDevice physicalDevice) {

            PhysicalDeviceRating rating{
                0, 0, VK_PRESENT_MODE_FIFO_KHR, VK_FORMAT_UNDEFINED
            };
            int result;

            /* vvv array of functions instead? vvv */

            result = _rateDeviceExtensions(physicalDevice);
            if (result > 0) {
                rating.rating += result;
            } else {
                rating.rating = -1;
                return rating;
            }

            result = _rateDevicePresentMode(physicalDevice, rating.presentMode);
            if (result > 0) {
                rating.rating += result;
            } else {
                rating.rating = -1;
                return rating;
            }

            result = _rateDeviceSurfaceFormat(physicalDevice, rating.surfaceFormat);
            if (result > 0) {
                rating.rating += result;
            } else {
                rating.rating = -1;
                return rating;
            }

            result = _rateDeviceQueueFamilies(physicalDevice, rating.queueFamilyIndex);
            if (result > 0) {
                rating.rating += result;
            } else {
                rating.rating = -1;
                return rating;
            }

            result = _rateDeviceProperties(physicalDevice);
            if (result > 0) {
                rating.rating += result;
            } else {
                rating.rating = -1;
                return rating;
            }

            return rating;

        }



        int _rateDeviceExtensions(const VkPhysicalDevice physicalDevice) {

            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

            for (auto requiredExtension : requiredDeviceExtensions) {
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



        int _rateDeviceQueueFamilies(const VkPhysicalDevice physicalDevice, uint32_t& queueFamilyIndex) {

            uint32_t queueFamilyCount;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                // Improve queue ratings: include option for queues to be 'specialised'
                VkBool32 surfaceSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surface, &surfaceSupport);
                if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && surfaceSupport) {
                    queueFamilyIndex = i;
                    return 256;
                }
            }

            return -1;

        }



        int _rateDevicePresentMode(const VkPhysicalDevice physicalDevice, VkPresentModeKHR& presentMode) {

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
                    presentMode = mode;
                    return 64;
                }
            }

            presentMode = VK_PRESENT_MODE_FIFO_KHR;
            return 0;

        }



        int _rateDeviceSurfaceFormat(const VkPhysicalDevice physicalDevice, VkSurfaceFormatKHR& surfaceFormat) {

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
                    surfaceFormat = format;
                    return 64;
                }
            }

            surfaceFormat = formats[0];
            return 0;

        }



        int _rateDeviceProperties(const VkPhysicalDevice physicalDevice) {

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



        void _createRenderPass() {

            VkAttachmentDescription attachment{
                0, //flags
                _surfaceFormat.format, //format
                VK_SAMPLE_COUNT_1_BIT, //samples
                VK_ATTACHMENT_LOAD_OP_CLEAR, //loadOp
                VK_ATTACHMENT_STORE_OP_STORE, //storeOp
                VK_ATTACHMENT_LOAD_OP_DONT_CARE, //stencilLoadOp
                VK_ATTACHMENT_STORE_OP_DONT_CARE, //stencilStoreOp
                VK_IMAGE_LAYOUT_UNDEFINED, //initialLayout
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR //finalLayout
            };

            VkAttachmentReference reference{
                0, //attachment
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL //layout
            };
            VkSubpassDescription subpass{
                0, //flags
                VK_PIPELINE_BIND_POINT_GRAPHICS, //pipelineBindPoint
                0, //inputAttachmentCount
                nullptr, //pInputAttachments
                1, //colorAttachmentCount
                &reference, //pColorAttachments
                nullptr, //pResolveAttachments
                nullptr, //pDepthStencilAttachment
                0, //preserveAttachmentCount
                nullptr //pPreserveAttachments
            };

            VkSubpassDependency dependency{
                VK_SUBPASS_EXTERNAL, //srcSubpass
                0, //dstSubpass
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, //srcStageMask
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, //dstStageMask
                0, //srcAccessMask
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, //dstAccessMask
                0 //dependencyFlags
            };

            VkRenderPassCreateInfo createInfo{
                VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                1, //attachmentCount
                &attachment, //pAttachments
                1, //subpassCount
                &subpass, //pSubpasses
                1, //dependencyCount
                &dependency //pDependencies
            };

            if (vkCreateRenderPass(_device, &createInfo, nullptr, &_renderPass) != VK_SUCCESS) {
                throw std::runtime_error("Error: could not create render pass");
            }

        }



        void _createFrameBuffers() {

            VkSurfaceCapabilitiesKHR capabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &capabilities);

            VkExtent2D extent;
            if (capabilities.currentExtent.width != UINT32_MAX) {
                extent = capabilities.currentExtent;
            } else {
                int width, height;
                glfwGetFramebufferSize(_window, &width, &height);
                extent.width = width > capabilities.minImageExtent.width ? width : capabilities.minImageExtent.width;
                extent.width = extent.width < capabilities.maxImageExtent.width ? extent.width : capabilities.maxImageExtent.width;
                extent.height = height > capabilities.minImageExtent.height ? height : capabilities.minImageExtent.height;
                extent.height = extent.height < capabilities.maxImageExtent.height ? extent.height : capabilities.maxImageExtent.height;
            }

            uint32_t imageCount = 3;
            if (imageCount < capabilities.minImageCount) {
                imageCount = capabilities.minImageCount;
            }
            if (capabilities.maxImageCount > 0) {
                if (imageCount > capabilities.maxImageCount) {
                    imageCount = capabilities.maxImageCount;
                }
            }

            VkSwapchainCreateInfoKHR swapchainCreateInfo{
                VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, //sTypeVk
                nullptr, //pNext
                0, //flags
                _surface, //surface
                imageCount, //minImageCount
                _surfaceFormat.format, //imageFormat
                _surfaceFormat.colorSpace, //imageColorSpace
                extent, //imageExtent
                1, //imageArrayLayers
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, //imageUsage
                VK_SHARING_MODE_EXCLUSIVE, //imageSharingMode
                0, //queueFamilyIndexCount
                nullptr, //pQueueFamilyIndices
                capabilities.currentTransform, //preTransform
                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, //compositeAlpha
                _presentMode, //presentMode
                VK_TRUE, //clipped
                VK_NULL_HANDLE //oldSwapchain
            };
            if (vkCreateSwapchainKHR(_device, &swapchainCreateInfo, nullptr, &_swapchain) != VK_SUCCESS) {
                throw std::runtime_error("Error: could not create swapchain.");
            }

            vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
            _swapchainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainImages.data());

            _swapchainImageViews.resize(imageCount);
            VkImageViewCreateInfo imageViewCreateInfo{
                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                VK_NULL_HANDLE, //imageType
                VK_IMAGE_VIEW_TYPE_2D, //viewType
                _surfaceFormat.format, //format
                { //components
                    VK_COMPONENT_SWIZZLE_IDENTITY, //r
                    VK_COMPONENT_SWIZZLE_IDENTITY, //g
                    VK_COMPONENT_SWIZZLE_IDENTITY, //b
                    VK_COMPONENT_SWIZZLE_IDENTITY, //a
                },
                { //subresourceRange
                    VK_IMAGE_ASPECT_COLOR_BIT, //aspectMask
                    0, //baseMipLevel
                    1, //levelCount
                    0, //baseArrayLayer
                    1 //layerCount
                }
            };
            for (size_t i = 0; i < imageCount; i++) {
                imageViewCreateInfo.image = _swapchainImages[i];
                if (vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &_swapchainImageViews[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Error: failed to create image view.");
                }
            }

            _swapchainFramebuffers.resize(imageCount);
            VkFramebufferCreateInfo framebufferCreateInfo{
                VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                _renderPass, //renderPass
                1, //attachmentCount
                nullptr, //pAttachments
                extent.width, //width
                extent.height, //height
                1 //layers
            };
            for (size_t i = 0; i < imageCount; i++) {
                framebufferCreateInfo.pAttachments = &_swapchainImageViews[i];
                if (vkCreateFramebuffer(_device, &framebufferCreateInfo, nullptr, &_swapchainFramebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Error: failed to create framebuffer.");
                }
            }

        }



    }



}