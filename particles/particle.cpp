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



void Particle::createBuffer() {

    size = particles.size();
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

    Vulkan::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, deviceMemory);
    Vulkan::copyBuffer(stagingBuffer, buffer, bufferSize);

    vkDestroyBuffer(Vulkan::device, stagingBuffer, nullptr);
    vkFreeMemory(Vulkan::device, stagingBufferMemory, nullptr);

}



void Particle::destroyBuffer() {

    vkDestroyBuffer(Vulkan::device, buffer, nullptr);
    vkFreeMemory(Vulkan::device, deviceMemory, nullptr);

}