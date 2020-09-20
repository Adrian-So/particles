#pragma once

#include <vulkan/vulkan.hpp>
#include "glm/glm.hpp"







struct Particle {

    glm::vec3 r;
    glm::vec3 v;
    glm::vec3 colour;

    static void createBuffer();
    static void destroyBuffer();

    static uint32_t size;
    static VkBuffer buffer;
    static VkDeviceMemory deviceMemory;
    static const VkVertexInputBindingDescription bindingDescription;
    static const std::vector<VkVertexInputAttributeDescription> attributeDescription;

};