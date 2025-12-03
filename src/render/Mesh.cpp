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
#include <nlohmann/json.hpp>
#include <cstdlib>



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
        std::string json_name = directory + "/coacd_temp.json";

        if (!generateCoacdJson(file_name, json_name.c_str(), threshold)) {
            debug::error("Failed to generate coacd json file for: " + full_path);
            debug::error("Ensure CoACD + trimesh is installed, use 'pip install coacd trimesh'");
            return DrawMesh{};
        }

        std::string output_path = directory + "/" + util::getStem(full_path) + "_collider.obj";
        coacdJsonToObj(util::getPath(json_name).c_str(), util::getPath(output_path.c_str()).c_str());

        auto mesh = loadStaticMesh(output_path.c_str());
        setAllMaterials(mesh, colliderMaterial);

        return mesh;
    }


    // Helper: given an OBJ path, run the Python CoACD preprocessor to create the JSON file.
    // This assumes Python and the `coacd` + `trimesh` packages are installed and that
    // src/python/coacd_preprocess.py exists relative to the project root.
    //
    // `obj_path` and `json_path` are relative to the project root in the same
    // way you pass paths to loadStaticMesh / loadCoacdJson.
    // Returns true on apparent success (Python exited with code 0).
    bool Mesh::generateCoacdJson(const char* obj_path, const char* json_path, float threshold) {
        // Resolve paths to actual filesystem locations
        std::string obj_fs = util::getPath(obj_path);
        std::string json_fs = util::getPath(json_path);
        std::string script_fs = util::getPath("src/python/coacd_preprocess.py");

        // Build command: python3 <script> <obj> <json> --threshold <value>
        char cmd[4096];
        std::snprintf(cmd, sizeof(cmd),
                      "python3 \"%s\" \"%s\" \"%s\" --threshold %f",
                      script_fs.c_str(), obj_fs.c_str(), json_fs.c_str(), threshold);

        debug::print("Running CoACD preprocessor: ");
        debug::print(cmd);

        int ret = std::system(cmd);
        if (ret != 0) {
            debug::print("CoACD preprocessor failed with exit code: ");
            debug::print(std::to_string(ret));
            std::remove(json_fs.c_str()); // remove incomplete file
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
    }

    void writeObjVertex(std::ofstream& ofs, const glm::vec3& v) {
        ofs << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    void writeObjFace(std::ofstream& ofs, const int i1, const int i2, const int i3) {
        // Obj face indexes from 1 not 0
        ofs << "f " << i1+1 << " " << i2+1 << " " << i3+1 << "\n";
    }

    void Mesh::coacdJsonToObj(const char* json_full_path, const char* collider_output_obj) {
        // Open and parse JSON
        std::ifstream in(json_full_path);
        if (!in) {
            debug::print("Failed to open CoACD JSON file: ");
            return;
        }

        nlohmann::json j;
        in >> j;

        if (!j.contains("parts") || !j["parts"].is_array()) {
            debug::print("Invalid CoACD JSON format (no 'parts' array):");
            return;
        }

        auto obj_writer = createObjFile(collider_output_obj);

        int shape_no = 0;

        int vertex_offset = 0;
        for (const auto& part : j["parts"]) {
            if (!part.contains("vertices") || !part.contains("indices")) continue;

            writeObjNewShape(obj_writer, "Collider: " + std::to_string(shape_no));
            shape_no++;
            const auto& vflat = part["vertices"];
            const auto& iflat = part["indices"];
            if (!vflat.is_array() || !iflat.is_array()) continue;
            int current_vertex_offset = 0;
            for (size_t i = 0; i + 2 < vflat.size(); i += 3) {
                float x = vflat[i].get<float>();
                float y = vflat[i + 1].get<float>();
                float z = vflat[i + 2].get<float>();
                writeObjVertex(obj_writer, glm::vec3(x, y, z));
                current_vertex_offset ++;
            }

            for (size_t i = 0; i + 2 < iflat.size(); i += 3) {
                int i0 = iflat[i].get<int>() + vertex_offset;
                int i1 = iflat[i + 1].get<int>() + vertex_offset;
                int i2 = iflat[i + 2].get<int>() + vertex_offset;
                if (i0 < 0 || i1 < 0 || i2 < 0) continue;
                writeObjFace(obj_writer, i0, i1, i2);
            }
            vertex_offset += current_vertex_offset;
        }
        obj_writer.close();
        std::remove(json_full_path); // clean up temp file after use
    }

    DrawMesh Mesh::loadCoacdJson(const char* json_filename, const char* collider_output_obj) {
        DrawMesh mesh;
        glm::vec3 min(std::numeric_limits<float>::max());
        glm::vec3 max(std::numeric_limits<float>::lowest());

        // Open and parse JSON
        std::ifstream in(util::getPath(json_filename));
        if (!in) {
            debug::print("Failed to open CoACD JSON file: ");
            return mesh;
        }

        nlohmann::json j;
        in >> j;

        if (!j.contains("parts") || !j["parts"].is_array()) {
            debug::print("Invalid CoACD JSON format (no 'parts' array):");
            return mesh;
        }

        // For now, render all convex parts with a default material.
        // We reuse the first material from a dummy load of the source model if available.
        // Otherwise, we leave material at its default.
        //
        // Note: You can extend this later to carry material info through JSON.

        auto obj_writer = createObjFile(collider_output_obj);

        int shape_no = 0;
        for (const auto& part : j["parts"]) {
            if (!part.contains("vertices") || !part.contains("indices")) continue;

            writeObjNewShape(obj_writer, "Collider: " + std::to_string(shape_no));
            shape_no++;
            const auto& vflat = part["vertices"];
            const auto& iflat = part["indices"];
            if (!vflat.is_array() || !iflat.is_array()) continue;

            std::vector<float> gl_data;

            // Rebuild positions from flat vertex array
            std::vector<glm::vec3> positions;
            positions.reserve(vflat.size() / 3);
            for (size_t i = 0; i + 2 < vflat.size(); i += 3) {
                float x = vflat[i].get<float>();
                float y = vflat[i + 1].get<float>();
                float z = vflat[i + 2].get<float>();
                positions.emplace_back(x, y, z);
                writeObjVertex(obj_writer, glm::vec3(x, y, z));
            }

            // Compute a simple face-normal per triangle for now
            for (size_t i = 0; i + 2 < iflat.size(); i += 3) {
                int i0 = iflat[i].get<int>();
                int i1 = iflat[i + 1].get<int>();
                int i2 = iflat[i + 2].get<int>();
                if (i0 < 0 || i1 < 0 || i2 < 0) continue;
                if ((size_t)i0 >= positions.size() || (size_t)i1 >= positions.size() || (size_t)i2 >= positions.size()) continue;

                writeObjFace(obj_writer, i0, i1, i2);

                glm::vec3 p0 = positions[i0];
                glm::vec3 p1 = positions[i1];
                glm::vec3 p2 = positions[i2];
                glm::vec3 n = glm::normalize(glm::cross(p1 - p0, p2 - p0));

                // For simplicity, we duplicate vertices per triangle, matching existing GL layout
                const glm::vec2 uv(0.0f); // no UVs from CoACD output

                auto push_vertex = [&](const glm::vec3& p) {
                    gl_data.push_back(p.x);
                    gl_data.push_back(p.y);
                    gl_data.push_back(p.z);
                    gl_data.push_back(n.x);
                    gl_data.push_back(n.y);
                    gl_data.push_back(n.z);
                    gl_data.push_back(uv.x);
                    gl_data.push_back(uv.y);
                };

                push_vertex(p0);
                push_vertex(p1);
                push_vertex(p2);
            }


            if (gl_data.empty()) continue;

            DrawObject object;
            object.shape = loadStaticShape(gl_data);
            object.material = colliderMaterial;
            // Use a default material; caller can override if desired.
            mesh.objects.push_back(object);
            min = glm::min(min, object.shape.min);
            max = glm::max(max, object.shape.max);
        }

        obj_writer.close();


        mesh.min = min;
        mesh.max = max;
        return mesh;
    }
}
