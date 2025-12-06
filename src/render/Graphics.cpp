#include "Graphics.h"

#include <iostream>

#include "Camera.h"
#include "Mesh.h"
#include "Shaders.h"
#include "SkeletalMesh.h"
#include "stb_image.h"
#include "shapes/Cube.h"

namespace gl {

    ShaderProgram* Graphics::active_shader_;
    ShaderProgram Graphics::phong_;
    ShaderProgram Graphics::skinned_;
    std::unordered_map<std::string, DrawShape> Graphics::shapes_;

    void Graphics::initialize() {
        initializePhongShader();
        setAmbientLight(glm::vec3(0.5));
        loadShapes();
    }

    void Graphics::tearDown() {
        phong_.deleteProgram();
    }

    void applyGLSettings() {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glEnable(GL_POLYGON_SMOOTH);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonOffset(1.0, 1.0);
    }

    void Graphics::usePhongShader() {
        phong_.use();
        active_shader_ = &phong_;
        applyGLSettings();
    }

    void Graphics::useSkinnedShader() {
        skinned_.use();
        active_shader_ = &skinned_;
        applyGLSettings();
    }

    void Graphics::drawObject(const DrawShape* drawShape, const Transform& transform, const DrawMaterial& material) {
        const auto model_matrix = transform.getModelMatrix();
        phong_.setMat4("model", model_matrix);
        phong_.setMat3("normal", glm::transpose(glm::inverse(glm::mat3(model_matrix))));

        setMaterialUniforms(material);

        glBindVertexArray(drawShape->vao);
        glDrawArrays(GL_TRIANGLES, 0, 3 * drawShape->numTriangles);
        glBindVertexArray(0);
    }

    void Graphics::drawMesh(const DrawMesh* draw_mesh, const Transform& transform) {
        const auto model_matrix = transform.getModelMatrix();
        phong_.setMat4("model", model_matrix);
        phong_.setMat3("normal", glm::transpose(glm::inverse(glm::mat3(model_matrix))));

        for (const auto& obj : draw_mesh->objects) {
            setMaterialUniforms(obj.material);
            glBindVertexArray(obj.shape.vao);
            glDrawArrays(GL_TRIANGLES, 0, 3 * obj.shape.numTriangles);
            glBindVertexArray(0);
        }

    }

    void Graphics::drawSkinned(const DrawMesh* draw_mesh, Skeleton& skeleton, const Transform& transform) {
        const auto model_matrix = transform.getModelMatrix();
        active_shader_->setMat4("model", model_matrix);
        active_shader_->setMat3("normal", glm::transpose(glm::inverse(glm::mat3(model_matrix))));

        skeleton.updateBoneMatrices();
        active_shader_->setMat4Vec("gBones", skeleton.num_bones_, skeleton.bone_matrices_);

        for (const auto& [shape, material] : draw_mesh->objects) {
            setMaterialUniforms(material);
            glBindVertexArray(shape.vao);
            glDrawArrays(GL_TRIANGLES, 0, 3 * shape.numTriangles);
            glBindVertexArray(0);
        }
    }

    void Graphics::drawSkinned(SkinnedMesh* skinned_mesh, const Transform& transform) {
        const auto model_matrix = transform.getModelMatrix();
        active_shader_->setMat4("model", model_matrix);
        active_shader_->setMat3("normal", glm::transpose(glm::inverse(glm::mat3(model_matrix))));


        const auto& draw_mesh = skinned_mesh->draw_mesh;

        auto& skeleton = skinned_mesh->skeleton;

        skeleton.updateBoneMatrices();
        active_shader_->setMat4Vec("gBones", skeleton.num_bones_, skeleton.bone_matrices_);

        for (const auto& obj : draw_mesh.objects) {
            setMaterialUniforms(obj.material);
            glBindVertexArray(obj.shape.vao);
            glDrawArrays(GL_TRIANGLES, 0, 3 * obj.shape.numTriangles);
            glBindVertexArray(0);
        }
    }

    const DrawShape* Graphics::getShape(const std::string& shape_name) {
        return &shapes_[shape_name];
    }


    void Graphics::setCameraUniforms(const Camera* camera) {
        active_shader_->setMat4("view", camera->getViewMatrix());
        active_shader_->setMat4("projection", camera->getProjection());
        active_shader_->setVec3("camera_pos", camera->getPosition());
    }

    void Graphics::setLight(const Light& light) {
        active_shader_->setVec3("light_position", light.position);
        active_shader_->setVec3("light_color", light.color);
    }

    void Graphics::setAmbientLight(const glm::vec3& ambient) {
        phong_.use();
        phong_.setVec3("ambient_light", ambient);
        skinned_.use();
        skinned_.setVec3("ambient_light", ambient);
        active_shader_->use();
    }

    void Graphics::initializePhongShader() {
        const auto frag = "Resources/Shaders/phong_frag.glsl";
        const auto phong_vert = "Resources/Shaders/phong_vert.glsl";
        phong_ = Shaders::createShaderProgram(phong_vert,frag);//ShaderProgram(vert, frag);

        const auto skinned_vert = "Resources/Shaders/skinned_vert.glsl";
        skinned_ = Shaders::createShaderProgram(skinned_vert,phong_.getFragmentID());

        active_shader_= &phong_;
    }

    void Graphics::loadShapes() {
        // auto cone = Cone();
        // auto sphere = Sphere();
        auto cube = Cube();
        // auto cylinder = Cylinder();
        // auto quad = Quad();
        const int quality = 8;

        // shapes["quad"] = Mesh::loadShape(quad.updateParams(1,1));
        // shapes["cone"] = Mesh::loadShape(cone.updateParams(1,quality));
        // shapes["sphere"] = Mesh::loadShape(sphere.updateParams(quality,quality));
        shapes_["cube"] = Mesh::loadStaticShape(cube.updateParams(1,1));
        // shapes["cylinder"] = Mesh::loadShape(cylinder.updateParams(1,quality));

    }

    void Graphics::setMaterialUniforms(const DrawMaterial& material) {
        active_shader_->setVec3("ambient", material.ambient);
        active_shader_->setVec3("diffuse", material.diffuse);
        active_shader_->setVec3("specular", material.specular);
        active_shader_->setFloat("shininess", material.shininess);
        active_shader_->setFloat("opacity", material.opacity);
        bindMaterialTextures(material.textures);
    }

    void Graphics::bindMaterialTextures(const Textures& textures) {
        active_shader_->setInt("texture_flags", textures.flags);

        if (textures.ambient != 0) {
            bindTexture(textures.ambient, 0, "texture_ambient");
        }
        if (textures.diffuse != 0) {
            bindTexture(textures.diffuse, 1, "texture_diffuse");
        }
        if (textures.specular != 0) {
            bindTexture(textures.specular, 2, "texture_specular");
        }

    }

    void Graphics::bindTexture(const GLuint texture, const int unit, const char* uniform_name) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture);
        active_shader_->setInt(uniform_name, unit); // sampler2D bound by binding the texture unit to the uniform
    }
}


