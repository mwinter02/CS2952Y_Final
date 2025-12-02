//
// Created by Marcus Winter on 11/17/25.
//

#include "Shape.h"
namespace gl {


    void Shape::insertData(glm::vec3 position, glm::vec3 normal, glm::vec2 tex_coord) {
        vertex_buffer_data.push_back(position.x);
        vertex_buffer_data.push_back(position.y);
        vertex_buffer_data.push_back(position.z);

        vertex_buffer_data.push_back(normal.x);
        vertex_buffer_data.push_back(normal.y);
        vertex_buffer_data.push_back(normal.z);

        vertex_buffer_data.push_back(tex_coord.x);
        vertex_buffer_data.push_back(tex_coord.y);
    }
}