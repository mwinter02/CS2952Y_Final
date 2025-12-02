# CMakeLists.txt for ImGui
cmake_minimum_required(VERSION 4.0)
project(imgui)

add_library(imgui)
target_sources(imgui PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(imgui PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui"
        "${GLEW_SOURCE_DIR}/include"
        "${GLFW_SOURCE_DIR}/include"
)

# Tell ImGui to use GLEW instead of GLAD
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)

# Link with glfw and StaticGLEW (instead of glad)
target_link_libraries(imgui PUBLIC glfw StaticGLEW)
