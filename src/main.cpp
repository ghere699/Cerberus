#include "core/Application.h"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        Cerberus::Application app;
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}