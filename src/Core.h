#pragma once
#include <string>

#include "render/Graphics.h"

namespace gl {
    class Camera;
    struct Light;
    struct DrawShape;
}

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
    void draw() const;

    void update(double delta_time);

    void keyInputHandler(double delta_time);
private:
    std::shared_ptr<gl::Camera> m_camera;
    std::shared_ptr<gl::Light> m_light;
    std::vector<Object> m_shapes;

};
