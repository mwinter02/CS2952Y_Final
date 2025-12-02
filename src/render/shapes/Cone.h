#pragma once

#include <vector>
#include <glm/glm.hpp>

class Cone {

public:
    std::vector<float> updateParams(int param1, int param2);
    std::vector<glm::vec3> getVertexData();

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void insertVec2(std::vector<float> &data, glm::vec2 v);
    void appendVertexData(glm::vec3 x, glm::vec3 y, glm::vec3 z);
    void makeWedge(float curT, float nexT);
    void setVertexData();
    void makeCone();

    std::vector<float> m_vertexData;
    std::vector<glm::vec3> vertcies;
    int m_param1;
    int m_param2;
    float m_r = 0.5f;
    float m_h = 0.5f;
};
