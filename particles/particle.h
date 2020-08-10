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






constexpr float sqrtHalf = 0.70710678118f;
constexpr float sqrtThird = 0.57735026919f;
const std::vector<Particle> particles{
    {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtHalf, sqrtHalf, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtHalf, -sqrtHalf, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtHalf, sqrtHalf, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtHalf, -sqrtHalf, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, sqrtHalf, sqrtHalf}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, sqrtHalf, -sqrtHalf}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, -sqrtHalf, sqrtHalf}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.0f, -sqrtHalf, -sqrtHalf}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtHalf, 0.0f, sqrtHalf}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtHalf, 0.0f, sqrtHalf}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtHalf, 0.0f, -sqrtHalf}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtHalf, 0.0f, -sqrtHalf}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtThird, sqrtThird, sqrtThird}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtThird, sqrtThird, -sqrtThird}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtThird, -sqrtThird, sqrtThird}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{sqrtThird, -sqrtThird, -sqrtThird}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtThird, sqrtThird, sqrtThird}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtThird, sqrtThird, -sqrtThird}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtThird, -sqrtThird, sqrtThird}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-sqrtThird, -sqrtThird, -sqrtThird}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
};