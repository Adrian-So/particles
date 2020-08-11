#include "vulkan.h"

#include "GLFW/glfw3.h"

#include "particle.h"

#include <iostream>
#include <fstream>







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
        uint32_t _queueFamilyIndex;
        VkQueue _queue;
        VkPresentModeKHR _presentMode;
        VkSurfaceFormatKHR _surfaceFormat;
        VkExtent2D _extent;
        uint32_t _imageCount;

        VkRenderPass _renderPass;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _graphicsPipeline;

        VkSwapchainKHR _swapchain;
        std::vector<VkImage> _swapchainImages;
        std::vector<VkImageView> _swapchainImageViews;
        std::vector<VkFramebuffer> _swapchainFramebuffers;

        VkCommandPool _commandPool;

        VkBuffer _particlesBuffer;
        VkDeviceMemory _particlesDeviceMemory;

        std::vector<VkCommandBuffer> _commandBuffers;

        void _createWindow();
        void _createInstance();
        void _createSurface();

        void _createDevice();
        PhysicalDeviceRating _ratePhysicalDevice(const VkPhysicalDevice physicalDevice);
        int _rateDeviceExtensions(const VkPhysicalDevice physicalDevice);
        int _rateDeviceQueueFamilies(const VkPhysicalDevice physicalDevice, uint32_t& queueFamilyIndex);
        int _rateDevicePresentMode(const VkPhysicalDevice physicalDevice, VkPresentModeKHR& presentMode);
        int _rateDeviceSurfaceFormat(const VkPhysicalDevice physicalDevice, VkSurfaceFormatKHR& surfaceFormat);
        int _rateDeviceProperties(const VkPhysicalDevice physicalDevice);

        void _createRenderPass();
        void _createGraphicsPipeline();
        VkShaderModule _createShaderModule(const std::string& fileName);

        void _createFramebuffers();

        void _createCommandPool();

        void _createParticlesBuffer();
        void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        uint32_t _findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void _copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void _createCommandBuffers();

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
        _createGraphicsPipeline();
        _createFramebuffers();
        //createDescriptorSetLayout();
        _createCommandPool();
        _createParticlesBuffer();
        //createUniformBuffers();
        //createDescriptorPool();
        //createDescriptorSets();
        _createCommandBuffers();
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

        vkFreeCommandBuffers(_device, _commandPool, _commandBuffers.size(), _commandBuffers.data());

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
#ifdef VALIDATION_LAYERS_ENABLED
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
            std::vector<PhysicalDeviceRating> ratings(physicalDeviceCount);
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

            float queuePriority = 1.0;
            VkDeviceQueueCreateInfo deviceQueueCreateInfo{
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                _queueFamilyIndex, //queueFamilyIndex
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
            vkGetDeviceQueue(_device, _queueFamilyIndex, 0, &_queue);

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



        void _createFramebuffers() {

            VkSurfaceCapabilitiesKHR capabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &capabilities);

            if (capabilities.currentExtent.width != UINT32_MAX) {
                _extent = capabilities.currentExtent;
            } else {
                int width, height;
                glfwGetFramebufferSize(_window, &width, &height);
                _extent.width = width > capabilities.minImageExtent.width ? width : capabilities.minImageExtent.width;
                _extent.width = _extent.width < capabilities.maxImageExtent.width ? _extent.width : capabilities.maxImageExtent.width;
                _extent.height = height > capabilities.minImageExtent.height ? height : capabilities.minImageExtent.height;
                _extent.height = _extent.height < capabilities.maxImageExtent.height ? _extent.height : capabilities.maxImageExtent.height;
            }

            _imageCount = 3;
            if (_imageCount < capabilities.minImageCount) {
                _imageCount = capabilities.minImageCount;
            }
            if (capabilities.maxImageCount > 0) {
                if (_imageCount > capabilities.maxImageCount) {
                    _imageCount = capabilities.maxImageCount;
                }
            }

            VkSwapchainCreateInfoKHR swapchainCreateInfo{
                VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, //sType
                nullptr, //pNext
                0, //flags
                _surface, //surface
                _imageCount, //minImageCount
                _surfaceFormat.format, //imageFormat
                _surfaceFormat.colorSpace, //imageColorSpace
                _extent, //imageExtent
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
                throw std::runtime_error("Error: failed to create swapchain.");
            }

            vkGetSwapchainImagesKHR(_device, _swapchain, &_imageCount, nullptr);
            _swapchainImages.resize(_imageCount);
            vkGetSwapchainImagesKHR(_device, _swapchain, &_imageCount, _swapchainImages.data());

            _swapchainImageViews.resize(_imageCount);
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
            for (size_t i = 0; i < _imageCount; i++) {
                imageViewCreateInfo.image = _swapchainImages[i];
                if (vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &_swapchainImageViews[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Error: failed to create image view.");
                }
            }

            _swapchainFramebuffers.resize(_imageCount);
            VkFramebufferCreateInfo framebufferCreateInfo{
                VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                _renderPass, //renderPass
                1, //attachmentCount
                nullptr, //pAttachments
                _extent.width, //width
                _extent.height, //height
                1 //layers
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
                    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, //sType
                    nullptr, //pNext
                    0, //flags
                    VK_SHADER_STAGE_VERTEX_BIT, //stage
                    vertexShaderModule, //module
                    "main", //pName
                    nullptr //pSpecializaionInfo
                }, {
                    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, //sType
                    nullptr, //pNext
                    0, //flags
                    VK_SHADER_STAGE_FRAGMENT_BIT, //stage
                    fragmentShaderModule, //module
                    "main", //pName
                    nullptr //pSpecializaionInfo
                }
            };

            VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                1, //vertexBindingDescriptionCount
                &Particle::bindingDescription, //pVertexBindingDescription
                static_cast<uint32_t>(Particle::attributeDescription.size()), //vertexAttributeDescriptionCount
                Particle::attributeDescription.data() //pVertexAttributeDescriptions
            };

            VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                VK_PRIMITIVE_TOPOLOGY_POINT_LIST, //topology
                VK_FALSE //primitiveRestartEnable
            };

            /*VkPipelineTessellationStateCreateInfo*/

            VkViewport viewport{
                0.0f, //x
                0.0f, //y
                (float)_extent.width, //width
                (float)_extent.height, //height
                0.0f, //minDepth
                1.0f //maxDepth
            };
            VkRect2D scissors{
                { //offset
                    0, //x
                    0 //y
                },
                _extent //extent
            };
            VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                1, //viewportCount
                &viewport, //pViewports
                1, //scissorCount
                &scissors //pScissors
            };

            VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                VK_FALSE, //depthClampEnable
                VK_FALSE, //rasterizerDiscardEnable
                VK_POLYGON_MODE_FILL, //polygonMode
                VK_CULL_MODE_BACK_BIT, //cullMode
                VK_FRONT_FACE_COUNTER_CLOCKWISE, //frontFace
                VK_FALSE, //depthBiasEnable
                0.0f, //depthBiasConstantFactor
                0.0f, //depthBiasClamp
                0.0f, //depthBiasSlopeFactor
                1.0f //lineWidth
            };

            VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                VK_SAMPLE_COUNT_1_BIT, //rasterizationSamples
                VK_FALSE, //sampleShadingEnable
                1.0f, //minSampleShading
                nullptr, //pSampleMask
                VK_FALSE, //alphaToCoverageEnable
                VK_FALSE //alphaToOneEnable
            };

            /*VkPipelineDepthStencilStateCreateInfo*/

            VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
                VK_FALSE, //blendEnable
                VK_BLEND_FACTOR_ONE, //srcColorBlendFactor
                VK_BLEND_FACTOR_ZERO, //dstColorBlendFactor
                VK_BLEND_OP_ADD, //colorBlendOp
                VK_BLEND_FACTOR_ONE, //srcAlphaBlendFactor
                VK_BLEND_FACTOR_ZERO, //dstAlphaBlendFactor
                VK_BLEND_OP_ADD, //alphaBlendOp
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT //colorWriteMask
            };
            VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                VK_FALSE, //logicOpEnable
                VK_LOGIC_OP_COPY, //logicOp
                1, //attachmentCount
                &colorBlendAttachmentState, //pAttachments
                { 0.0f, 0.0f, 0.0f, 0.0f } //blendConstants[4]
            };

            /*VkDynamicState dynamicState{
            };*/
            VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                0, //dynamicStateCount
                nullptr //pDynamicStates
            };

            VkPipelineLayoutCreateInfo layoutCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                0, //setLayoutCount
                nullptr, //pSetLayouts
                0, //pushConstantRangeCount
                nullptr //pPushConstantRanges
            };
            if (vkCreatePipelineLayout(_device, &layoutCreateInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("Error: failed to create pipeline layout.");
            }

            VkGraphicsPipelineCreateInfo pipelineCreateInfo{
                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                2, //stageCount
                shaderStageCreateInfos, //pStages
                &vertexInputStateCreateInfo, //pVertexInputState
                &inputAssemblyStateCreateInfo, //pInputAssemblyState
                nullptr, //pTessellationState
                &viewportStateCreateInfo, //pViewportState
                &rasterizationStateCreateInfo, //pRasterizationState
                &multisampleStateCreateInfo, //pMultisampleState
                nullptr, //pDepthStencilState
                &colorBlendStateCreateInfo, //pColorBlendState
                &dynamicStateCreateInfo, //pDynamicState
                _pipelineLayout,
                _renderPass, //renderPass
                0, //subpass
                VK_NULL_HANDLE, //basePipelineHandle
                -1 //basePipelineIndex
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
                VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                fileSize, //codeSize
                reinterpret_cast<const uint32_t*>(buffer.data()) //pCode
            };
            VkShaderModule shaderModule;
            if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                throw std::runtime_error("Error: failed to create shader module.");
            }

            return shaderModule;

        }



        void _createCommandPool() {

            VkCommandPoolCreateInfo createInfo{
                VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, //sType
                nullptr, //pNext
                0, //flags
                _queueFamilyIndex //queueFamilyIndex
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
            vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, particles.data(), (size_t)bufferSize);
            vkUnmapMemory(_device, stagingBufferMemory);

            _createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _particlesBuffer, _particlesDeviceMemory);
            _copyBuffer(stagingBuffer, _particlesBuffer, bufferSize);

            vkDestroyBuffer(_device, stagingBuffer, nullptr);
            vkFreeMemory(_device, stagingBufferMemory, nullptr);

        }
        
        
        
        void _createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create vertex buffer.");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = _findMemoryType(memRequirements.memoryTypeBits, properties);
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

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = _commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
            vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(_queue);

            vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);

        }



        void _createCommandBuffers() {

            _commandBuffers.resize(_imageCount);

            VkCommandBufferAllocateInfo commandBufferAllocateInfo{
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, //sType
                nullptr, //pNext
                _commandPool, //commandPool
                VK_COMMAND_BUFFER_LEVEL_PRIMARY, //level
                _imageCount //commandBufferCount
            };
            if (vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, _commandBuffers.data()) != VK_SUCCESS) {
                throw std::runtime_error("Error: failed to allocate command buffers.");
            }

            VkCommandBufferBeginInfo commandBufferBeginInfo{
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, //sType
                nullptr, //pNext
                0, //flags
                nullptr //pInheritanceInfo
            };

            VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

            VkRenderPassBeginInfo renderPassBeginInfo{
                VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, //sType
                nullptr, //pNext
                _renderPass, //renderPass
                VK_NULL_HANDLE, //framebuffer
                { 0, 0 }, //renderArea
                1, //clearValueCount
                &clearColor //pClearValues
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
                    vkCmdDraw(_commandBuffers[i], particles.size(), 1, 0, 0);
                vkCmdEndRenderPass(_commandBuffers[i]);
                if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Error: failed to record command buffer.");
                }
            }

        }



    }



}