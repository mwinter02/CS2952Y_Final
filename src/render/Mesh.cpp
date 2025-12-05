#include "Mesh.h"
#include "Graphics.h"
#include "MaterialConstants.h"
#include "../Util.h"
#include "../Debug.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "Texture.h"
#include <fstream>



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


    void setAllMaterials(DrawMesh& mesh, const DrawMaterial& material) {
        for (auto& obj : mesh.objects) {
            obj.material = material;
        }
    }
    DrawMesh Mesh::loadStaticMesh(const char* filename) {
        auto directory = util::getDirectory(filename);
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(util::getPath(filename), IMPORT_PRESET);
        if (!scene) {
            scene = importer.ReadFile(filename, IMPORT_PRESET);
        }
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

    // DrawMesh Mesh::loadStaticMesh(const char* file_name, std::vector<coacd::Mesh>& c_meshes) {
    //     auto directory = util::getDirectory(file_name);
    //     Assimp::Importer importer;
    //     const aiScene* scene = importer.ReadFile(util::getPath(file_name), IMPORT_PRESET);
    //
    //     DrawMesh mesh;
    //     glm::vec3 min(std::numeric_limits<float>::max());
    //     glm::vec3 max(std::numeric_limits<float>::lowest());
    //
    //     auto materials = Texture::loadSceneMaterials(scene, directory);
    //
    //     for (unsigned int i = 0; i < scene->mNumMeshes; i++) { // for each mesh
    //         const aiMesh* aimesh = scene->mMeshes[i];
    //         std::vector<float> gl_data;
    //
    //         // c_meshes.emplace_back();
    //         // auto& c_mesh = c_meshes.back();
    //         //
    //         // for (unsigned int f = 0; f < aimesh->mNumFaces; f++) {
    //         //     aiFace& face = aimesh->mFaces[f];
    //         //     int i1 = face.mIndices[0];
    //         //     int i2 = face.mIndices[1];
    //         //     int i3 = face.mIndices[2];
    //         //     c_mesh.indices.push_back({i1,i2,i3});
    //         // }
    //         //
    //         // for (unsigned int v = 0; v < aimesh->mNumVertices; v++) {
    //         //     aiVector3D& pos = aimesh->mVertices[v];
    //         //     c_mesh.vertices.push_back({pos.x,pos.y,pos.z});
    //         // }
    //
    //         // model.vertices.push_back({pos.x,pos.y,pos.z});
    //
    //         if (!aimesh->HasNormals() || !aimesh->HasPositions()) {
    //             continue;
    //         }
    //         for (unsigned int f = 0; f < aimesh->mNumFaces; f++) { // for each face
    //             const aiFace& face = aimesh->mFaces[f];
    //             for (unsigned int v = 0; v < face.mNumIndices; v++) { // for each vertex in face
    //                 auto index = face.mIndices[v];
    //                 const aiVector3D& pos = aimesh->mVertices[index];
    //                 const aiVector3D& normal = aimesh->mNormals[index];
    //                 const aiVector3D& texcoord = aimesh->HasTextureCoords(0) ? aimesh->mTextureCoords[0][index] : aiVector3D(0.0f, 0.0f, 0.0f);
    //
    //
    //
    //                 gl_data.push_back(pos.x);
    //                 gl_data.push_back(pos.y);
    //                 gl_data.push_back(pos.z);
    //
    //                 gl_data.push_back(normal.x);
    //                 gl_data.push_back(normal.y);
    //                 gl_data.push_back(normal.z);
    //
    //                 gl_data.push_back(texcoord.x);
    //                 gl_data.push_back(texcoord.y);
    //             }
    //         }
    //         auto material_name = scene->mMaterials[aimesh->mMaterialIndex]->GetName().C_Str();
    //
    //         DrawObject object;
    //         object.shape = loadStaticShape(gl_data);
    //         object.material = materials[material_name];
    //         mesh.objects.push_back(object);
    //         min = glm::min(min, object.shape.min);
    //         max = glm::max(max, object.shape.max);
    //     }
    //     mesh.min = min;
    //     mesh.max = max;
    //
    //     return mesh;
    // }

    /**
     *
     * @param file_name
     * @param quality - A value between 0 and 1 indicating the quality of the decomposition (higher is better)
     * @return
     */
    DrawMesh Mesh::decomposeObj(const char* file_name, float quality) {

        float threshold = std::clamp(1.f-quality, 0.01f, 1.f);
        std::string full_path = util::getPath(file_name);
        std::string directory = util::getDirectory(file_name);

        std::string output_name = file_name;
        if (util::removeExtension(output_name) != ".obj") {
            debug::error("Input file must be an OBJ file for decomposition: " + full_path);
            return DrawMesh{};
        }
        output_name += "_collider.obj";


        if (!generateObjPyScript(file_name, output_name.c_str(), threshold)) {
            debug::error("Failed to generate coacd obj file for: " + full_path);
            debug::error("Ensure CoACD + trimesh is installed, use 'pip install coacd trimesh'");
            return DrawMesh{};
        }
        // coacdJsonToObj(util::getPath(json_name).c_str(), util::getPath(output_name.c_str()).c_str());

        return loadColliderMeshObj(output_name.c_str());
    }

    DrawMesh Mesh::loadColliderMeshObj(const char* filename) {
        auto mesh = loadStaticMesh(filename);
        setAllMaterials(mesh, colliderMaterial);

        auto colors = getRainbow(mesh.objects.size());

        for (int i=0; i<mesh.objects.size(); i++) {
            auto& obj = mesh.objects[i];
            auto color = colors[i];
            obj.material = getColliderMaterial(color);
        }
        return mesh;
    }


    bool Mesh::generateObjPyScript(const char* obj_path, const char* output_path, float threshold) {
        std::string obj_fs = util::getPath(obj_path);
        std::string outpus_fs = util::getPath(output_path);
        std::string script_fs = util::getPath("src/python/coacd_preprocess.py");

        // Build command: python3 <script> <obj> <json> --threshold <value>
        char cmd[4096];
        std::snprintf(cmd, sizeof(cmd),
                      "python3 \"%s\" \"%s\" \"%s\" --threshold %f",
                      script_fs.c_str(), obj_fs.c_str(), outpus_fs.c_str(), threshold);

        debug::print("Running CoACD preprocessor: ");
        debug::print(cmd);

        int ret = std::system(cmd);
        if (ret != 0) {
            debug::print("CoACD preprocessor failed with exit code: ");
            debug::print(std::to_string(ret));
            std::remove(outpus_fs.c_str()); // remove incomplete file
            return false;
        }
        return true;
    }

    std::ofstream createObjFile(const std::string& filepath) {
        std::filesystem::path p(filepath);

        if (std::filesystem::exists(p)) {
            std::filesystem::remove(p);
        }
        if (p.extension() != ".obj") p += ".obj";

        return std::ofstream(p);
    }

    void writeObjNewShape(std::ofstream& ofs, const std::string& name) {
        ofs << "\n#\n# New Shape: " << name << "\n#\n";
        ofs << "g " << name << std::endl;
    }

    void writeObjVertex(std::ofstream& ofs, const glm::vec3& v) {
        ofs << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    void writeObjFace(std::ofstream& ofs, const int i1, const int i2, const int i3) {
        // Obj face indexes from 1 not 0
        ofs << "f " << i1+1 << " " << i2+1 << " " << i3+1 << "\n";
    }

}
