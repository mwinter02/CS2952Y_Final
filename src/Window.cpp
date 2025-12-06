#include <GL/glew.h>
#include "Window.h"

#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

#include <sstream>

#include "render/Graphics.h"
#include "Core.h"
#include "imgui_internal.h"
#include "UI.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace gl {}
    GLFWwindow* Window::window_ = nullptr;
    Core* Window::core_ = nullptr;

    int Window::width_, Window::height_;
    float Window::aspect_ratio_;
    bool Window::keys_[1024] = { false };
    bool Window::cursor_visible_ = true;
    double Window::mouse_x_, Window::mouse_y_ = 0;
    glm::vec3 Window::clear_color_ = glm::vec3(0.0f);
    Window::~Window() {
        shutDown();
    }

    static const char* s_title = "";
    int Window::initialize(const int width, const int height, const char* title) {
        s_title = title;
        initializeGLFW(width, height);
        initializeGLEW();

        UI::initialize(window_);

        gl::Graphics::initialize();

        core_ = new Core();


        return 0;
    }

    bool Window::isActive() {
        return !glfwWindowShouldClose(window_);
    }

    static double s_currentTime = glfwGetTime();
    static double s_lastTime = s_currentTime;
    void Window::update() {
        s_currentTime = glfwGetTime();
        displayFrameRate();


        const auto delta_time = static_cast<float>(s_currentTime - s_lastTime);
        core_->update(delta_time);
        display();
        // ui_->update();

        glfwSwapBuffers(window_);

        glfwPollEvents();
        s_lastTime = s_currentTime;
    }

    void Window::display() {
        // Ensure proper viewport and disable scissor test for 3D rendering
        glViewport(0, 0, width_, height_);
        glDisable(GL_SCISSOR_TEST);

        glClearColor(clear_color_.x,clear_color_.y,clear_color_.z, 1.0f);  // Alpha = 1.0 for opaque background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Reset blend state for 3D rendering
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        core_->draw();
    }

    void Window::shutDown() {
        if (window_) {
            glfwDestroyWindow(window_);
        }
        delete core_;
        gl::Graphics::tearDown();
        glfwTerminate();
    }

    bool Window::key(const GLint key) {
        return keys_[key];
    }

    void Window::hideMouse() {
        cursor_visible_ = false;
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void Window::showMouse() {
        cursor_visible_ = true;
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    glm::vec2 Window::getMousePosition() {
        return {static_cast<float>(mouse_x_), static_cast<float>(mouse_y_)};
    }


float Window::getAspectRatio() {
        return aspect_ratio_;
    }

bool Window::isCursorVisible() {
        return cursor_visible_;
}

double Window::getCurrentTime() {
        return s_currentTime;
}

void Window::setClearColor(glm::vec3 color) {
    clear_color_ = color;
}

int Window::initializeGLFW(int width, int height) {
        if (!glfwInit()) return -1;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); // Disable Retina scaling
#endif

        window_ = glfwCreateWindow(width, height, s_title, nullptr, nullptr);

        if (!window_) {
            const char* errorMsg;
            int err = glfwGetError(&errorMsg);
            std::cerr << "Failed to create GLFW window: " << errorMsg << std::endl;
            glfwTerminate();
            return err;
        }
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1);


        glfwSetDropCallback(window_, dragDropCallback);
        glfwSetCursorPosCallback(window_, mousePositionCallback);
        glfwSetMouseButtonCallback(window_, mouseButtonCallback);
        glfwSetScrollCallback(window_, scrollCallback);
        glfwSetKeyCallback(window_, keyboardCallback);
        glfwSetInputMode(window_, GLFW_STICKY_KEYS, GLFW_TRUE);
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorEnterCallback(window_, cursorEnterCallback);
        glfwSetWindowSizeCallback(window_, resizeWindowCallback);


        // Set initial variables
        glfwGetCursorPos(window_, &mouse_x_, &mouse_y_);
        glfwGetWindowSize(window_, &width_, &height_);
        aspect_ratio_ = static_cast<float>(width_) / static_cast<float>(height_);

        return 1;
    }

    int Window::initializeGLEW() {
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "GLEW Initialization Failed\n";
            return -1;
        }
        return 0;
    }

    void Window::resizeWindowCallback(GLFWwindow *window, int width, int height) {
        width_ = width;
        height_ = height;
        aspect_ratio_ = static_cast<float>(width_) / static_cast<float>(height_);
        glViewport(0, 0, width_, height_);
    }

    void Window::keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            keys_[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            keys_[key] = false;
        }
    }

    void Window::scrollCallback(GLFWwindow *window, double x_offset, double y_offset) {
        if (core_) {
            core_->onScroll(x_offset, y_offset);
        }
    }

    void Window::cursorEnterCallback(GLFWwindow *window, int entered) {

    }

    void Window::mousePositionCallback(GLFWwindow *window, const double x_pos, const double y_pos) {
        mouse_x_ = x_pos;
        mouse_y_ = y_pos;
    }

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (core_) {
            core_->onMouseButton(button, action, mods);
        }
}

void Window::dragDropCallback(GLFWwindow *window, int count, const char **paths) {

    }

    static int s_nbFrames = 0;
    static double s_previousFrameTime = s_lastTime;
    void Window::displayFrameRate() {
        s_nbFrames++;
        // Calculate FPS
        if (s_currentTime - s_previousFrameTime >= 1.0) { // If last update was more than 1 second ago
            const int fps = static_cast<int>(static_cast<double>(s_nbFrames) / (s_currentTime - s_previousFrameTime));
            std::stringstream ss;
            ss << s_title << " | FPS: " << fps;
            glfwSetWindowTitle(window_, ss.str().c_str());

            s_nbFrames = 0;
            s_previousFrameTime = s_currentTime;
        }
    }

