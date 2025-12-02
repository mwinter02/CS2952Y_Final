#include "Cone.h"
#include <glm/gtc/constants.hpp>

std::vector<float> Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
    return m_vertexData;
}

glm::vec3 findNormal(glm::vec3 point) {
    glm::vec3 n0(2.0f * point.x, 0.25f - 0.5f * point.y, 2.0f * point.z);
    return n0;
}

static glm::vec2 uvForCap(const glm::vec3 &p) {

    float u = p.x + 0.5f;
    float v = p.z + 0.5f;
    return glm::vec2(u, v);
}

static glm::vec2 uvForSide(float theta, float y) {

    float twoPi = 2.f * glm::pi<float>();
    float u = theta / twoPi;
    float v = (y + 0.5f) / 1.0f;
    return glm::vec2(u, v);
}

void Cone::makeWedge(float curT, float nexT) {

    m_r = 0.5f;
    m_h = 0.5f;


    int numDivisions = std::max(1, m_param1);

    for (int i = 0; i < numDivisions; i++) {

        float r0 = float(i)     / float(numDivisions);
        float r1 = float(i + 1) / float(numDivisions);

        glm::vec3 t0 = glm::vec3(m_r * r0 * sin(curT), -m_h, m_r * r0 * cos(curT));
        glm::vec3 t1 = glm::vec3(m_r * r0 * sin(nexT), -m_h, m_r * r0 * cos(nexT));
        glm::vec3 t2 = glm::vec3(m_r * r1 * sin(curT), -m_h, m_r * r1 * cos(curT));
        glm::vec3 t3 = glm::vec3(m_r * r1 * sin(nexT), -m_h, m_r * r1 * cos(nexT));

        glm::vec3 nt0 = glm::vec3(0.0f, -1.0f, 0.0f);

        glm::vec2 uv0 = uvForCap(t2);
        glm::vec2 uv1 = uvForCap(t0);
        glm::vec2 uv2 = uvForCap(t3);
        glm::vec2 uv3 = uvForCap(t1);

        insertVec3(m_vertexData, t2); insertVec3(m_vertexData, nt0); insertVec2(m_vertexData, uv0);
        insertVec3(m_vertexData, t0); insertVec3(m_vertexData, nt0); insertVec2(m_vertexData, uv1);
        insertVec3(m_vertexData, t3); insertVec3(m_vertexData, nt0); insertVec2(m_vertexData, uv2);
        appendVertexData(t2, t0, t3);

        insertVec3(m_vertexData, t3); insertVec3(m_vertexData, nt0); insertVec2(m_vertexData, uv2);
        insertVec3(m_vertexData, t0); insertVec3(m_vertexData, nt0); insertVec2(m_vertexData, uv1);
        insertVec3(m_vertexData, t1); insertVec3(m_vertexData, nt0); insertVec2(m_vertexData, uv3);
        appendVertexData(t3, t0, t1);

        float y0 = m_h - float(i)     / float(numDivisions);
        float y1 = m_h - float(i + 1) / float(numDivisions);

        glm::vec3 s0(r0 * m_r * sin(curT),  y0, r0 * m_r * cos(curT));
        glm::vec3 s1(r0 * m_r * sin(nexT),  y0, r0 * m_r * cos(nexT));
        glm::vec3 s2(r1 * m_r * sin(nexT),  y1, r1 * m_r * cos(nexT));
        glm::vec3 s3(r1 * m_r * sin(curT),  y1, r1 * m_r * cos(curT));

        glm::vec3 n0 = glm::normalize(findNormal(s0));
        glm::vec3 n1 = glm::normalize(findNormal(s1));
        glm::vec3 n2 = glm::normalize(findNormal(s2));
        glm::vec3 n3 = glm::normalize(findNormal(s3));

        glm::vec2 uvS0 = uvForSide(curT,  y0);
        glm::vec2 uvS1 = uvForSide(nexT,  y0);
        glm::vec2 uvS2 = uvForSide(nexT,  y1);
        glm::vec2 uvS3 = uvForSide(curT,  y1);

        insertVec3(m_vertexData, s3); insertVec3(m_vertexData, n3); insertVec2(m_vertexData, uvS3);
        insertVec3(m_vertexData, s2); insertVec3(m_vertexData, n2); insertVec2(m_vertexData, uvS2);
        insertVec3(m_vertexData, s1); insertVec3(m_vertexData, n1); insertVec2(m_vertexData, uvS1);
        appendVertexData(s3, s2, s1);

        insertVec3(m_vertexData, s3); insertVec3(m_vertexData, n3); insertVec2(m_vertexData, uvS3);
        insertVec3(m_vertexData, s1); insertVec3(m_vertexData, n1); insertVec2(m_vertexData, uvS1);
        insertVec3(m_vertexData, s0); insertVec3(m_vertexData, n0); insertVec2(m_vertexData, uvS0);
        appendVertexData(s3, s1, s0);
    }
}

void Cone::makeCone() {

    int numDivisions = std::max(3, m_param2);
    float thetaStep = glm::radians(360.f / numDivisions);
    for (int i = 0; i < numDivisions; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Cone::setVertexData(){
    m_vertexData.clear();
    makeCone();
}

std::vector<glm::vec3> Cone::getVertexData() {return vertcies;}


void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cone::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}

void Cone::appendVertexData(glm::vec3 x, glm::vec3 y, glm::vec3 z) {
    vertcies.push_back(x);
    vertcies.push_back(y);
    vertcies.push_back(z);
}

