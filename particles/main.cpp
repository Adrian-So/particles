/**/



#include <iostream>

#include "vulkan.h"







int main() {

    std::cout << "Hello! This project will totally not fail (:\n\n";

    try {

        Vulkan::init();
        Vulkan::run();
        Vulkan::cleanup();

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}