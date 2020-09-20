#pragma once

#define GLM_FORCE_RADIANS

#include <vulkan/vulkan.hpp>
#include "glm/glm.hpp"







struct ModelViewProjection {

    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;

    static void initialise();
    static void cleanup();

    static void update(uint32_t imageNum);

    static VkDescriptorSetLayout descriptorSetLayout;
    static std::vector<VkDescriptorSet> descriptorSets;

private:

    static void _createBuffers();
    static void _createDescriptorSetLayout();
    static void _createDescriptorPool();
    static void _createDescriptorSet();

    static std::vector<VkBuffer> _buffers;
    static std::vector<VkDeviceMemory> _deviceMemories;

    static VkDescriptorPool _descriptorPool;

};