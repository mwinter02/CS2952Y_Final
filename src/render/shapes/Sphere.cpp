#include "Sphere.h"
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <algorithm>

std::vector<float> Sphere::updateParams(int param1, int param2) {
    m_vertexData.clear();
    vertcies.clear();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
    return m_vertexData;
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight)
{
    glm::vec3 vertices[6] = {
            topLeft,    bottomLeft,  bottomRight,
            bottomRight, topRight,   topLeft
    };

    for (auto p : vertices) {
        glm::vec3 n  = glm::normalize(p);
        float u = 0.5f + (atan2(n.z, n.x) / (2.f * glm::pi<float>()));
        float v = 0.5f + (asinf(n.y)       /  glm::pi<float>());

        insertVec3(m_vertexData, p);
        insertVec3(m_vertexData, n);
        insertVec2(m_vertexData, glm::vec2(u, v));
    }

    appendVertexData(vertices[0], vertices[1], vertices[2]);
    appendVertexData(vertices[3], vertices[4], vertices[5]);
}

void Sphere::makeWedge(float curT, float nexT)
{
    float r = 0.5f;
    int numDivisions = std::max(2, m_param1);

    float phiStep = glm::radians(180.f / float(numDivisions));

    for (int i = 0; i < numDivisions; i++) {
        float phi1 =  i      * phiStep;
        float phi2 = (i + 1) * phiStep;

        glm::vec3 topLeft = r * glm::vec3(glm::sin(phi1) * glm::sin(curT),
                                          glm::cos(phi1),
                                          glm::sin(phi1) * glm::cos(curT));

        glm::vec3 topRight = r * glm::vec3(glm::sin(phi1) * glm::sin(nexT),
                                           glm::cos(phi1),
                                           glm::sin(phi1) * glm::cos(nexT));

        glm::vec3 bottomLeft = r * glm::vec3(glm::sin(phi2) * glm::sin(curT),
                                             glm::cos(phi2),
                                             glm::sin(phi2) * glm::cos(curT));

        glm::vec3 bottomRight = r * glm::vec3(glm::sin(phi2) * glm::sin(nexT),
                                              glm::cos(phi2),
                                              glm::sin(phi2) * glm::cos(nexT));

        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Sphere::makeSphere() {
    m_vertexData.clear();
    int numDivisions = std::max(3, m_param2);

    float thetaStep = glm::radians(360.f / float(numDivisions));
    for (int i = 0; i < numDivisions; i++) {
        float currentTheta =  i      * thetaStep;
        float nextTheta    = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Sphere::setVertexData() {
    makeSphere();
}

std::vector<glm::vec3> Sphere::getVertexData() {
    return vertcies;
}

void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Sphere::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}

void Sphere::appendVertexData(glm::vec3 x, glm::vec3 y, glm::vec3 z) {
    vertcies.push_back(x);
    vertcies.push_back(y);
    vertcies.push_back(z);
}
