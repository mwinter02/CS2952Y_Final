#pragma once
#include <vector>
#include <string>

#include "Graphics.h"
#include "glm/glm.hpp"

namespace gl {

#define MAX_BONES_PER_VERTEX 4
    using BoneIDs = std::array<unsigned int, MAX_BONES_PER_VERTEX>;
    using BoneWeights = std::array<float, MAX_BONES_PER_VERTEX>;

    struct Bone {
        std::string name;              // Bone name from FBX file
        unsigned int id;                        // Unique bone index
        int parent_id;                 // Parent bone index (-1 for root)
        glm::mat4 offset_matrix;       // Inverse bind pose - transforms from mesh space to bone space
        glm::mat4 local_transform;   // Local transform relative to parent bone
        const glm::mat4 bind_pose_transform; // Bind pose local transform
        std::vector<unsigned int> children;     // Indices of child bones

        Bone(const std::string& bone_name, const unsigned int bone_id, const int parent,
            const glm::mat4& offset, const glm::mat4& bind_pose_transform) :
        name(bone_name),
        id(bone_id),
        parent_id(parent),
        offset_matrix(offset),
        local_transform(bind_pose_transform),
        bind_pose_transform(bind_pose_transform) {}

        void addChild(const Bone& child) {
            children.push_back(child.id);
        }
    };

    template<typename T>
    struct Keyframe {
        double time;        // Time in ticks
        T value;           // Position (vec3), Rotation (quat), or Scale (vec3)
    };

    using PositionKey = Keyframe<glm::vec3>;
    using RotationKey = Keyframe<glm::quat>;
    using ScaleKey = Keyframe<glm::vec3>;

    // Animation channel - one per animated bone
    struct AnimationChannel {
        std::string bone_name;
        unsigned int bone_id;                    // Which bone this channel affects
        std::vector<PositionKey> position_keys;  // Translation keyframes
        std::vector<RotationKey> rotation_keys;  // Rotation keyframes
        std::vector<ScaleKey> scale_keys;        // Scale keyframes

        glm::mat4 calculateTransform(double animation_time) const;
    };

    struct Animation {
        double duration; // in ticks
        double ticks_per_second;

        // Map from bone_id to its animation channel
        std::unordered_map<unsigned int, AnimationChannel> channels;

        // Optional: playback control
        bool looping = true;
        double playback_speed = 1.0; // scalar


    };

    struct Skeleton {
        public:
        void addBone(const std::string& bone_name, const unsigned int current_id, const int parent_id,
            const glm::mat4& offset_matrix, const glm::mat4& local_transform);
        void updateBoneMatrices();


        void setCurrentAnimation(const std::string& animation_name);
        void playCurrentAnimation(double current_time);
        void resetToBindPose();

        void traverseBoneHierarchy(const unsigned int bone_id, const glm::mat4& parent_transform);
        unsigned int num_bones_;
        std::vector<Bone> bones_;
        std::unordered_map<std::string, unsigned int> bone_map_;
        std::vector<glm::mat4> bone_matrices_;

        std::unordered_map<std::string, Animation> animations_;
        Animation* current_animation_ = nullptr;
    };

    struct SkinnedMesh {
        DrawMesh draw_mesh;
        Skeleton skeleton;
    };







    class SkeletalMesh {
    public:


        static SkinnedMesh loadFbx(const char* filename);




    private:

        static DrawShape loadSkinnedShape(
            const std::vector<glm::vec3>& positions,
            const std::vector<glm::vec3>& normals,
            const std::vector<glm::vec2>& texcoords,
            const std::vector<BoneIDs>& bone_ids,
            const std::vector<BoneWeights>& bone_weights);


        std::vector<glm::vec3> vertex_data_;
        std::vector<glm::ivec4> bone_ids_; // Bone IDs for each vertex
        std::vector<glm::vec4> bone_weights_; // Bone weights for each vertex



    };
}
