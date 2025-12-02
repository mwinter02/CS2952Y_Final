#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 aBoneIDs;
layout(location = 4) in vec4 aWeights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal;

const int MAX_BONES = 200;
uniform mat4 gBones[MAX_BONES];

void main() {
    // Compute bone transformation matrix by blending up to 4 bones
    mat4 BoneTransform = gBones[aBoneIDs[0]] * aWeights[0];
    BoneTransform     += gBones[aBoneIDs[1]] * aWeights[1];
    BoneTransform     += gBones[aBoneIDs[2]] * aWeights[2];
    BoneTransform     += gBones[aBoneIDs[3]] * aWeights[3];

//    BoneTransform = mat4(1.0); // TEMPORARY DISABLE SKINNING

    // Transform position by bone matrix, then by model matrix
    vec4 skinnedPosition = BoneTransform * vec4(aPosition, 1.0);
    FragPos = vec3(model * skinnedPosition);

    // Transform normal by bone matrix (use mat3 to ignore translation), then by model normal matrix
    vec3 skinnedNormal = mat3(BoneTransform) * aNormal;
    Normal = normal * skinnedNormal;

    TexCoord = aTexCoord;

    gl_Position = projection * view * vec4(FragPos, 1.0);

}