#pragma once
#include <algorithm>
#include <filesystem>
#include <string>

#include "assimp/matrix4x4.h"

namespace util {
    /**
     * Gets the absolute path to a file given its path from the project root directory
     * Helps avoid issues with relative paths when running from different working directories.
     * @param path_from_root The path from the project root directory, e.g "Resources/Shaders/phong_vert.glsl"
     * @return The absolute path to the file from your computer's root directory
     */
    static std::string getPath(const char* path_from_root) {
        std::string full_path = std::string(PROJECT_ROOT) + "/" + path_from_root;
        return full_path;
    }
    /**
     * Gets the absolute path to a file given its path from the project root directory
     * Helps avoid issues with relative paths when running from different working directories.
     * @param path_from_root The path from the project root directory, e.g "Resources/Shaders/phong_vert.glsl"
     * @return The absolute path to the file from your computer's root directory
     */
    static std::string getPath(const std::string& path_from_root) {
        return getPath(path_from_root.c_str());
    }

    /**
     * Gets the directory of a file given its path from the project root directory
     * @param path_from_root The path from the project root directory, e.g "Resources/Shaders/phong_vert.glsl"
     * @return The directory of the file from your computer's root directory
     */
    static std::string getFullDirectory(const char* path_from_root) {
        const auto full_path = getPath(path_from_root);
        return std::filesystem::path(full_path).parent_path().string();
    }

    /**
     * Gets the directory of a file from the project root
     * @param path_from_root The path from the project root directory, e.g "Resources/Shaders/phong_vert.glsl"
     * @return The directory of a file from the project root directory, e.g "Resources/Shaders"
     */
    static std::string getDirectory(const std::string& path_from_root) {
        return std::filesystem::path(path_from_root).parent_path().string();
    }

    /**
     * Fixes path separators to be correct for the current OS
     * @param path - The path to fix
     */
    static void fixPath(std::string &path) {
        if (path.empty()) return;

        // Remove leading ".\" or "./"
        if (path.size() >= 2 && path[0] == '.' && (path[1] == '\\' || path[1] == '/')) {
            path = path.substr(2);
        }
#ifdef _WIN32
        std::ranges::replace(path, '/', '\\');
#else
        std::ranges::replace(path, '\\', '/');
#endif
    }


    static glm::mat4 aiToGlmMat4(const aiMatrix4x4& mat) {
        return {
            mat.a1, mat.b1, mat.c1, mat.d1,
            mat.a2, mat.b2, mat.c2, mat.d2,
            mat.a3, mat.b3, mat.c3, mat.d3,
            mat.a4, mat.b4, mat.c4, mat.d4
        };
    }
}
