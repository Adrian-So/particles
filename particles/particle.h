#pragma once

#include <vulkan/vulkan.hpp>
#include "glm/glm.hpp"







struct Particle {

    alignas(16) glm::vec3 r;
    alignas(16) glm::vec3 v;
    alignas(16) glm::vec3 colour;

    static void initialise();
    static void cleanup();

    static uint32_t size;
    static VkBuffer buffer;
    static VkDeviceMemory deviceMemory;

    static const VkVertexInputBindingDescription bindingDescription;
    static const std::vector<VkVertexInputAttributeDescription> attributeDescription;

    static VkDescriptorSetLayout descriptorSetLayout;
    static VkDescriptorSet descriptorSets;

private:

    static void _createBuffer();

    static void _createDescriptorSetLayout();
    static void _createDescriptorPool();
    static void _createDescriptorSet();

    static VkDescriptorPool _descriptorPool;

};