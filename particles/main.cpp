/**/



#define GLM_FORCE_RADIANS



#include <iostream>
#include <string>

#include "vulkan.h"
#include "glm/glm.hpp"







int main() {

    std::cout << "Hello! This project will totally not fail (:\n";

    try {
        Vulkan::init();
        Vulkan::cleanup();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}