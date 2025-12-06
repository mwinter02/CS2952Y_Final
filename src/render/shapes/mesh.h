#pragma once

#include "glm/fwd.hpp"
#include <string>
#include <vector>

class Mesh {

public:
    Mesh(const std::string& filename);
    std::vector<float> updateParams(int param1, int param2);

private:
    glm::vec3 getNormal(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    std::vector<float> m_vertexData;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    bool hasNormals = false;
    int param;
};


