#include "mvp.h"

#include <chrono>

#include <glm/gtc/matrix_transform.hpp>







VkDescriptorSetLayout ModelViewProjection::descriptorSetLayout = VK_NULL_HANDLE;
std::vector<VkDescriptorSet> ModelViewProjection::descriptorSets(1);
VkPhysicalDevice ModelViewProjection::_physicalDevice = VK_NULL_HANDLE;
VkDevice ModelViewProjection::_device = VK_NULL_HANDLE;
uint32_t ModelViewProjection::_num = 0;
VkExtent2D ModelViewProjection::_extent{ 0, 0 };
std::vector<VkBuffer> ModelViewProjection::_buffers(1);
std::vector<VkDeviceMemory> ModelViewProjection::_deviceMemories(1);
VkDescriptorPool ModelViewProjection::_descriptorPool = VK_NULL_HANDLE;







void ModelViewProjection::setParameters(const VkPhysicalDevice physicalDevice, const VkDevice device, const uint32_t num, const VkExtent2D extent) {
    _physicalDevice = physicalDevice;
    _device = device;
    _num = num;
    _extent = extent;
}







VkResult ModelViewProjection::createBuffers() {

    VkDeviceSize bufferSize = sizeof(ModelViewProjection);

    _buffers.resize(_num);
    _deviceMemories.resize(_num);

    VkBufferCreateInfo bufferCreateInfo{
        .sType                  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .size                   = bufferSize,
        .usage                  = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode            = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount  = 0,
        .pQueueFamilyIndices    = nullptr,
    };

    VkMemoryAllocateInfo memoryAllocateInfo{
        .sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext              = nullptr,
        .allocationSize     = 0,
        .memoryTypeIndex    = 0,
    };

    for (uint32_t i = 0; i < _num; i++) {

        if (vkCreateBuffer(_device, &bufferCreateInfo, nullptr, &_buffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to create model view projection buffer.");
        }

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(_device, _buffers[i], &memoryRequirements);
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (vkAllocateMemory(_device, &memoryAllocateInfo, nullptr, &_deviceMemories[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to allocate vertex buffer memory.");
        }

        vkBindBufferMemory(_device, _buffers[i], _deviceMemories[i], 0);

    }

    return VK_SUCCESS;

}







void ModelViewProjection::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{
        .binding            = 0,
        .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount    = 1,
        .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr,
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext          = nullptr,
        .flags          = 0,
        .bindingCount   = 1,
        .pBindings      = &uboLayoutBinding,
    };
    if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create descriptor set layout.");
    }
}







void ModelViewProjection::createDescriptorPool() {
    VkDescriptorPoolSize descriptorPoolSize{
        .type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount    = _num,
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext          = nullptr,
        .flags          = 0,
        .maxSets        = _num,
        .poolSizeCount  = 1,
        .pPoolSizes     = &descriptorPoolSize,
    };
    if (vkCreateDescriptorPool(_device, &descriptorPoolCreateInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create descriptor pool.");
    }
}







void ModelViewProjection::createDescriptorSet() {

    descriptorSets.resize(_num);

    std::vector<VkDescriptorSetLayout> layouts(_num, descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = _descriptorPool,
        .descriptorSetCount = _num,
        .pSetLayouts        = layouts.data(),
    };
    if (vkAllocateDescriptorSets(_device, &descriptorSetAllocateInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create descriptor sets.");
    }

    VkDescriptorBufferInfo bufferInfo{
        .buffer = VK_NULL_HANDLE,
        .offset = 0,
        .range = sizeof(ModelViewProjection),
    };
    VkWriteDescriptorSet descriptorWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &bufferInfo,
        .pTexelBufferView = nullptr,
    };
    for (uint32_t i = 0; i < _num; i++) {
        bufferInfo.buffer = _buffers[i];
        descriptorWrite.dstSet = descriptorSets[i];
        vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);
    }

}







uint32_t ModelViewProjection::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Error: failed to find suitable memory type.");
}







void ModelViewProjection::update(uint32_t imageNum) {

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    ModelViewProjection matrices;
    matrices.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    matrices.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    matrices.proj = glm::perspective(glm::radians(45.0f), _extent.width / (float)_extent.height, 0.1f, 10.0f);
    matrices.proj[1][1] *= -1;

    void* data;
    vkMapMemory(_device, _deviceMemories[imageNum], 0, sizeof(ModelViewProjection), 0, &data);
    memcpy(data, &matrices, sizeof(ModelViewProjection));
    vkUnmapMemory(_device, _deviceMemories[imageNum]);

}