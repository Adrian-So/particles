#include "graphics_pipeline.h"

#include "vulkan.h"
#include "particle.h"
#include "mvp.h"

#include <fstream>



static void _createRenderPass();
static void _createGraphicsPipeline();
static void _createSwapchain();
static void _createCommandBuffers();
static void _createSyncObjects();
static VkShaderModule _createShaderModule(const std::string& fileName);



namespace GraphicsPipeline {

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame;

}







void GraphicsPipeline::initialise() {
    _createRenderPass();
    _createGraphicsPipeline();
    _createSwapchain();
    _createCommandBuffers();
    _createSyncObjects();
}



void GraphicsPipeline::cleanup() {

    for (auto i = 0; i < Vulkan::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(Vulkan::device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(Vulkan::device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(Vulkan::device, inFlightFences[i], nullptr);
    }

    for (auto framebuffer : swapchainFramebuffers) {
        vkDestroyFramebuffer(Vulkan::device, framebuffer, nullptr);
    }
    for (auto imageView : swapchainImageViews) {
        vkDestroyImageView(Vulkan::device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(Vulkan::device, swapchain, nullptr);

    vkFreeCommandBuffers(Vulkan::device, Vulkan::commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    vkDestroyPipeline(Vulkan::device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(Vulkan::device, pipelineLayout, nullptr);
    vkDestroyRenderPass(Vulkan::device, renderPass, nullptr);

}







void _createRenderPass() {

    VkAttachmentDescription attachment{
        .flags = 0,
        .format = Vulkan::surfaceFormat.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference reference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpass{
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &reference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0,
    };

    VkRenderPassCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    if (vkCreateRenderPass(Vulkan::device, &createInfo, nullptr, &(GraphicsPipeline::renderPass)) != VK_SUCCESS) {
        throw std::runtime_error("Error: could not create render pass.");
    }

}



void _createGraphicsPipeline() {

    VkShaderModule vertexShaderModule =
#ifdef NDEBUG
        _createShaderModule("../../shaders/vert.spv");
#else
        _createShaderModule("../shaders/vert.spv");
#endif
    VkShaderModule fragmentShaderModule =
#ifdef NDEBUG
        _createShaderModule("../../shaders/frag.spv");
#else
        _createShaderModule("../shaders/frag.spv");
#endif
    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[]{
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr,
        }, {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr,
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &Particle::bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(Particle::attributeDescription.size()),
        .pVertexAttributeDescriptions = Particle::attributeDescription.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    /*VkPipelineTessellationStateCreateInfo*/

    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)Vulkan::extent.width,
        .height = (float)Vulkan::extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    VkRect2D scissors{
        .offset = {
            .x = 0,
            .y = 0, },
        .extent = Vulkan::extent,
    };
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissors,
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    /*VkPipelineDepthStencilStateCreateInfo*/

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState,
        .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    /*VkDynamicState dynamicState{
    };*/
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = 0,
        .pDynamicStates = nullptr,
    };

    VkPipelineLayoutCreateInfo layoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &ModelViewProjection::descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };
    if (vkCreatePipelineLayout(Vulkan::device, &layoutCreateInfo, nullptr, &GraphicsPipeline::pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create pipeline layout.");
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStageCreateInfos,
        .pVertexInputState = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = GraphicsPipeline::pipelineLayout,
        .renderPass = GraphicsPipeline::renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };
    if (vkCreateGraphicsPipelines(Vulkan::device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &GraphicsPipeline::graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create graphics pipeline.");
    }

    vkDestroyShaderModule(Vulkan::device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(Vulkan::device, fragmentShaderModule, nullptr);

}



VkShaderModule _createShaderModule(const std::string& fileName) {

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
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = fileSize,
        .pCode = reinterpret_cast<const uint32_t*>(buffer.data()),
    };
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(Vulkan::device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create shader module.");
    }

    return shaderModule;

}



void _createSwapchain() {

    VkSwapchainCreateInfoKHR swapchainCreateInfo{
        .sType                  = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext                  = nullptr,
        .flags                  = 0,
        .surface                = Vulkan::surface,
        .minImageCount          = Vulkan::imageCount,
        .imageFormat            = Vulkan::surfaceFormat.format,
        .imageColorSpace        = Vulkan::surfaceFormat.colorSpace,
        .imageExtent            = Vulkan::extent,
        .imageArrayLayers       = 1,
        .imageUsage             = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode       = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount  = 0,
        .pQueueFamilyIndices    = nullptr,
        .preTransform           = Vulkan::preTransform,
        .compositeAlpha         = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode            = Vulkan::presentMode,
        .clipped                = VK_TRUE,
        .oldSwapchain           = VK_NULL_HANDLE,
    };
    if (vkCreateSwapchainKHR(Vulkan::device, &swapchainCreateInfo, nullptr, &GraphicsPipeline::swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create swapchain.");
    }

    vkGetSwapchainImagesKHR(Vulkan::device, GraphicsPipeline::swapchain, &Vulkan::imageCount, nullptr);
    GraphicsPipeline::swapchainImages.resize(Vulkan::imageCount);
    vkGetSwapchainImagesKHR(Vulkan::device, GraphicsPipeline::swapchain, &Vulkan::imageCount, GraphicsPipeline::swapchainImages.data());

    GraphicsPipeline::swapchainImageViews.resize(Vulkan::imageCount);
    VkImageViewCreateInfo imageViewCreateInfo{
        .sType              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .image              = VK_NULL_HANDLE,
        .viewType           = VK_IMAGE_VIEW_TYPE_2D,
        .format             = Vulkan::surfaceFormat.format,
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
    for (size_t i = 0; i < Vulkan::imageCount; i++) {
        imageViewCreateInfo.image = GraphicsPipeline::swapchainImages[i];
        if (vkCreateImageView(Vulkan::device, &imageViewCreateInfo, nullptr, &GraphicsPipeline::swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to create image view.");
        }
    }

    GraphicsPipeline::swapchainFramebuffers.resize(Vulkan::imageCount);
    VkFramebufferCreateInfo framebufferCreateInfo{
        .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext           = nullptr,
        .flags           = 0,
        .renderPass      = GraphicsPipeline::renderPass,
        .attachmentCount = 1,
        .pAttachments    = nullptr,
        .width           = Vulkan::extent.width,
        .height          = Vulkan::extent.height,
        .layers          = 1,
    };
    for (size_t i = 0; i < Vulkan::imageCount; i++) {
        framebufferCreateInfo.pAttachments = &GraphicsPipeline::swapchainImageViews[i];
        if (vkCreateFramebuffer(Vulkan::device, &framebufferCreateInfo, nullptr, &GraphicsPipeline::swapchainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to create framebuffer.");
        }
    }

}



void _createCommandBuffers() {

    GraphicsPipeline::commandBuffers.resize(Vulkan::imageCount);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = Vulkan::commandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = Vulkan::imageCount,
    };
    if (vkAllocateCommandBuffers(Vulkan::device, &commandBufferAllocateInfo, GraphicsPipeline::commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to allocate command buffers.");
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = 0,
        .pInheritanceInfo = nullptr,
    };

    VkClearValue clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
    VkRenderPassBeginInfo renderPassBeginInfo{
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext           = nullptr,
        .renderPass      = GraphicsPipeline::renderPass,
        .framebuffer     = VK_NULL_HANDLE,
        .renderArea      = {
            .offset          = { 0, 0 },
            .extent          = Vulkan::extent },
        .clearValueCount = 1,
        .pClearValues    = &clearColour,
    };

    VkDeviceSize offset = 0;

    for (size_t i = 0; i < Vulkan::imageCount; i++) {
        if (vkBeginCommandBuffer(GraphicsPipeline::commandBuffers[i], &commandBufferBeginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to begin recording command buffer.");
        }
        renderPassBeginInfo.framebuffer = GraphicsPipeline::swapchainFramebuffers[i];
        vkCmdBeginRenderPass(GraphicsPipeline::commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(GraphicsPipeline::commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline::graphicsPipeline);
        vkCmdBindDescriptorSets(GraphicsPipeline::commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline::pipelineLayout, 0, 1, &ModelViewProjection::descriptorSets[i], 0, nullptr);
        vkCmdBindVertexBuffers(GraphicsPipeline::commandBuffers[i], 0, 1, &Particle::buffer, &offset);
        vkCmdDraw(GraphicsPipeline::commandBuffers[i], Particle::size, 1, 0, 0);
        vkCmdEndRenderPass(GraphicsPipeline::commandBuffers[i]);
        if (vkEndCommandBuffer(GraphicsPipeline::commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to record command buffer.");
        }
    }

}



void _createSyncObjects() {

    GraphicsPipeline::imageAvailableSemaphores.resize(Vulkan::MAX_FRAMES_IN_FLIGHT);
    GraphicsPipeline::renderFinishedSemaphores.resize(Vulkan::MAX_FRAMES_IN_FLIGHT);
    GraphicsPipeline::inFlightFences.resize(Vulkan::MAX_FRAMES_IN_FLIGHT);
    GraphicsPipeline::imagesInFlight.resize(Vulkan::imageCount, VK_NULL_HANDLE);

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

    for (size_t i = 0; i < Vulkan::MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(Vulkan::device, &semaphoreCreateInfo, nullptr, &GraphicsPipeline::imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(Vulkan::device, &semaphoreCreateInfo, nullptr, &GraphicsPipeline::renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(Vulkan::device, &fenceCreateInfo, nullptr, &GraphicsPipeline::inFlightFences[i]) != VK_SUCCESS
            ) {
            throw std::runtime_error("Error: failed to create synchronisation objects.");
        }
    }

}