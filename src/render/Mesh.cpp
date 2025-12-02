#include "Mesh.h"
#include "Graphics.h"
#include "MaterialConstants.h"
#include "../Util.h"
#include "../Debug.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <stb_image.h>

#include "Texture.h"


namespace gl {



    DrawShape Mesh::loadStaticShape(const std::vector<float>& data) {
        int attribute_size = (3 + 3 + 2);
        GLsizei stride = attribute_size * sizeof(float);

        DrawShape shape;
        GLuint vao;
        GLuint vbo;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

        glEnableVertexAttribArray(1); // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

        glEnableVertexAttribArray(2); // texcoord
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

        glBindVertexArray(0);

        glm::vec3 bmin(FLT_MAX);
        glm::vec3 bmax(-FLT_MAX);
        for (int x = 0; x < data.size() - 2; x += attribute_size) {
            glm::vec3 v = {data[x], data[x + 1], data[x + 2]};
            bmin = glm::min(bmin, v);
            bmax = glm::max(bmax, v);
        }

        shape.vao = vao;
        shape.vbo = vbo;
        shape.numTriangles = data.size() / (3 + 3 + 2) / 3;
        shape.min = bmin;
        shape.max = bmax;

        return shape;
    }

    constexpr unsigned int IMPORT_PRESET =  aiProcess_Triangulate |
                                            aiProcess_JoinIdenticalVertices |
                                            aiProcess_OptimizeMeshes |
                                            aiProcess_GenSmoothNormals |
                                            aiProcess_CalcTangentSpace |
                                            aiProcess_FlipUVs | // since OpenGL's UVs are flipped
                                            aiProcess_LimitBoneWeights; // Limit bone weights to 4 per vertex

    /**
     * Loads a static mesh from file, supporting various formats (OBJ, FBX, etc.)
     * @param filename - The file path to the mesh from project root, e.g "Resources/Models/model.obj"
     * @return DrawMesh struct containing the loaded mesh data
     */
    DrawMesh Mesh::loadStaticMesh(const char* filename) {
        auto directory = util::getDirectory(filename);
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(util::getPath(filename), IMPORT_PRESET);

        DrawMesh mesh;
        glm::vec3 min(std::numeric_limits<float>::max());
        glm::vec3 max(std::numeric_limits<float>::lowest());

        auto materials = Texture::loadSceneMaterials(scene, directory);

        for (unsigned int i = 0; i < scene->mNumMeshes; i++) { // for each mesh
            const aiMesh* aimesh = scene->mMeshes[i];
            std::vector<float> vertex_data;

            if (!aimesh->HasNormals() || !aimesh->HasPositions()) {
                continue;
            }
            for (unsigned int f = 0; f < aimesh->mNumFaces; f++) { // for each face
                const aiFace& face = aimesh->mFaces[f];
                for (unsigned int v = 0; v < face.mNumIndices; v++) { // for each vertex in face
                    auto index = face.mIndices[v];
                    const aiVector3D& pos = aimesh->mVertices[index];
                    const aiVector3D& normal = aimesh->mNormals[index];
                    const aiVector3D& texcoord = aimesh->HasTextureCoords(0) ? aimesh->mTextureCoords[0][index] : aiVector3D(0.0f, 0.0f, 0.0f);

                    vertex_data.push_back(pos.x);
                    vertex_data.push_back(pos.y);
                    vertex_data.push_back(pos.z);

                    vertex_data.push_back(normal.x);
                    vertex_data.push_back(normal.y);
                    vertex_data.push_back(normal.z);

                    vertex_data.push_back(texcoord.x);
                    vertex_data.push_back(texcoord.y);
                }
            }
            auto material_name = scene->mMaterials[aimesh->mMaterialIndex]->GetName().C_Str();

            DrawObject object;
            object.shape = loadStaticShape(vertex_data);
            object.material = materials[material_name];
            mesh.objects.push_back(object);
            min = glm::min(min, object.shape.min);
            max = glm::max(max, object.shape.max);
        }
        mesh.min = min;
        mesh.max = max;

        return mesh;
    }






    void Mesh::setTextureFlags(Textures& texture) {
        if (texture.ambient != 0) {
            texture.flags |= material::TEXTURE_FLAG_AMBIENT;
        }
        if (texture.diffuse != 0) {
            texture.flags |= material::TEXTURE_FLAG_DIFFUSE;
        }
        if (texture.specular != 0) {
            texture.flags |= material::TEXTURE_FLAG_SPECULAR;
        }

    }
}
