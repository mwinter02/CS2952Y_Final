#pragma once

#include "Texture.h"
#include "Transform.h"


namespace gl {
    struct Skeleton;
    struct SkinnedMesh;
    class Camera;
    class ShaderProgram;



    struct DrawShape {
        GLuint vao = 0;
        GLuint vbo = 0; // vertex buffer id
        size_t numTriangles = 0;
        glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
    };


    struct DrawObject {
        DrawShape shape;
        DrawMaterial material = defaultMaterial;
    };

    struct DrawMesh {
        std::vector<DrawObject> objects;
        glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
    };

    struct Light {
        glm::vec3 position = glm::vec3(0,0,0);
        glm::vec3 color = glm::vec3(1.f,1.f,1.f);
    };


    class Graphics {
    public:
        static void initialize();
        static void tearDown();

        static void usePhongShader();
        static void useSkinnedShader();
        static void setCameraUniforms(const Camera* camera);
        static void setLight(const Light& light);
        static void setAmbientLight(const glm::vec3& ambient);


        static void drawObject(const DrawShape* drawShape, const Transform& transform, const DrawMaterial& material = defaultMaterial);
        static void drawMesh(const DrawMesh* draw_mesh, const Transform& transform);
        static void drawSkinned(const DrawMesh* draw_mesh, Skeleton& skeleton, const Transform& transform);
        static void drawSkinned(SkinnedMesh* skinned_mesh, const Transform& transform);

        static const DrawShape* getShape(const std::string& shape_name);


    private:
        static void initializePhongShader();
        static void loadShapes();
        static void setMaterialUniforms(const DrawMaterial& material);
        static void bindMaterialTextures(const Textures& textures);
        static void bindTexture(GLuint texture, int unit, const char* uniform_name);


        static std::unordered_map<std::string, DrawShape> shapes_;

        static ShaderProgram* active_shader_;
        static ShaderProgram phong_;
        static ShaderProgram skinned_;

    };
}

