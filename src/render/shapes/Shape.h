#pragma once

namespace gl {
    class Shape {
    public:
        // virtual

    protected:

        void insertData(glm::vec3 position, glm::vec3 normal, glm::vec2 tex_coord);
        std::vector<float> vertex_buffer_data;
        std::vector<glm::vec3> vertices;

    private:

    };
}