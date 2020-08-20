#include "particle.h"







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