#include "Quad.h"

Quad::Quad(float width, float height)
        : m_width(width), m_height(height) {
    setVertexData();
}

std::vector<float> Quad::updateParams(float width, float height) {
    m_vertexData.clear();
    m_positions.clear();
    m_normals.clear();
    m_textureCoords.clear();

    m_width = width;
    m_height = height;

    setVertexData();
    return m_vertexData;
}

std::vector<glm::vec3> Quad::getVertexData() {
    return m_positions;
}

std::vector<glm::vec3> Quad::getNormals() const {
    return m_normals;
}

std::vector<glm::vec2> Quad::getTexCoords() const {
    return m_textureCoords;
}

std::vector<float> Quad::createQuad(glm::vec3 position, float width, float height) {
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    std::vector<float> data;

    glm::vec3 topLeft     = position + glm::vec3(-halfWidth,  0.0f, -halfHeight);
    glm::vec3 bottomLeft  = position + glm::vec3(-halfWidth,  0.0f,  halfHeight);
    glm::vec3 bottomRight = position + glm::vec3( halfWidth,  0.0f,  halfHeight);
    glm::vec3 topRight    = position + glm::vec3( halfWidth,  0.0f, -halfHeight);

    glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec2 texTopLeft     = glm::vec2(0.0f, 1.0f);
    glm::vec2 texBottomLeft  = glm::vec2(0.0f, 0.0f);
    glm::vec2 texBottomRight = glm::vec2(1.0f, 0.0f);
    glm::vec2 texTopRight    = glm::vec2(1.0f, 1.0f);

    insertData(data,topLeft, normal, texTopLeft);
    insertData(data,bottomLeft, normal, texBottomLeft);
    insertData(data,bottomRight, normal, texBottomRight);

    insertData(data,bottomRight, normal, texBottomRight);
    insertData(data,topRight, normal, texTopRight);
    insertData(data,topLeft, normal, texTopLeft);

    return data;
}

std::vector<float> Quad::createUpsideDownQuad(glm::vec3 position, float width, float height) {
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    std::vector<float> data;

    glm::vec3 topLeft     = position + glm::vec3(-halfWidth,  0.0f, -halfHeight);
    glm::vec3 bottomLeft  = position + glm::vec3(-halfWidth,  0.0f,  halfHeight);
    glm::vec3 bottomRight = position + glm::vec3( halfWidth,  0.0f,  halfHeight);
    glm::vec3 topRight    = position + glm::vec3( halfWidth,  0.0f, -halfHeight);

    glm::vec3 normal = glm::vec3(0.0f, -1.0f, 0.0f);

    glm::vec2 texTopLeft     = glm::vec2(0.0f, 1.0f);
    glm::vec2 texBottomLeft  = glm::vec2(0.0f, 0.0f);
    glm::vec2 texBottomRight = glm::vec2(1.0f, 0.0f);
    glm::vec2 texTopRight    = glm::vec2(1.0f, 1.0f);

    insertData(data,bottomLeft, normal, texBottomLeft);
    insertData(data,topLeft, normal, texTopLeft);
    insertData(data,bottomRight, normal, texBottomRight);

    insertData(data,bottomRight, normal, texBottomRight);
    insertData(data,topLeft, normal, texTopLeft);
    insertData(data,topRight, normal, texTopRight);


    return data;
}

void Quad::insertData(std::vector<float> &data, const glm::vec3 &position, const glm::vec3 &normal,
                      const glm::vec2 &texCoord) {
    insertVec3(data, position);
    // Normal
    insertVec3(data, normal);
    // Texture Coordinate
    insertVec2(data, texCoord);
}

void Quad::setVertexData() {
    float halfWidth = m_width / 2.0f;
    float halfHeight = m_height / 2.0f;

    glm::vec3 topLeft     = glm::vec3(-halfWidth,  0.0f, -halfHeight);
    glm::vec3 bottomLeft  = glm::vec3(-halfWidth,  0.0f,  halfHeight);
    glm::vec3 bottomRight = glm::vec3( halfWidth,  0.0f,  halfHeight);
    glm::vec3 topRight    = glm::vec3( halfWidth,  0.0f, -halfHeight);

    glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec2 texTopLeft     = glm::vec2(0.0f, 1.0f);
    glm::vec2 texBottomLeft  = glm::vec2(0.0f, 0.0f);
    glm::vec2 texBottomRight = glm::vec2(1.0f, 0.0f);
    glm::vec2 texTopRight    = glm::vec2(1.0f, 1.0f);

    appendVertexData(topLeft, normal, texTopLeft);
    appendVertexData(bottomLeft, normal, texBottomLeft);
    appendVertexData(bottomRight, normal, texBottomRight);

    appendVertexData(bottomRight, normal, texBottomRight);
    appendVertexData(topRight, normal, texTopRight);
    appendVertexData(topLeft, normal, texTopLeft);
}

void Quad::appendVertexData(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texCoord) {
    // Position
    insertVec3(m_vertexData, position);
    m_positions.push_back(position);

    // Normal
    insertVec3(m_vertexData, normal);
    m_normals.push_back(normal);

    // Texture Coordinate
    insertVec2(m_vertexData, texCoord);
    m_textureCoords.push_back(texCoord);
}

void Quad::insertVec3(std::vector<float> &data, const glm::vec3 &v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Quad::insertVec2(std::vector<float> &data, const glm::vec2 &v) {
    data.push_back(v.x);
    data.push_back(v.y);
}
