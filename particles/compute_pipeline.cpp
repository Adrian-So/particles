#include "compute_pipeline.h"

#include "vulkan.h"
#include "particle.h"

#include <fstream>



namespace ComputePipeline {

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    VkCommandBuffer commandBuffer;

    static void _createPipeline();
    static void _createCommandBuffer();

}







void ComputePipeline::initialise() {

    _createPipeline();
    _createCommandBuffer();

}



void ComputePipeline::cleanup() {

    vkDestroyPipeline(Vulkan::device, pipeline, nullptr);
    vkDestroyPipelineLayout(Vulkan::device, pipelineLayout, nullptr);

}







void ComputePipeline::_createPipeline() {

    std::ifstream file("../shaders/comp.spv", std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error: failed to open compute shader spv.");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    VkShaderModuleCreateInfo shaderModuleCreateInfo{
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0,
        .codeSize = fileSize,
        .pCode    = reinterpret_cast<const uint32_t*>(buffer.data()),
    };
    VkShaderModule computeShaderModule;
    if (vkCreateShaderModule(Vulkan::device, &shaderModuleCreateInfo, nullptr, &computeShaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create compute shader module.");
    }

    VkPipelineLayoutCreateInfo layoutCreateInfo{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .setLayoutCount         = 1,
        .pSetLayouts            = &Particle::descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = nullptr,
    };
    if (vkCreatePipelineLayout(Vulkan::device, &layoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create compute pipeline layout.");
    }

    VkComputePipelineCreateInfo computePipelineCreateInfo{
        .sType                   = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .stage                   = {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0,
            .stage                   = VK_SHADER_STAGE_COMPUTE_BIT,
            .module                  = computeShaderModule,
            .pName                   = "main",
            .pSpecializationInfo     = nullptr },
        .layout                  = pipelineLayout,
        .basePipelineHandle      = VK_NULL_HANDLE,
        .basePipelineIndex       = -1,
    };
    if (vkCreateComputePipelines(Vulkan::device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create compute pipeline.");
    }

    vkDestroyShaderModule(Vulkan::device, computeShaderModule, nullptr);

}







void ComputePipeline::_createCommandBuffer() {

    VkCommandBufferAllocateInfo allocateInfo{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = Vulkan::commandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    if (vkAllocateCommandBuffers(Vulkan::device, &allocateInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to allocate compute command buffer.");
    }

    VkCommandBufferBeginInfo beginInfo{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = 0,
        .pInheritanceInfo = nullptr,
    };
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to begin recording compute command buffer.");
    }
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &Particle::descriptorSets, 0, nullptr);
    vkCmdDispatch(commandBuffer, 26, 1, 1);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to record compute command buffer.");
    }

}