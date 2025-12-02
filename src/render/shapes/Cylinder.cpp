#include "Cylinder.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>

static glm::vec2 uvForCap(const glm::vec3 &p)
{
    float u = p.x + 0.5f;
    float v = p.z + 0.5f;
    return glm::vec2(u, v);
}

static glm::vec2 uvForSide(float theta, float y)
{
    float twoPi = 2.f * glm::pi<float>();
    float u = theta / twoPi;
    float v = (y + 0.5f) / 1.f;
    return glm::vec2(u, v);
}

std::vector<float> Cylinder::updateParams(int param1, int param2) {
    m_vertexData.clear();
    vertcies.clear();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
    return m_vertexData;
}

void Cylinder::makeWedge(float curT, float nexT) {
    m_r = 0.5f;
    m_h = 0.5f;

    int numDivisions = std::max(1, m_param1);

    for (int i = 0; i < numDivisions; i++)
    {
        float y0 = -m_h + (i     / float(numDivisions)) * (2.f * m_h);
        float y1 = -m_h + ((i+1) / float(numDivisions)) * (2.f * m_h);

        float r0 = i     / float(numDivisions);
        float r1 = (i+1) / float(numDivisions);

        glm::vec3 t0(m_r * r0 * sin(curT),  m_h, m_r * r0 * cos(curT));
        glm::vec3 t1(m_r * r0 * sin(nexT),  m_h, m_r * r0 * cos(nexT));
        glm::vec3 t2(m_r * r1 * sin(curT),  m_h, m_r * r1 * cos(curT));
        glm::vec3 t3(m_r * r1 * sin(nexT),  m_h, m_r * r1 * cos(nexT));

        glm::vec3 nTop(0.f, 1.f, 0.f); // Normal pointing upwards

        glm::vec2 uvT0 = uvForCap(t0);
        glm::vec2 uvT1 = uvForCap(t1);
        glm::vec2 uvT2 = uvForCap(t2);
        glm::vec2 uvT3 = uvForCap(t3);

        // Ensure counter-clockwise order for top cap
        insertVec3(m_vertexData, t0); insertVec3(m_vertexData, nTop); insertVec2(m_vertexData, uvT0);
        insertVec3(m_vertexData, t2); insertVec3(m_vertexData, nTop); insertVec2(m_vertexData, uvT2);
        insertVec3(m_vertexData, t1); insertVec3(m_vertexData, nTop); insertVec2(m_vertexData, uvT1);
        appendVertexData(t0, t2, t1);

        insertVec3(m_vertexData, t1); insertVec3(m_vertexData, nTop); insertVec2(m_vertexData, uvT1);
        insertVec3(m_vertexData, t2); insertVec3(m_vertexData, nTop); insertVec2(m_vertexData, uvT2);
        insertVec3(m_vertexData, t3); insertVec3(m_vertexData, nTop); insertVec2(m_vertexData, uvT3);
        appendVertexData(t1, t2, t3);

        glm::vec3 b0(m_r * r0 * sin(curT), -m_h, m_r * r0 * cos(curT));
        glm::vec3 b1(m_r * r0 * sin(nexT), -m_h, m_r * r0 * cos(nexT));
        glm::vec3 b2(m_r * r1 * sin(curT), -m_h, m_r * r1 * cos(curT));
        glm::vec3 b3(m_r * r1 * sin(nexT), -m_h, m_r * r1 * cos(nexT));

        glm::vec3 nBot(0.f, -1.f, 0.f); // Normal pointing downwards

        glm::vec2 uvB0 = uvForCap(b2);
        glm::vec2 uvB1 = uvForCap(b0);
        glm::vec2 uvB2 = uvForCap(b3);
        glm::vec2 uvB3 = uvForCap(b1);

        insertVec3(m_vertexData, b2); insertVec3(m_vertexData, nBot); insertVec2(m_vertexData, uvB0);
        insertVec3(m_vertexData, b0); insertVec3(m_vertexData, nBot); insertVec2(m_vertexData, uvB1);
        insertVec3(m_vertexData, b3); insertVec3(m_vertexData, nBot); insertVec2(m_vertexData, uvB2);
        appendVertexData(b2, b0, b3);

        // Triangle #2: (b3, b0, b1)
        insertVec3(m_vertexData, b3); insertVec3(m_vertexData, nBot); insertVec2(m_vertexData, uvB2);
        insertVec3(m_vertexData, b0); insertVec3(m_vertexData, nBot); insertVec2(m_vertexData, uvB1);
        insertVec3(m_vertexData, b1); insertVec3(m_vertexData, nBot); insertVec2(m_vertexData, uvB3);
        appendVertexData(b3, b0, b1);

        glm::vec3 s0(m_r * sin(curT),  y0, m_r * cos(curT));
        glm::vec3 s1(m_r * sin(nexT),  y0, m_r * cos(nexT));
        glm::vec3 s2(m_r * sin(nexT),  y1, m_r * cos(nexT));
        glm::vec3 s3(m_r * sin(curT),  y1, m_r * cos(curT));

        glm::vec3 n0 = glm::normalize(glm::vec3(sin(curT),  0.f, cos(curT)));
        glm::vec3 n1 = glm::normalize(glm::vec3(sin(nexT), 0.f, cos(nexT)));
        glm::vec3 n2 = n1;
        glm::vec3 n3 = n0;

        glm::vec2 uvS0 = uvForSide(curT,  y0);
        glm::vec2 uvS1 = uvForSide(nexT,  y0);
        glm::vec2 uvS2 = uvForSide(nexT,  y1);
        glm::vec2 uvS3 = uvForSide(curT,  y1);

        insertVec3(m_vertexData, s0); insertVec3(m_vertexData, n0); insertVec2(m_vertexData, uvS0);
        insertVec3(m_vertexData, s1); insertVec3(m_vertexData, n1); insertVec2(m_vertexData, uvS1);
        insertVec3(m_vertexData, s3); insertVec3(m_vertexData, n3); insertVec2(m_vertexData, uvS3);
        appendVertexData(s0, s1, s3);

        insertVec3(m_vertexData, s1); insertVec3(m_vertexData, n1); insertVec2(m_vertexData, uvS1);
        insertVec3(m_vertexData, s2); insertVec3(m_vertexData, n2); insertVec2(m_vertexData, uvS2);
        insertVec3(m_vertexData, s3); insertVec3(m_vertexData, n3); insertVec2(m_vertexData, uvS3);
        appendVertexData(s1, s2, s3);
    }
}

void Cylinder::makeCylinder()
{
    int numDivisions = std::max(3, m_param2);
    float thetaStep = glm::radians(360.f / numDivisions);

    for (int i = 0; i < numDivisions; i++) {
        float currentTheta = i       * thetaStep;
        float nextTheta    = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Cylinder::setVertexData() {
    m_vertexData.clear();
    vertcies.clear();
    makeCylinder();
}

std::vector<glm::vec3> Cylinder::getVertexData() {
    return vertcies;
}

void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cylinder::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}

void Cylinder::appendVertexData(glm::vec3 x, glm::vec3 y, glm::vec3 z) {
    vertcies.push_back(x);
    vertcies.push_back(y);
    vertcies.push_back(z);
}
