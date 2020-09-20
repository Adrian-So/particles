#pragma once

#include "vulkan/vulkan.hpp"



#ifndef NDEBUG
//#define FRAME_COUNTER_ENABLED
#endif







namespace GraphicsPipeline {

    void initialise();
    void cleanup();

    extern VkRenderPass renderPass;
    extern VkPipelineLayout pipelineLayout;
    extern VkPipeline graphicsPipeline;

    extern VkSwapchainKHR swapchain;
    extern std::vector<VkImage> swapchainImages;
    extern std::vector<VkImageView> swapchainImageViews;
    extern std::vector<VkFramebuffer> swapchainFramebuffers;

    extern std::vector<VkCommandBuffer> commandBuffers;

    extern std::vector<VkSemaphore> imageAvailableSemaphores;
    extern std::vector<VkSemaphore> renderFinishedSemaphores;
    extern std::vector<VkFence> inFlightFences;
    extern std::vector<VkFence> imagesInFlight;
    extern size_t currentFrame;

#ifdef FRAME_COUNTER_ENABLED
    static int _frameCounter = 0;
#endif

}