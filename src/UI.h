#pragma once
#include <string>

#include "GLFW/glfw3.h"


class UI {

public:

    struct FileFilters {

        FileFilters(const std::string& filter_name, const std::string& filter_spec) : name(filter_name), spec(filter_spec) {}
        std::string name;
        std::string spec;
    };
    static void initialize(GLFWwindow* window);
    static void beginDraw(float x_pos, float y_pos, float width, float height);
    static void endDraw();
    static std::string openFileExplorer(std::vector<FileFilters>);
};
