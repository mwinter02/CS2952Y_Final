#include "Cube.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
namespace gl {
    std::vector<float> Cube::updateParams(int param1, int param2) {

        m_vertexData.clear();
        vertcies.clear();
        m_param1 = param1;

        setVertexData();
        return m_vertexData;
    }

    void Cube::setVertexData() {

        // +Z face (looking at the cube "front")
        makeFace(glm::vec3(-0.5f,  0.5f,  0.5f),
                 glm::vec3( 0.5f,  0.5f,  0.5f),
                 glm::vec3(-0.5f, -0.5f,  0.5f),
                 glm::vec3( 0.5f, -0.5f,  0.5f));

        // -Z face (the back of the cube)
        makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
                 glm::vec3(-0.5f, -0.5f, -0.5f),
                 glm::vec3( 0.5f,  0.5f, -0.5f),
                 glm::vec3( 0.5f, -0.5f, -0.5f));

        // -X face (the left side)
        makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
                 glm::vec3(-0.5f,  0.5f,  0.5f),
                 glm::vec3(-0.5f, -0.5f, -0.5f),
                 glm::vec3(-0.5f, -0.5f,  0.5f));

        // +X face (the right side)
        makeFace(glm::vec3( 0.5f, -0.5f,  0.5f),
                 glm::vec3( 0.5f,  0.5f,  0.5f),
                 glm::vec3( 0.5f, -0.5f, -0.5f),
                 glm::vec3( 0.5f,  0.5f, -0.5f));

        // +Y face (top)
        makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
                 glm::vec3( 0.5f,  0.5f, -0.5f),
                 glm::vec3(-0.5f,  0.5f,  0.5f),
                 glm::vec3( 0.5f,  0.5f,  0.5f));

        // -Y face (bottom)
        makeFace(glm::vec3(-0.5f, -0.5f, -0.5f),
                 glm::vec3(-0.5f, -0.5f,  0.5f),
                 glm::vec3( 0.5f, -0.5f, -0.5f),
                 glm::vec3( 0.5f, -0.5f,  0.5f));
    }

    void Cube::makeFace(glm::vec3 topLeftPos,
                        glm::vec3 topRightPos,
                        glm::vec3 bottomLeftPos,
                        glm::vec3 bottomRightPos)
    {
        int rows = m_param1;
        int cols = m_param1;

        glm::vec3 rowStep = (bottomLeftPos - topLeftPos) / float(rows);
        glm::vec3 colStep = (topRightPos   - topLeftPos) / float(cols);

        for(int r = 0; r < rows; r++) {
            for(int c = 0; c < cols; c++) {

                glm::vec3 tileTopLeft     = topLeftPos + float(r) * rowStep + float(c) * colStep;
                glm::vec3 tileTopRight    = tileTopLeft + colStep;
                glm::vec3 tileBottomLeft  = tileTopLeft + rowStep;
                glm::vec3 tileBottomRight = tileTopRight + rowStep;
                float u0 = (float(c)) / float(cols);
                float u1 = (float(c) + 1.f) / float(cols);
                float v0 = (float(r)) / float(rows);
                float v1 = (float(r) + 1.f) / float(rows);

                glm::vec2 uvTopLeft(u0, 1.f - v0);
                glm::vec2 uvTopRight(u1, 1.f - v0);
                glm::vec2 uvBottomLeft(u0, 1.f - v1);
                glm::vec2 uvBottomRight(u1, 1.f - v1);

                makeTile(tileTopLeft, tileTopRight,
                         tileBottomLeft, tileBottomRight,
                         uvTopLeft, uvTopRight,
                         uvBottomLeft, uvBottomRight);
            }
        }
    }

    void Cube::makeTile(glm::vec3 topLeftPos,
                        glm::vec3 topRightPos,
                        glm::vec3 bottomLeftPos,
                        glm::vec3 bottomRightPos,
                        glm::vec2 uvTopLeft,
                        glm::vec2 uvTopRight,
                        glm::vec2 uvBottomLeft,
                        glm::vec2 uvBottomRight)
    {
        glm::vec3 normal1 = glm::normalize(glm::cross(bottomLeftPos - topLeftPos,
                                                      topRightPos   - topLeftPos));
        glm::vec3 normal2 = glm::normalize(glm::cross(topLeftPos - topRightPos,
                                                      bottomRightPos - topRightPos));

        glm::vec3 positions[6] = {
            topLeftPos,    bottomLeftPos,  bottomRightPos,
            topRightPos,   topLeftPos,     bottomRightPos
    };

        glm::vec3 normals[6] = {
            normal1, normal1, normal1,
            normal2, normal2, normal2
    };

        glm::vec2 uvs[6] = {
            uvTopLeft, uvBottomLeft, uvBottomRight,
            uvTopRight, uvTopLeft,   uvBottomRight
    };

        for (int i = 0; i < 6; i++) {
            insertVec3(m_vertexData, positions[i]);
            insertVec3(m_vertexData, normals[i]);
            insertVec2(m_vertexData, uvs[i]);
        }

        appendVertexData(positions[0], positions[1], positions[2]);
        appendVertexData(positions[3], positions[4], positions[5]);
    }

    void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
        data.push_back(v.x);
        data.push_back(v.y);
        data.push_back(v.z);
    }

    void Cube::insertVec2(std::vector<float> &data, glm::vec2 v) {
        data.push_back(v.x);
        data.push_back(v.y);
    }

    void Cube::appendVertexData(glm::vec3 x, glm::vec3 y, glm::vec3 z) {
        vertcies.push_back(x);
        vertcies.push_back(y);
        vertcies.push_back(z);
    }

    std::vector<glm::vec3> Cube::getVertexData() {
        return vertcies;
    }
}