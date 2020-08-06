#pragma once

#include "vulkan.h"
#include "glm/glm.hpp"



struct Particle {

    glm::vec3 r;
    glm::vec3 v;
    glm::vec3 colour;

    static const VkVertexInputBindingDescription bindingDescription;
    static const std::vector<VkVertexInputAttributeDescription> attributeDescription;

};