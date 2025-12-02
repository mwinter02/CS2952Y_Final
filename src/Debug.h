#pragma once
#include <iostream>

#ifndef NDEBUG
    #define DEBUG_ENABLED 1
#else
    #define DEBUG_ENABLED 0
#endif

namespace debug {
    /**
     * Prints a debug message to the console if cmake's debug profile is enabled
     * @param message to print
     */
    static void print(const std::string& message) {
        if constexpr (DEBUG_ENABLED) {
            std::cout << message << std::endl;
        }
    }


    /**
     * Prints an error message to the console
     * @param message to be displayed after "Error: "
     */
    static void error(const std::string& message) {
        std::cerr << "Error: " << message << std::endl;
    }

    inline GLenum checkGLError(const char *file, int line)
    {
        GLenum errorCode;
        while ((errorCode = glGetError()) != GL_NO_ERROR)
        {
            std::string error;
            switch (errorCode)
            {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            }
            std::cout << error << " | " << file << " (" << line << ")" << std::endl;
        }
        return errorCode;
    }

#define checkGLError() checkGLError(__FILE__, __LINE__)


}
