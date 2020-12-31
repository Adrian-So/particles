#include "particle.h"

#include "vulkan.h"







static constexpr float sqrtHalf = 0.70710678118f;
static constexpr float sqrtThird = 0.57735026919f;
std::vector<Particle> particles{
    {{1.0f, 0.0f, 0.0f},                    {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-1.0f, 0.0f, 0.0f},                   {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, 1.0f, 0.0f},                    {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, -1.0f, 0.0f},                   {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, 0.0f, 1.0f},                    {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, 0.0f, -1.0f},                   {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtHalf, sqrtHalf, 0.0f},            {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtHalf, -sqrtHalf, 0.0f},           {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtHalf, sqrtHalf, 0.0f},           {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtHalf, -sqrtHalf, 0.0f},          {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, sqrtHalf, sqrtHalf},            {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, sqrtHalf, -sqrtHalf},           {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, -sqrtHalf, sqrtHalf},           {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, -sqrtHalf, -sqrtHalf},          {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtHalf, 0.0f, sqrtHalf},            {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtHalf, 0.0f, sqrtHalf},           {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtHalf, 0.0f, -sqrtHalf},           {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtHalf, 0.0f, -sqrtHalf},          {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtThird, sqrtThird, sqrtThird},     {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtThird, sqrtThird, -sqrtThird},    {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtThird, -sqrtThird, sqrtThird},    {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtThird, -sqrtThird, -sqrtThird},   {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtThird, sqrtThird, sqrtThird},    {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtThird, sqrtThird, -sqrtThird},   {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtThird, -sqrtThird, sqrtThird},   {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtThird, -sqrtThird, -sqrtThird},  {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
};







uint32_t Particle::size = 0;
VkBuffer Particle::buffer = VK_NULL_HANDLE;
VkDeviceMemory Particle::deviceMemory = VK_NULL_HANDLE;

const VkVertexInputBindingDescription Particle::bindingDescription = {
    .binding    = 0,
    .stride     = sizeof(Particle),
    .inputRate  = VK_VERTEX_INPUT_RATE_VERTEX,
};

const std::vector<VkVertexInputAttributeDescription> Particle::attributeDescription = {
    {
        .location   = 0,
        .binding    = 0,
        .format     = VK_FORMAT_R32G32B32_SFLOAT,
        .offset     = offsetof(Particle, r),
    }, {
        .location   = 1,
        .binding    = 0,
        .format     = VK_FORMAT_R32G32B32_SFLOAT,
        .offset     = offsetof(Particle, v),
    }, {
        .location   = 2,
        .binding    = 0,
        .format     = VK_FORMAT_R32G32B32_SFLOAT,
        .offset     = offsetof(Particle, colour),
    }
};

VkDescriptorSetLayout Particle::descriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorSet Particle::descriptorSets = VK_NULL_HANDLE;
VkDescriptorPool Particle::_descriptorPool = VK_NULL_HANDLE;







void Particle::initialise() {
    size = static_cast<uint32_t>(particles.size());
    _createBuffer();
    _createDescriptorSetLayout();
    _createDescriptorPool();
    _createDescriptorSet();
}



void Particle::cleanup() {
    vkDestroyDescriptorPool(Vulkan::device, _descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(Vulkan::device, descriptorSetLayout, nullptr);
    vkDestroyBuffer(Vulkan::device, buffer, nullptr);
    vkFreeMemory(Vulkan::device, deviceMemory, nullptr);
}







void Particle::_createBuffer() {

    VkDeviceSize bufferSize = sizeof(Particle) * size;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Vulkan::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    if (vkMapMemory(Vulkan::device, stagingBufferMemory, 0, bufferSize, 0, &data) != VK_SUCCESS) {
        throw std::runtime_error("Error: unable to map memory.");
    }
    memcpy(data, particles.data(), (size_t)bufferSize);
    vkUnmapMemory(Vulkan::device, stagingBufferMemory);

    Vulkan::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, deviceMemory);
    Vulkan::copyBuffer(stagingBuffer, buffer, bufferSize);

    vkDestroyBuffer(Vulkan::device, stagingBuffer, nullptr);
    vkFreeMemory(Vulkan::device, stagingBufferMemory, nullptr);

}







void Particle::_createDescriptorSetLayout() {

    VkDescriptorSetLayoutBinding layoutBinding{
        .binding            = 0,
        .descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount    = 1,
        .stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT,
        .pImmutableSamplers = nullptr,
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext          = nullptr,
        .flags          = 0,
        .bindingCount   = 1,
        .pBindings      = &layoutBinding,
    };
    if (vkCreateDescriptorSetLayout(Vulkan::device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create descriptor set layout.");
    }

}



void Particle::_createDescriptorPool() {

    VkDescriptorPoolSize descriptorPoolSize{
        .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount    = 1,
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext          = nullptr,
        .flags          = 0,
        .maxSets        = 1,
        .poolSizeCount  = 1,
        .pPoolSizes     = &descriptorPoolSize,
    };
    if (vkCreateDescriptorPool(Vulkan::device, &descriptorPoolCreateInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create descriptor pool.");
    }

}



void Particle::_createDescriptorSet() {

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = _descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &descriptorSetLayout,
    };
    if (vkAllocateDescriptorSets(Vulkan::device, &descriptorSetAllocateInfo, &descriptorSets) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create descriptor sets.");
    }

    VkDescriptorBufferInfo bufferInfo{
        .buffer = buffer,
        .offset = 0,
        .range  = sizeof(Particle) * particles.size(),
    };
    VkWriteDescriptorSet descriptorWrite{
        .sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext              = nullptr,
        .dstSet             = descriptorSets,
        .dstBinding         = 0,
        .dstArrayElement    = 0,
        .descriptorCount    = 1,
        .descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo         = nullptr,
        .pBufferInfo        = &bufferInfo,
        .pTexelBufferView   = nullptr,
    };
    vkUpdateDescriptorSets(Vulkan::device, 1, &descriptorWrite, 0, nullptr);

}