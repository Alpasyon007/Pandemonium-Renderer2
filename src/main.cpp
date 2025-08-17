#include "Renderer.hpp"
#include <iostream>

int main() {
    try {
        rtvk::Renderer app(800, 600, "Vulkan Triangle");
        app.mainLoop();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
