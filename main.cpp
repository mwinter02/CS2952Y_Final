#include <iostream>
#include <GLFW/glfw3.h>

#include "src/Window.h"


int main() {
    Window::initialize(1280, 720, "Project Name");
    while (Window::isActive()) {
        Window::update();
    }
    Window::shutDown();

    return 0;
}
