/**/



#include <iostream>

#include "vulkan.h"
#include "particle.h"
#include "mvp.h"
#include "graphics_pipeline.h"



void draw();







int main() {

    std::cout << "Hello! This project will totally not fail (:\n\n";

    try {

        Vulkan::initialise();
        Particle::createBuffer();
        ModelViewProjection::initialise();
        GraphicsPipeline::initialise();

        while (!glfwWindowShouldClose(Vulkan::window)) {
            glfwPollEvents();
            draw();
        }
        vkDeviceWaitIdle(Vulkan::device);

        GraphicsPipeline::cleanup();
        ModelViewProjection::cleanup();
        Particle::destroyBuffer();
        Vulkan::cleanup();

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}







void draw() {

    vkWaitForFences(Vulkan::device, 1, &GraphicsPipeline::inFlightFences[GraphicsPipeline::currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    if (vkAcquireNextImageKHR(Vulkan::device, GraphicsPipeline::swapchain, UINT64_MAX, GraphicsPipeline::imageAvailableSemaphores[GraphicsPipeline::currentFrame], VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to acquire swapchain image.");
    }
    if (GraphicsPipeline::imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(Vulkan::device, 1, &GraphicsPipeline::imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    GraphicsPipeline::imagesInFlight[imageIndex] = GraphicsPipeline::inFlightFences[GraphicsPipeline::currentFrame];

    ModelViewProjection::update(imageIndex);

    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &GraphicsPipeline::imageAvailableSemaphores[GraphicsPipeline::currentFrame],
        .pWaitDstStageMask = &waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &GraphicsPipeline::commandBuffers[imageIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &GraphicsPipeline::renderFinishedSemaphores[GraphicsPipeline::currentFrame],
    };
    vkResetFences(Vulkan::device, 1, &GraphicsPipeline::imagesInFlight[GraphicsPipeline::currentFrame]);
    if (vkQueueSubmit(Vulkan::queue, 1, &submitInfo, GraphicsPipeline::imagesInFlight[GraphicsPipeline::currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to submit draw command.");
    }

    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &GraphicsPipeline::renderFinishedSemaphores[GraphicsPipeline::currentFrame],
        .swapchainCount = 1,
        .pSwapchains = &GraphicsPipeline::swapchain,
        .pImageIndices = &imageIndex,
        .pResults = nullptr,
    };
    if (vkQueuePresentKHR(Vulkan::queue, &presentInfo) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to present swapchain image.");
    }

#ifdef FRAME_COUNTER_ENABLED
    std::cout << _frameCounter++ << std::endl;
#endif
    GraphicsPipeline::currentFrame = (GraphicsPipeline::currentFrame + 1) % Vulkan::MAX_FRAMES_IN_FLIGHT;

}