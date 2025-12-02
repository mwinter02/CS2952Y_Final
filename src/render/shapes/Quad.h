#pragma once

#include <vector>
#include <glm/glm.hpp>

class Quad {
public:
    Quad(float width = 1.0f, float height = 1.0f);

    std::vector<float> updateParams(float width, float height);


    std::vector<glm::vec3> getVertexData();
    std::vector<glm::vec3> getNormals() const;
    std::vector<glm::vec2> getTexCoords() const;

    static std::vector<float> createQuad(glm::vec3 position, float width = 1.f, float height = 1.f);
    static std::vector<float> createUpsideDownQuad(glm::vec3 position, float width = 1.f, float height = 1.f);
private:
    float m_width;
    float m_height;
    std::vector<float> m_vertexData;
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_textureCoords;

    static void insertData(std::vector<float>& data, const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texCoord);
    void setVertexData();
    void appendVertexData(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texCoord);
    static void insertVec3(std::vector<float> &data, const glm::vec3 &v);

    static void insertVec2(std::vector<float> &data, const glm::vec2 &v);
};
