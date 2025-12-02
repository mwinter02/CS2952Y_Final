#pragma once
#include <string>
#include <unordered_map>

#include "GL/glew.h"

#define INVALID_UNIFORM_LOCATION 0xffffffff

namespace gl {

    class ShaderProgram {
    public:
        ShaderProgram() : program_id(0), vertex_id(0), fragment_id(0) {}
        ShaderProgram(const GLuint program_id, const GLuint vertex_id, const GLuint fragment_id) :
        program_id(program_id), vertex_id(vertex_id), fragment_id(fragment_id) {}

        void use() const;
        void deleteProgram() const;

        void setMat4Vec(const char* name, size_t size, const std::vector<glm::mat4>& matrices);
        void setMat4(const char* name, const glm::mat4& matrix);
        void setMat3(const char* name, const glm::mat3& matrix);
        void setVec4(const char* name, const glm::vec4& vector);
        void setVec3(const char* name, const glm::vec3& vector);
        void setVec2(const char* name, const glm::vec2& vector);
        void setFloat(const char* name, const float& value);
        void setInt(const char* name, int value);

        GLuint getVertexID() const;
        GLuint getFragmentID() const;


    private:
        GLint getLocation(const char* name) const;
        GLuint program_id;
        GLuint vertex_id;
        GLuint fragment_id;
        std::unordered_map<std::string, GLuint> uniforms;
    };


    class Shaders {
    public:
        static ShaderProgram createShaderProgram(const char* vertex_path, const char* fragment_path);
        static ShaderProgram createShaderProgram(const char* vertex_path, GLuint fragment_program_id);

    private:
        static GLuint initializeProgram(GLuint vertex_shader, GLuint fragment_shader);
        static GLuint initializeShader(GLenum type, const char* source);
        static void getShaderErrors(GLuint shader);
        static void getProgramErrors(GLuint program);
        static GLuint getUniformLocation(GLuint shader_program, const char* uniformName);
        static std::string readTextFile(const char* filename);
    };
}
