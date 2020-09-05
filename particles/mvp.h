#pragma once

#define GLM_FORCE_RADIANS

#include <vulkan/vulkan.hpp>
#include "glm/glm.hpp"



struct ModelViewProjection {

    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;

    static void setParameters(const VkPhysicalDevice physicalDevice, const VkDevice device, const uint32_t num, const VkExtent2D extent);

    static VkResult createBuffers();

    static void createDescriptorSetLayout();
    static void createDescriptorPool();
    static void createDescriptorSet();

    static void update(uint32_t imageNum);

    static VkDescriptorSetLayout descriptorSetLayout;
    static std::vector<VkDescriptorSet> descriptorSets;

private:

    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static VkPhysicalDevice _physicalDevice;
    static VkDevice _device;
    static uint32_t _num;
    static VkExtent2D _extent;

    static std::vector<VkBuffer> _buffers;
    static std::vector<VkDeviceMemory> _deviceMemories;

    static VkDescriptorPool _descriptorPool;

};