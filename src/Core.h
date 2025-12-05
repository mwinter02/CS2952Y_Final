#pragma once
#include <string>

#include "render/Graphics.h"
#include "render/SkeletalMesh.h"

namespace gl {
    class Camera;
    struct Light;
    struct DrawShape;
}


struct ObjectInfo {
    std::string object_path;
};

struct DecompParameters {
    float quality = 0.5f;
};

struct RenderOptions {

    bool mesh_wireframe = false;
    bool collider_wireframe = false;

    bool is_skeletal = false;
    bool show_mesh = true;
    bool show_collider = false;
    bool play_animation = true;
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
    void guiStatic();
    void setAnimation(int index);
    void guiSkeletal();
    void guiRenderOptions();
    void drawGUI();
    void loadObject(const std::string& name);
    void drawCurrentObject();
    std::shared_ptr<gl::Camera> camera_;
    std::shared_ptr<gl::Light> light_;

    ObjectInfo info_;
    RenderOptions render_options_;

    std::unique_ptr<gl::DrawMesh> static_mesh_;
    std::unique_ptr<gl::SkinnedMesh> skinned_mesh_;
    std::unique_ptr<gl::DrawMesh> collider_;
    gl::Transform transform_;

    DecompParameters params_;

};
