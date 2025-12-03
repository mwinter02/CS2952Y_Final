#pragma once
#include "UI.h"
#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

class Core;

namespace gl {}
    class Window {
    public:
        ~Window();

        static int initialize(int width, int height, const char* title);
        static bool isActive();

        static void update();
        static void shutDown();

        static bool key(GLint key);

        static void hideMouse();
        static void showMouse();

        static glm::vec2 getMousePosition();

        static float getAspectRatio();
        static bool isCursorVisible();
        static double getCurrentTime();

    private:
        static void display();
        static int initializeGLFW(int width, int height);
        static int initializeGLEW();

        // GLFW callback functions
        static void resizeWindowCallback(GLFWwindow* window, int width, int height);
        static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void scrollCallback(GLFWwindow * window, double x_offset, double y_offset);
        static void cursorEnterCallback(GLFWwindow* window, int entered);
        static void mousePositionCallback(GLFWwindow * window, double x_pos, double y_pos);
        static void dragDropCallback(GLFWwindow * window, int count, const char** paths);
        static void displayFrameRate();


        static Core* core_;
        static GLFWwindow *window_;
        static int width_, height_;
        static float aspect_ratio_;

        static bool keys_[1024];
        static bool cursor_visible_;

        static double mouse_x_, mouse_y_;
    };


