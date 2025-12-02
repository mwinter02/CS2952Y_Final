cmake_minimum_required(VERSION 4.0)
project(Test_project) # Enter your project name here

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
#set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIR})

# Find dependencies
find_package(OpenGL REQUIRED)

# External libraries
add_subdirectory(External/glfw)
add_subdirectory(External/glm)
add_subdirectory(External/freetype)
add_subdirectory(External/assimp)
add_subdirectory(External/imgui)

add_library(StaticGLEW STATIC External/glew/src/glew.c)
target_include_directories(StaticGLEW PUBLIC External/glew/include)
target_compile_definitions(StaticGLEW PUBLIC GLEW_STATIC)

add_library(stb_image STATIC External/stb/stb_image_impl.cpp)
target_include_directories(stb_image PUBLIC External/stb)

target_include_directories(imgui PUBLIC External/imgui)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)
add_executable(${PROJECT_NAME} main.cpp ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        StaticGLEW
        OpenGL::GL
        glm::glm
        freetype
        assimp::assimp
        stb_image
        imgui
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
        <glm/glm.hpp>
        <glm/ext.hpp>
)

# Defines PROJECT_ROOT macro for use in source files
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_ROOT="${PROJECT_SOURCE_DIR}")
