#include "SkeletalMesh.h"

#include <ranges>

#include "ConvexHull.hpp"
#include "Mesh.h"
#include "QuickHull.hpp"
#include "../Debug.h"
#include "../Util.h"
#include "assimp/Exporter.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"


namespace gl {

    // Helper function to find the index of the keyframe just before or at the given time
    template<typename T>
    int findKeyframeIndex(const std::vector<Keyframe<T>>& keys, double time) {
        for (int i = 0; i < keys.size() - 1; i++) {
            if (time < keys[i + 1].time) {
                return i;
            }
        }
        return keys.size() - 1;
    }

    // Interpolate position (linear interpolation)
    glm::vec3 interpolatePosition(const std::vector<PositionKey>& keys, double time) {
        if (keys.empty()) {
            return glm::vec3(0.0f);
        }
        if (keys.size() == 1) {
            return keys[0].value;
        }

        int index = findKeyframeIndex(keys, time);
        int next_index = index + 1;

        // Clamp to last keyframe if we're past the end
        if (next_index >= keys.size()) {
            return keys[index].value;
        }

        const auto& key1 = keys[index];
        const auto& key2 = keys[next_index];

        double delta_time = key2.time - key1.time;
        float factor = static_cast<float>((time - key1.time) / delta_time);

        // Linear interpolation
        return glm::mix(key1.value, key2.value, factor);
    }

    // Interpolate rotation (spherical linear interpolation for smooth rotation)
    glm::quat interpolateRotation(const std::vector<RotationKey>& keys, double time) {
        if (keys.empty()) {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
        }
        if (keys.size() == 1) {
            return keys[0].value;
        }

        int index = findKeyframeIndex(keys, time);
        int next_index = index + 1;

        // Clamp to last keyframe if we're past the end
        if (next_index >= keys.size()) {
            return keys[index].value;
        }

        const auto& key1 = keys[index];
        const auto& key2 = keys[next_index];

        double delta_time = key2.time - key1.time;
        float factor = static_cast<float>((time - key1.time) / delta_time);

        // Spherical linear interpolation (slerp) for smooth rotation
        return glm::slerp(key1.value, key2.value, factor);
    }

    // Interpolate scale (linear interpolation)
    glm::vec3 interpolateScale(const std::vector<ScaleKey>& keys, double time) {
        if (keys.empty()) {
            return glm::vec3(1.0f); // Default scale
        }
        if (keys.size() == 1) {
            return keys[0].value;
        }

        int index = findKeyframeIndex(keys, time);
        int next_index = index + 1;

        // Clamp to last keyframe if we're past the end
        if (next_index >= keys.size()) {
            return keys[index].value;
        }

        const auto& key1 = keys[index];
        const auto& key2 = keys[next_index];

        double delta_time = key2.time - key1.time;
        float factor = static_cast<float>((time - key1.time) / delta_time);

        // Linear interpolation
        return glm::mix(key1.value, key2.value, factor);
    }

    glm::mat4 AnimationChannel::calculateTransform(double animation_time) const {
        const glm::vec3 position = interpolatePosition(position_keys, animation_time);
        const glm::quat rotation = interpolateRotation(rotation_keys, animation_time);
        const glm::vec3 scale = interpolateScale(scale_keys, animation_time);

        return glm::translate(glm::mat4(1.0f), position)
             * glm::mat4_cast(rotation)
             * glm::scale(glm::mat4(1.0f), scale);
    }

    std::string findRootBone(aiNode* curr_node, const std::unordered_set<std::string>& bone_map) {
        if (bone_map.contains(curr_node->mName.C_Str())) {
            return curr_node->mName.C_Str();
        }
        for (size_t i = 0; i < curr_node->mNumChildren; i++) {
            auto result = findRootBone(curr_node->mChildren[i], bone_map);
            if (!result.empty()) {
                return result;
            }
        }
        debug::error("Failed to find root bone");
        return "";
    }

    glm::mat4 getBoneMatrix(const std::string& bone_name, const aiScene* scene) {
        if (auto bone = scene->findBone(aiString(bone_name))) {
            return util::aiToGlmMat4(bone->mOffsetMatrix);
        }

        debug::error("Failed to find bone " + bone_name);
        return {1.0f};
    }

    void constructSkeleton(Skeleton& skeleton, aiNode* curr_node, const aiScene* scene,
                          int parent_id, const glm::mat4& parent_global_transform) {
        aiBone* ai_bone = scene->findBone(curr_node->mName);
        const glm::mat4 node_local = util::aiToGlmMat4(curr_node->mTransformation);
        const glm::mat4 node_global = parent_global_transform * node_local;

        if (ai_bone) {
            const int current_id = (int) skeleton.bones_.size();
            const glm::mat4 offset_matrix = util::aiToGlmMat4(ai_bone->mOffsetMatrix);

            // The offset matrix is the INVERSE of the bind pose global transform
            // So the bind pose global transform is the inverse of the offset matrix
            const glm::mat4 bind_pose_global = glm::inverse(offset_matrix);

            // Determine the local transform to store
            glm::mat4 local_transform;
            if (parent_id == -1) {
                // Root bone: local_transform IS the bind pose global transform
                local_transform = bind_pose_global;
            } else {
                // Child bone: extract local transform by dividing out parent's bind pose
                // child_global = parent_global × child_local
                // child_local = inverse(parent_global) × child_global
                const glm::mat4& parent_bind_global = glm::inverse(skeleton.bones_[parent_id].offset_matrix);
                local_transform = glm::inverse(parent_bind_global) * bind_pose_global;
            }

            skeleton.addBone(curr_node->mName.C_Str(), current_id, parent_id, offset_matrix, local_transform);

            // Continue to children with this bone as parent (use node_global for hierarchy traversal)
            for (size_t i = 0; i < curr_node->mNumChildren; i++) {
                constructSkeleton(skeleton, curr_node->mChildren[i], scene, current_id, node_global);
            }
        }
        else {
            // Current node not a bone, accumulate its transform but don't change parent_id
            for (size_t i = 0; i < curr_node->mNumChildren; i++) {
                constructSkeleton(skeleton, curr_node->mChildren[i], scene, parent_id, node_global);
            }
        }
    }

    constexpr unsigned int SKINNED_IMPORT_PRESET =  aiProcess_Triangulate |
                                                    aiProcess_JoinIdenticalVertices |
                                                    aiProcess_OptimizeMeshes |
                                                    aiProcess_GenSmoothNormals |
                                                    aiProcess_CalcTangentSpace |
                                                    // aiProcess_PopulateArmatureData | // Create bone hierarchy
                                                    aiProcess_FlipUVs | // since OpenGL's UVs are flipped
                                                    aiProcess_LimitBoneWeights; // Limit bone weights to 4 per vertex


    Skeleton loadSkeleton(const aiScene* scene) {
        auto root_transform = util::aiToGlmMat4(scene->mRootNode->mTransformation);

        Skeleton skeleton;
        std::unordered_set<std::string> bone_names;

        for (int i=0; i < scene->mNumMeshes; i++) {
            const aiMesh* aimesh = scene->mMeshes[i];
            if (!aimesh->HasBones()) { continue; }


            for (size_t b = 0; b < aimesh->mNumBones; b++) {
                const aiBone* bone = aimesh->mBones[b];

                std::string bone_name = bone->mName.C_Str();
                if (!bone_names.contains(bone_name)) {
                    bone_names.insert(bone_name);
                }
            }
        }
        std::string root_name = findRootBone(scene->mRootNode, bone_names);
        auto node = scene->mRootNode->FindNode(aiString(root_name));
        constructSkeleton(skeleton, node, scene, -1, glm::mat4(1.0f));
        return skeleton;
    }

    void Skeleton::addBone(const std::string& bone_name, const unsigned int current_id, const int parent_id,
        const glm::mat4& offset_matrix, const glm::mat4& local_transform) {

        const auto to_add = Bone(bone_name, current_id, parent_id, offset_matrix, local_transform);
        bones_.push_back(to_add);
        bone_matrices_.push_back(offset_matrix);
        bone_map_[to_add.name] = current_id;
        num_bones_ = (unsigned int) bones_.size();
        if (parent_id != -1) bones_[parent_id].addChild(to_add);
    }


    void Skeleton::traverseBoneHierarchy(const unsigned int bone_id, const glm::mat4& parent_transform) {
        auto& bone = bones_[bone_id];

        // Accumulate transforms: parent's global transform × this bone's local transform
        const glm::mat4 global_transform = parent_transform * bone.local_transform;

        // Calculate final bone matrix for shader
        bone_matrices_[bone_id] = global_transform * bone.offset_matrix;

        // Recursively update all children with this bone's global transform as their parent
        for (const auto& child_id : bone.children) {
            traverseBoneHierarchy(child_id, global_transform);
        }
    }

    void Skeleton::updateBoneMatrices() {
        // Ensure bone_matrices_ is sized correctly
        if (bone_matrices_.size() != num_bones_) {
            bone_matrices_.resize(num_bones_, glm::mat4(1.0f));
        }

        // Find and traverse from all root bones (those with no parent)
        for (unsigned int i = 0; i < bones_.size(); i++) {
            if (bones_[i].parent_id == -1) {
                // Start traversal from root with identity parent transform
                traverseBoneHierarchy(i, glm::mat4(1.0f));
                break;
            }
        }
    }

    void Skeleton::setCurrentAnimation(const std::string& animation_name) {
        if (!animations_.contains(animation_name)) {
            debug::error("Animation " + animation_name + " not found in skeleton.");
            return;
        }
        current_animation_ = &animations_.at(animation_name);
    }

    void Skeleton::playCurrentAnimation(double current_time) {
        if (current_animation_) {
            const double time_in_ticks = current_time * current_animation_->ticks_per_second;
            const double animation_time = fmod(time_in_ticks, current_animation_->duration);
            // Update each bone's local transform based on animation channels
            for (auto& [bone_id, channel] : current_animation_->channels) {
                bones_[bone_id].local_transform = channel.calculateTransform(animation_time);
            }
        }

        updateBoneMatrices();
    }

    void Skeleton::resetToBindPose() {
        for (auto& bone : bones_) {
            bone.local_transform = bone.bind_pose_transform;
        }
    }

    std::unordered_map<std::string, Animation> loadAnimations(const aiScene* scene, const Skeleton& skeleton) {
        if (!scene->HasAnimations()) return {};
        std::unordered_map<std::string,Animation> animations_map;
        for (size_t i = 0; i < scene->mNumAnimations; i++) {
            const aiAnimation* ai_anim = scene->mAnimations[i];


            Animation animation;
            animation.ticks_per_second = ai_anim->mTicksPerSecond != 0.0 ? ai_anim->mTicksPerSecond : 25.0;
            animation.duration = ai_anim->mDuration;
            for (size_t c = 0; c < ai_anim->mNumChannels; c++) {
                const aiNodeAnim* ai_channel = ai_anim->mChannels[c];
                const std::string bone_name = ai_channel->mNodeName.C_Str();
                AnimationChannel channel;

                // Skip channels for bones not in skeleton
                if (!skeleton.bone_map_.contains(bone_name)) {
                    // debug::print("Skipping animation channel for unknown bone: " + bone_name);
                    // These are often bone ends which can be used for inverse kinematics
                    // Also the root node is skipped which can be used for global transforms
                    continue;
                }

                channel.bone_id = skeleton.bone_map_.at(bone_name);
                channel.bone_name = bone_name;

                // Position keys
                for (size_t pk = 0; pk < ai_channel->mNumPositionKeys; pk++) {
                    const aiVectorKey& ai_pos_key = ai_channel->mPositionKeys[pk];
                    PositionKey pos_key;
                    pos_key.time = ai_pos_key.mTime;
                    pos_key.value = glm::vec3(ai_pos_key.mValue.x, ai_pos_key.mValue.y, ai_pos_key.mValue.z);
                    channel.position_keys.push_back(pos_key);
                }

                // Rotation keys
                for (size_t rk = 0; rk < ai_channel->mNumRotationKeys; rk++) {
                    const aiQuatKey& ai_rot_key = ai_channel->mRotationKeys[rk];
                    RotationKey rot_key;
                    rot_key.time = ai_rot_key.mTime;
                    rot_key.value = glm::quat(ai_rot_key.mValue.w, ai_rot_key.mValue.x, ai_rot_key.mValue.y, ai_rot_key.mValue.z);
                    channel.rotation_keys.push_back(rot_key);
                }

                // Scale keys
                for (size_t sk = 0; sk < ai_channel->mNumScalingKeys; sk++) {
                    const aiVectorKey& ai_scale_key = ai_channel->mScalingKeys[sk];
                    ScaleKey scale_key;
                    scale_key.time = ai_scale_key.mTime;
                    scale_key.value = glm::vec3(ai_scale_key.mValue.x, ai_scale_key.mValue.y,  ai_scale_key.mValue.z);
                    channel.scale_keys.push_back(scale_key);
                }

                animation.channels[channel.bone_id] = channel;
            }

            animations_map[ai_anim->mName.C_Str()] = animation;
        }
        return animations_map;
    }

    void SkeletalMesh::exportWithColliders(const char* output_path,
                                           const Skeleton& skeleton,
                                           const DrawMesh& collision_mesh,
                                           const aiScene* original_scene) {
        // Create a new scene that will contain both original and collision meshes
        aiScene* export_scene = nullptr;

        // Use Assimp's CopyScene for proper deep copy
        aiCopyScene(original_scene, &export_scene);

        // Now we need to expand materials array to add collision materials
        unsigned int old_mat_count = export_scene->mNumMaterials;
        unsigned int new_mat_count = old_mat_count + collision_mesh.objects.size();

        aiMaterial** new_materials = new aiMaterial*[new_mat_count];

        // Copy existing material pointers
        for (unsigned int i = 0; i < old_mat_count; i++) {
            new_materials[i] = export_scene->mMaterials[i];
        }

        // Add collision mesh materials
        for (size_t i = 0; i < collision_mesh.objects.size(); i++) {
            aiMaterial* collision_mat = new aiMaterial();
            aiString mat_name("CollisionMaterial_" + std::to_string(i));
            collision_mat->AddProperty(&mat_name, AI_MATKEY_NAME);

            const auto& mat = collision_mesh.objects[i].material;
            aiColor3D diffuse(mat.diffuse.r, mat.diffuse.g, mat.diffuse.b);
            collision_mat->AddProperty(&diffuse, 1, AI_MATKEY_COLOR_DIFFUSE);

            float opacity = mat.opacity;
            collision_mat->AddProperty(&opacity, 1, AI_MATKEY_OPACITY);

            new_materials[old_mat_count + i] = collision_mat;
        }

        // Replace materials array
        delete[] export_scene->mMaterials;
        export_scene->mMaterials = new_materials;
        export_scene->mNumMaterials = new_mat_count;

        // Expand meshes array to add collision meshes
        unsigned int old_mesh_count = export_scene->mNumMeshes;
        unsigned int new_mesh_count = old_mesh_count + collision_mesh.objects.size();

        aiMesh** new_meshes = new aiMesh*[new_mesh_count];

        // Copy existing mesh pointers
        for (unsigned int i = 0; i < old_mesh_count; i++) {
            new_meshes[i] = export_scene->mMeshes[i];
        }

        // Replace meshes array (we'll populate collision meshes next)
        delete[] export_scene->mMeshes;
        export_scene->mMeshes = new_meshes;
        export_scene->mNumMeshes = new_mesh_count;

        // Convert DrawMesh collision hulls to aiMesh
        for (size_t i = 0; i < collision_mesh.objects.size(); i++) {
            const auto& obj = collision_mesh.objects[i];
            const auto& shape = obj.shape;

            aiMesh* ai_collision = new aiMesh();
            ai_collision->mName = aiString("CollisionHull_" + std::to_string(i));
            ai_collision->mMaterialIndex = old_mat_count + i;
            ai_collision->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

            // Read back vertex data from GPU buffers
            std::vector<glm::vec3> positions(shape.numTriangles * 3);
            std::vector<glm::vec3> normals(shape.numTriangles * 3);
            std::vector<BoneIDs> bone_ids(shape.numTriangles * 3);
            std::vector<BoneWeights> bone_weights(shape.numTriangles * 3);

            glBindVertexArray(shape.vao);

            // Read positions (attribute 0)
            GLuint vbo_pos;
            glGetVertexAttribIuiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vbo_pos);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

            // Read normals (attribute 1)
            GLuint vbo_normal;
            glGetVertexAttribIuiv(1, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vbo_normal);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, normals.size() * sizeof(glm::vec3), normals.data());

            // Read bone IDs and weights
            GLuint vbo_bone_ids, vbo_bone_weights;
            glGetVertexAttribIuiv(3, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vbo_bone_ids);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_bone_ids);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, bone_ids.size() * sizeof(BoneIDs), bone_ids.data());

            glGetVertexAttribIuiv(4, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vbo_bone_weights);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_bone_weights);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, bone_weights.size() * sizeof(BoneWeights), bone_weights.data());

            glBindVertexArray(0);

            // Populate aiMesh vertices
            ai_collision->mNumVertices = positions.size();
            ai_collision->mVertices = new aiVector3D[ai_collision->mNumVertices];
            ai_collision->mNormals = new aiVector3D[ai_collision->mNumVertices];

            for (size_t v = 0; v < positions.size(); v++) {
                ai_collision->mVertices[v] = aiVector3D(positions[v].x, positions[v].y, positions[v].z);
                ai_collision->mNormals[v] = aiVector3D(normals[v].x, normals[v].y, normals[v].z);
            }

            // Create faces
            ai_collision->mNumFaces = shape.numTriangles;
            ai_collision->mFaces = new aiFace[ai_collision->mNumFaces];

            for (unsigned int f = 0; f < shape.numTriangles; f++) {
                aiFace& face = ai_collision->mFaces[f];
                face.mNumIndices = 3;
                face.mIndices = new unsigned int[3];
                face.mIndices[0] = f * 3;
                face.mIndices[1] = f * 3 + 1;
                face.mIndices[2] = f * 3 + 2;
            }

            // Add bones to collision mesh
            std::unordered_map<unsigned int, std::vector<std::pair<unsigned int, float>>> bone_vertex_map;

            for (size_t v = 0; v < bone_ids.size(); v++) {
                for (int b = 0; b < MAX_BONES_PER_VERTEX; b++) {
                    if (bone_weights[v][b] > 0.0f) {
                        bone_vertex_map[bone_ids[v][b]].emplace_back(v, bone_weights[v][b]);
                    }
                }
            }

            ai_collision->mNumBones = bone_vertex_map.size();
            ai_collision->mBones = new aiBone*[ai_collision->mNumBones];

            size_t bone_idx = 0;
            for (const auto& [bone_id, weights] : bone_vertex_map) {
                const auto& skel_bone = skeleton.bones_[bone_id];

                aiBone* ai_bone = new aiBone();
                ai_bone->mName = aiString(skel_bone.name);
                ai_bone->mOffsetMatrix = util::glmToAiMat4(skel_bone.offset_matrix); // You'll need this helper
                ai_bone->mNumWeights = weights.size();
                ai_bone->mWeights = new aiVertexWeight[weights.size()];

                for (size_t w = 0; w < weights.size(); w++) {
                    ai_bone->mWeights[w].mVertexId = weights[w].first;
                    ai_bone->mWeights[w].mWeight = weights[w].second;
                }

                ai_collision->mBones[bone_idx++] = ai_bone;
            }

            export_scene->mMeshes[old_mesh_count + i] = ai_collision;
        }

        // Add collision meshes to root node
        aiNode* collision_node = new aiNode("CollisionMeshes");
        collision_node->mNumMeshes = collision_mesh.objects.size();
        collision_node->mMeshes = new unsigned int[collision_node->mNumMeshes];

        for (size_t i = 0; i < collision_mesh.objects.size(); i++) {
            collision_node->mMeshes[i] = old_mesh_count + i;
        }

        // Attach collision node to root
        unsigned int old_num_children = export_scene->mRootNode->mNumChildren;
        aiNode** new_children = new aiNode*[old_num_children + 1];

        for (unsigned int i = 0; i < old_num_children; i++) {
            new_children[i] = export_scene->mRootNode->mChildren[i];
        }
        new_children[old_num_children] = collision_node;
        collision_node->mParent = export_scene->mRootNode;

        delete[] export_scene->mRootNode->mChildren;
        export_scene->mRootNode->mChildren = new_children;
        export_scene->mRootNode->mNumChildren = old_num_children + 1;

        // Export FBX
        Assimp::Exporter exporter;
        aiReturn result = exporter.Export(export_scene, "fbx", output_path, aiProcess_FlipUVs);

        if (result != AI_SUCCESS) {
            debug::error("Failed to export FBX: " + std::string(exporter.GetErrorString()));
        }

        delete export_scene;
    }

    static Assimp::Importer importer;
    SkinnedMesh SkeletalMesh::loadFbx(const char* filename) {
        auto directory = util::getDirectory(filename);


        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

        // aiProcess_LimitBoneWeights; // Limit bone weights to 4 per vertex


        auto path = util::getPath(filename);
        const aiScene* scene = importer.ReadFile(path,SKINNED_IMPORT_PRESET);     // Calculate tangent space for normal mapping

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            debug::error("Failed to load FBX: " + std::string(importer.GetErrorString()));
            return {};
        }

        auto skeleton = loadSkeleton(scene);
        skeleton.animations_ = loadAnimations(scene, skeleton);
        std::vector<AnimationChannel> channels;
        for (const auto& [k, v] : skeleton.animations_["Take 001"].channels) {
            channels.push_back(v);
        }

        skeleton.updateBoneMatrices();  // Calculate bone matrices for bind pose

        auto materials = Texture::loadSceneMaterials(scene, directory);
        DrawMesh mesh;
        for (size_t i=0; i<scene->mNumMeshes; i++) {
            const aiMesh* aimesh = scene->mMeshes[i];
            if (!aimesh->HasBones()) { continue; }

            for (auto vi =0; vi < aimesh->mNumVertices; vi++) {
                const aiVector3D& pos = aimesh->mVertices[vi];
                skeleton.vertices_.emplace_back(pos.x, pos.y, pos.z);
            }

            for (auto fi = 0; fi < aimesh->mNumFaces; fi++) {
                const aiFace& face = aimesh->mFaces[fi];
                skeleton.faces_.emplace_back(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
            }

            std::vector<glm::vec3> vertices(aimesh->mNumVertices);
            std::vector<BoneIDs> bone_ids(aimesh->mNumVertices, BoneIDs{});
            std::vector<BoneWeights> bone_weights(aimesh->mNumVertices, BoneWeights{});



            for (size_t b = 0; b < aimesh->mNumBones; b++) { // populate bone data
                const aiBone* bone = aimesh->mBones[b];
                auto bone_id = skeleton.bone_map_[bone->mName.C_Str()];



                for (size_t w = 0; w < bone->mNumWeights; w++) {
                    const aiVertexWeight& weight = bone->mWeights[w];
                    const size_t vertex_id = weight.mVertexId;

                    auto& s_bone = skeleton.bones_[bone_id];
                    s_bone.vertex_weights[vertex_id] = weight.mWeight;
                    skeleton.vertex_to_boneID_map_[vertex_id].push_back(bone_id);


                    for (size_t j = 0; j < MAX_BONES_PER_VERTEX; j++) {
                        if (bone_weights[vertex_id][j] == 0.0f) {
                            bone_ids[vertex_id][j] = bone_id;
                            bone_weights[vertex_id][j] = weight.mWeight;
                            break;
                        }
                    }
                }
            }

            // Normalize bone weights, default to 1.0 for root bone if no weights
            for (auto& b_w : bone_weights) {
                if (b_w == BoneWeights{}) {
                    b_w[0] = 1.0f;
                } else {
                    float sum = 0;
                    for (auto w : b_w) sum += w;
                    if (sum != 1.0f) for (auto& w : b_w) w /= sum; // normalize
                }
            }

            // skeleton.bones_[0].vertex_weights

            // Separate vectors for each attribute type
            std::vector<glm::vec3> positions;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec2> texcoords;
            std::vector<BoneIDs> face_bone_ids;
            std::vector<BoneWeights> face_bone_weights;

            positions.reserve(aimesh->mNumFaces * 3);
            normals.reserve(aimesh->mNumFaces * 3);
            texcoords.reserve(aimesh->mNumFaces * 3);
            face_bone_ids.reserve(aimesh->mNumFaces * 3);
            face_bone_weights.reserve(aimesh->mNumFaces * 3);

            for (int f = 0; f < aimesh->mNumFaces; f++) {
                const aiFace& face = aimesh->mFaces[f];
                for (int v = 0; v < face.mNumIndices; v++) {
                    auto index = face.mIndices[v];
                    const aiVector3D& pos = aimesh->mVertices[index];
                    const aiVector3D& normal = aimesh->mNormals[index];
                    const aiVector3D& texcoord = aimesh->HasTextureCoords(0) ? aimesh->mTextureCoords[0][index] : aiVector3D(0.0f, 0.0f, 0.0f);

                    vertices[index] = glm::vec3(pos.x, pos.y, pos.z);

                    positions.push_back(glm::vec3(pos.x, pos.y, pos.z));
                    normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
                    texcoords.push_back(glm::vec2(texcoord.x, texcoord.y));
                    face_bone_ids.push_back(bone_ids[index]);
                    face_bone_weights.push_back(bone_weights[index]);
                }
            }

            auto material_name = scene->mMaterials[aimesh->mMaterialIndex]->GetName().C_Str();
            DrawObject object;
            object.shape = loadSkinnedShape(positions, normals, texcoords, face_bone_ids, face_bone_weights);
            object.material = materials[material_name];
            mesh.objects.push_back(object);

        }

        auto collision_mesh = decomposeSkeleton(skeleton);

        skeleton.updateBoneMatrices();
        return {mesh, collision_mesh, skeleton};
    }


    void sumChildrenWeights(const Skeleton& skeleton, const std::unordered_map<unsigned int, float>& bone_weights_sum,
        unsigned int current_id, float& total_sum) {
        total_sum += bone_weights_sum.at(current_id);
        for (const auto& child_id : skeleton.bones_.at(current_id).children) {
            sumChildrenWeights(skeleton, bone_weights_sum, child_id, total_sum);
        }
    }


    /* Heuristic stuff
    std::unordered_map<unsigned int, std::vector<glm::vec3>> bone_to_meshes;
        std::unordered_map<unsigned int, float> bone_weights_sum;

        for (const auto& [vertex_id, bone_ids] : skeleton.vertex_to_boneID_map_) {
            for (const auto& bone_id : bone_ids) {
                // initialize if not present
                if (!bone_weights_sum.contains(bone_id)) bone_weights_sum[bone_id] = 0;
                bone_weights_sum[bone_id] += skeleton.bones_.at(bone_id).vertex_weights.at(vertex_id);
            }
        }

        std::unordered_map<unsigned int, float> global_bone_weights; // weight of all child nodes
        for (const auto& bone : skeleton.bones_) {
            global_bone_weights[bone.id] = 0;
            float& sum = global_bone_weights[bone.id];
            sum += bone_weights_sum.at(bone.id);
            sumChildrenWeights(skeleton, bone_weights_sum, bone.id, sum);
        }

        int counter = 0;
        std::vector<std::pair<unsigned int, float>> bone_importance; // bone_id, importance score
        bone_importance.reserve(bone_weights_sum.size());
        for (const auto& [bone_id, weight_sum] : global_bone_weights) {
            bone_importance.emplace_back(bone_id, weight_sum);
        }
        // Sort bones by importance
        std::ranges::sort(bone_importance,
            [](const auto& a, const auto& b) { return a.second > b.second; });

        auto most = bone_importance[0];

     */


    DrawMesh SkeletalMesh::decomposeSkeleton(const Skeleton& skeleton) {


        std::unordered_set<unsigned int> important_bones;
        for (const auto& bone : skeleton.bones_) {
            if (bone.children.size() > 1) {
                important_bones.emplace(bone.id);
            }
        }


        std::unordered_map<unsigned int, std::vector<glm::vec3>> bone_to_meshes;
        for (unsigned int i = 0; i < skeleton.vertices_.size(); i++) {
            const auto& vertex = skeleton.vertices_[i];

            for (const auto bone_id : skeleton.vertex_to_boneID_map_.at(i)) {
                if (important_bones.contains(bone_id)) bone_to_meshes[bone_id].push_back(vertex);
            }
        }

        DrawMesh mesh;
        auto object_color = getRainbow(important_bones.size());
        debug::print(std::to_string(bone_to_meshes.size()));

        int color_idx = 0;
        for (const auto bone : important_bones) {

            quickhull::QuickHull<float> qh;
            std::vector<quickhull::Vector3<float>> points;
            for (const auto& v : bone_to_meshes[bone]) {
                points.emplace_back(v.x, v.y, v.z);
            }
            auto hull = qh.getConvexHull(points, true, false);

            const auto& hullVertices = hull.getVertexBuffer();
            const auto& hullIndices = hull.getIndexBuffer();


            std::vector<glm::vec3> positions;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec2> texcoords;
            std::vector<BoneIDs> bone_ids;
            std::vector<BoneWeights> bone_weights;

            positions.reserve(hullIndices.size());
            normals.reserve(hullIndices.size());
            texcoords.reserve(hullIndices.size());
            bone_ids.reserve(hullIndices.size());
            bone_weights.reserve(hullIndices.size());

            auto toGlmVec3 = [](const quickhull::Vector3<float>& v) {
                return glm::vec3(v.x, v.y, v.z);
            };


            for (int i = 0; i < hullIndices.size(); i += 3) {
                const auto idx0 = hullIndices[i];
                const auto idx1 = hullIndices[i + 1];
                const auto idx2 = hullIndices[i + 2];

                // Get vertices using the indices
                const glm::vec3 v0 = toGlmVec3(hullVertices[idx0]);
                const glm::vec3 v1 = toGlmVec3(hullVertices[idx1]);
                const glm::vec3 v2 = toGlmVec3(hullVertices[idx2]);


                positions.push_back(v0);
                positions.push_back(v1);
                positions.push_back(v2);

                // Calculate normal - cross product of two edges
                glm::vec3 edge1 = v1 - v0;
                glm::vec3 edge2 = v2 - v0;
                glm::vec3 norm = glm::normalize(glm::cross(edge1, edge2));

                // QuickHull should give CCW winding (outward normals)
                // If normals point inward, negate them: norm = -norm;

                normals.push_back(norm);
                normals.push_back(norm);
                normals.push_back(norm);

                texcoords.push_back(glm::vec2(0.0f, 0.0f));
                texcoords.push_back(glm::vec2(0.0f, 0.0f));
                texcoords.push_back(glm::vec2(0.0f, 0.0f));
                bone_ids.push_back(BoneIDs{ bone, 0, 0, 0 });
                bone_ids.push_back(BoneIDs{ bone, 0, 0, 0 });
                bone_ids.push_back(BoneIDs{ bone, 0, 0, 0 });
                bone_weights.push_back(BoneWeights{ 1.0f, 0.0f, 0.0f, 0.0f });
                bone_weights.push_back(BoneWeights{ 1.0f, 0.0f, 0.0f, 0.0f });
                bone_weights.push_back(BoneWeights{ 1.0f, 0.0f, 0.0f, 0.0f });

            }
            DrawShape shape = loadSkinnedShape(positions, normals, texcoords, bone_ids, bone_weights);
            DrawObject object;
            object.shape = shape;
            object.material = getColliderMaterial(object_color[color_idx]);
            color_idx++;
            mesh.objects.push_back(object);
        }
        return mesh;
    }

    DrawShape SkeletalMesh::loadSkinnedShape(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& normals,
        const std::vector<glm::vec2>& texcoords,
        const std::vector<BoneIDs>& bone_ids,
        const std::vector<BoneWeights>& bone_weights) {

        DrawShape shape;
        GLuint vao;
        GLuint vbo_pos, vbo_normal, vbo_texcoord, vbo_bone_ids, vbo_bone_weights;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Position buffer (attribute 0)
        glGenBuffers(1, &vbo_pos);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Normal buffer (attribute 1)
        glGenBuffers(1, &vbo_normal);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Texcoord buffer (attribute 2)
        glGenBuffers(1, &vbo_texcoord);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), texcoords.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Bone IDs buffer (attribute 3) - PROPER INTEGER HANDLING
        glGenBuffers(1, &vbo_bone_ids);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_bone_ids);
        glBufferData(GL_ARRAY_BUFFER, bone_ids.size() * sizeof(BoneIDs), bone_ids.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, 0, nullptr);  // Use GL_UNSIGNED_INT

        // Bone weights buffer (attribute 4)
        glGenBuffers(1, &vbo_bone_weights);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_bone_weights);
        glBufferData(GL_ARRAY_BUFFER, bone_weights.size() * sizeof(BoneWeights), bone_weights.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindVertexArray(0);

        // Calculate bounding box
        glm::vec3 bmin(FLT_MAX);
        glm::vec3 bmax(-FLT_MAX);
        for (const auto& pos : positions) {
            bmin = glm::min(bmin, pos);
            bmax = glm::max(bmax, pos);
        }

        shape.vao = vao;
        shape.vbo = vbo_pos;  // Store primary VBO (we could also store all VBOs if needed)
        shape.numTriangles = positions.size() / 3;
        shape.min = bmin;
        shape.max = bmax;

        return shape;
    }
}