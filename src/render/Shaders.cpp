#include "Shaders.h"

#include <fstream>

#include "../Debug.h"
#include "../Util.h"

namespace gl {

    void ShaderProgram::use() const {
        glUseProgram(program_id);
    }

    void ShaderProgram::deleteProgram() const {
        glDeleteProgram(program_id);
    }

    void ShaderProgram::setMat4Vec(const char* name, const size_t size, const std::vector<glm::mat4>& matrices) {
        glUniformMatrix4fv(getLocation(name), (GLsizei) size, GL_FALSE, glm::value_ptr(matrices[0]));
    }

    void ShaderProgram::setMat4(const char* name, const glm::mat4& matrix) {
        glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void ShaderProgram::setMat3(const char* name, const glm::mat3& matrix) {
        glUniformMatrix3fv(getLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void ShaderProgram::setVec4(const char* name, const glm::vec4& vector) {
        glUniform3fv(getLocation(name), 1, glm::value_ptr(vector));
    }

    void ShaderProgram::setVec3(const char* name, const glm::vec3& vector) {
        glUniform3fv(getLocation(name), 1, glm::value_ptr(vector));
    }

    void ShaderProgram::setVec2(const char* name, const glm::vec2& vector) {
        glUniform2fv(getLocation(name), 1, glm::value_ptr(vector));
    }

    void ShaderProgram::setFloat(const char* name, const float& value) {
        glUniform1f(getLocation(name), value);
    }

    void ShaderProgram::setInt(const char* name, int value) {
        glUniform1i(getLocation(name), value);
    }

    GLuint ShaderProgram::getVertexID() const {
        return vertex_id;
    }


    GLuint ShaderProgram::getFragmentID() const {
        return fragment_id;
    }

    GLint ShaderProgram::getLocation(const char* name) const {
        return glGetUniformLocation(program_id, name);
    }

    /**
     *
     * @param vertex_path The file path to the vertex shader from project root, e.g "Resources/Shaders/phong_vert.glsl"
     * @param fragment_path The file path to the vertex shader from project root
     * @return ShaderProgram struct containing the program ID and an empty map of uniform locations
     */
    ShaderProgram Shaders::createShaderProgram(const char* vertex_path, const char* fragment_path) {
        const auto full_vert_path = util::getPath(vertex_path); // Get full path of the file
        const auto full_frag_path = util::getPath(fragment_path);

        const auto vertex_shader = initializeShader(GL_VERTEX_SHADER, full_vert_path.c_str());
        const auto fragment_shader = initializeShader(GL_FRAGMENT_SHADER, full_frag_path.c_str());

        const auto program_id = initializeProgram(vertex_shader, fragment_shader);

        return {program_id, vertex_shader, fragment_shader};
    }

    ShaderProgram Shaders::createShaderProgram(const char* vertex_path, const GLuint fragment_program_id) {
        const auto full_vert_path = util::getPath(vertex_path); // Get full path of the file

        const auto vertex_shader = initializeShader(GL_VERTEX_SHADER, full_vert_path.c_str());

        const auto program_id = initializeProgram(vertex_shader, fragment_program_id);

        return {program_id, vertex_shader, fragment_program_id};
    }


    GLuint Shaders::initializeProgram(GLuint vertex_shader, GLuint fragment_shader){
        GLint linked;
        GLuint program = glCreateProgram();

        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &linked);

        if (linked) glUseProgram(program);
        else {
            getProgramErrors(program);
            throw std::runtime_error("Shader program did not link correctly!");
        }
        return program;
    }

    GLuint Shaders::initializeShader(GLenum type, const char* filename) {
        GLuint shader = glCreateShader(type);
        GLint compiled;

        std::string str = readTextFile(filename);
        const char* cstr = str.c_str();

        glShaderSource(shader, 1, &cstr, nullptr);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (!compiled) {
            getShaderErrors(shader);
            throw std::runtime_error("Shader did not compile!");
        }
        return shader;
    }

    void Shaders::getShaderErrors(const GLuint shader) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        std::string log(length, '\0');
        glGetShaderInfoLog(shader, length, nullptr, &log[0]);
        glDeleteShader(shader);
        throw std::runtime_error("Compile Error, Log Below: \n" + log + "\n");
    }

    void Shaders::getProgramErrors(const GLuint program) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::string log(length, '\0');
        glGetProgramInfoLog(program, length, &length, &log[0]);
        glDeleteProgram(program);
        throw std::runtime_error("Compile Error, Log Below: \n" + log + "\n");
    }

    GLuint Shaders::getUniformLocation(const GLuint shader_program, const char* uniform_name) {
        const GLuint location = glGetUniformLocation(shader_program, uniform_name);
        if (location == INVALID_UNIFORM_LOCATION) {
            fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", uniform_name);
        }
        return location;
    }

    std::string Shaders::readTextFile(const char* filename) {
        std::string line;
        std::string content;


        std::string path = filename;
        std::ifstream fileStream(path, std::ios::in);

        if (!fileStream.is_open()) {
            throw std::runtime_error("Could not read file " + std::string(filename) + ".");
        }

        while (std::getline(fileStream, line)) {
            content.append(line + "\n");
        }

        fileStream.close();
        return content;
    }
}
