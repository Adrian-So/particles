#include "particle.h"







const VkVertexInputBindingDescription Particle::bindingDescription = {
    0, //binding
    sizeof(Particle), //stride
    VK_VERTEX_INPUT_RATE_VERTEX //inputRate
};



const std::vector<VkVertexInputAttributeDescription> Particle::attributeDescription = {
    {
        0, //location
        0, //binding
        VK_FORMAT_R32G32B32_SFLOAT, //format
        offsetof(Particle, r) //offset
    }, {
        1, //location
        0, //binding
        VK_FORMAT_R32G32B32_SFLOAT, //format
        offsetof(Particle, v) //offset
    }, {
        2, //location
        0, //binding
        VK_FORMAT_R32G32B32_SFLOAT, //format
        offsetof(Particle, colour) //offset
    }
};