#pragma once
#include <string>

#include "render/Graphics.h"

namespace gl {
    class Camera;
    struct Light;
    struct DrawShape;
}


struct ObjectInfo {
    std::string object_path;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct DecompParameters {
    float quality = 0.5f;
};

struct RenderOptions {
    GLenum mesh_polygon_mode = GL_FILL;
    GLenum collider_polygon_mode = GL_FILL;

    bool show_mesh = true;
    bool show_collider = false;
};

struct Object {
    explicit Object(const std::string& name) {
        shape = gl::Graphics::getShape(name);
        transform = gl::Transform();
        material = gl::defaultMaterial;
    }
    const gl::DrawShape* shape;
    gl::Transform transform;
    gl::DrawMaterial material;
};

class Core {
public:
    Core();
    ~Core() = default;
    void draw();

    void update(double delta_time);

    void keyInputHandler(double delta_time);
private:

    void drawGUI();
    void loadObject(const std::string& name);
    void drawCurrentObject();
    std::shared_ptr<gl::Camera> camera_;
    std::shared_ptr<gl::Light> light_;

    ObjectInfo info_;
    RenderOptions render_options_;

    std::unique_ptr<gl::DrawMesh> draw_object_;
    std::unique_ptr<gl::DrawMesh> collider_;
    gl::Transform transform_;

    DecompParameters params_;

};
