#include "particle.h"







const VkVertexInputBindingDescription Particle::bindingDescription = {
    0, //binding
    sizeof(Particle), //stride
    VK_VERTEX_INPUT_RATE_VERTEX //inputRate
};



const std::vector<VkVertexInputAttributeDescription> Particle::attributeDescription = {
    {
        0, //binding
        0, //location
        VK_FORMAT_R32G32B32_SFLOAT, //format
        offsetof(Particle, r) //offset
    }, {
        0, //binding
        1, //location
        VK_FORMAT_R32G32B32_SFLOAT, //format
        offsetof(Particle, v) //offset
    }, {
        0, //binding
        2, //location
        VK_FORMAT_R32G32B32_SFLOAT, //format
        offsetof(Particle, colour) //offset
    }
};