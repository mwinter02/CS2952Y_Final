#pragma once

#include <vector>
#include <glm/glm.hpp>
namespace gl {
    class Cube {
    public:
        std::vector<float> updateParams(int param1, int param2);
        std::vector<glm::vec3> getVertexData();

    private:
        void insertVec3(std::vector<float> &data, glm::vec3 v);
        void insertVec2(std::vector<float> &data, glm::vec2 v);
        void appendVertexData(glm::vec3 x, glm::vec3 y, glm::vec3 z);
        void setVertexData();
        void makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight,
                      glm::vec2 uvTopLeft,
                      glm::vec2 uvTopRight,
                      glm::vec2 uvBottomLeft,
                      glm::vec2 uvBottomRight);

        void makeFace(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight);

        std::vector<float> m_vertexData;
        std::vector<glm::vec3> vertcies;
        int m_param1;
    };
}