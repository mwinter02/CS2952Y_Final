#include "mesh.h"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;


vector<float> Mesh::updateParams(int param1, int param2){
    return m_vertexData;
}

Mesh::Mesh(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open the file!" << endl;
        return;
    }

    m_vertexData.clear();
    vertices.clear();
    normals.clear();

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string lineHeader;
        iss >> lineHeader;

        if (lineHeader == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        } else if (lineHeader == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
            hasNormals = true;
        } else if (lineHeader == "f") {
            vector<glm::vec3> faceVerts;

            for (int i = 0; i < 3; ++i) {
                string vertexData;
                iss >> vertexData;
                istringstream vertexStream(vertexData);
                string vertIS, normIS;
                getline(vertexStream, vertIS, '/');
                int vertI = stoi(vertIS);
                faceVerts.push_back(vertices[vertI - 1]);

                if (hasNormals) {
                    vertexStream.ignore(numeric_limits<streamsize>::max(), '/');
                    getline(vertexStream, normIS, '/');
                    int normalIndex = stoi(normIS);
                    glm::vec3 t = vertices[vertI - 1];
                    glm::vec3 nt = normals[normalIndex - 1];
                    insertVec3(m_vertexData, t);
                    insertVec3(m_vertexData, nt);
                }
            }

            if (!hasNormals) {
                glm::vec3 norms = getNormal(faceVerts[0], faceVerts[1], faceVerts[2]);
                for (const glm::vec3& vertex : faceVerts) {
                    insertVec3(m_vertexData, vertex);
                    insertVec3(m_vertexData, norms);
                }
            }
        }
    }

    file.close();
}

glm::vec3 Mesh::getNormal(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3) {
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v1;
    return glm::normalize(glm::cross(edge1, edge2));
}

void Mesh::insertVec3(vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
