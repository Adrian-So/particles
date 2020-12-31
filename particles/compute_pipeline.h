#pragma once

#include "vulkan/vulkan.hpp"







namespace ComputePipeline {

    void initialise();
    void cleanup();

    extern VkPipelineLayout pipelineLayout;
    extern VkPipeline pipeline;

    extern VkCommandBuffer commandBuffer;

}