#include "Core.h"

#include <iostream>


#include "Debug.h"
#include "imgui.h"
#include "Window.h"

#include "render/Camera.h"
#include "render/Mesh.h"
#include "render/SkeletalMesh.h"


static gl::SkinnedMesh skinned_mesh;
static gl::Transform skinned_transform;



Core::Core() : camera_(std::make_shared<gl::Camera>()), light_(std::make_shared<gl::Light>()) {
    light_->position = glm::vec3(0, 5, 0);

    // obj_mesh = gl::Mesh::loadStaticMesh(obj_path);
    // coacd_mesh = gl::Mesh::decomposeObj(obj_path, 0.9);
    // obj_transform.setScale(glm::vec3(0.01f));

    skinned_mesh = gl::SkeletalMesh::loadFbx("Resources/Models/walking.fbx");
    skinned_mesh.skeleton.setCurrentAnimation("Take 001");
    skinned_transform.setScale(glm::vec3(0.01));

    // obj_mesh = gl::Mesh::loadStaticMesh("Resources/Models/sponza/sponza.obj");
    // obj_transform.setScale(glm::vec3(0.01));
}

float getScale(const gl::DrawMesh* mesh) {
    glm::vec3 size = mesh->max - mesh->min;
    float max_extent = std::max({size.x, size.y, size.z});
    return 10.0f / max_extent;
}

void Core::drawGUI() {
    UI::beginDraw(0,0, 300, 200);
    ImGui::Begin("GUI");
    ImGui::Text("Use WASD + Space/Shift to move camera");
    ImGui::Text("Press Enter to lock cursor");
    ImGui::Text("Press Escape to unlock cursor");

    if (ImGui::Button("Upload File")) {
        auto path = UI::openFileExplorer({{"obj file","obj"}});
        if (!path.empty()) {
            debug::print("Selected file: " + path);
            info_.object_path = path;

            draw_object_.reset();
            draw_object_ = std::make_unique<gl::DrawMesh>(gl::Mesh::loadStaticMesh(path.c_str()));
            transform_.setScale(getScale(draw_object_.get()));
            collider_.reset();



            // Load the selected OBJ file

        }
    }

    if (ImGui::Button("Decompose Mesh")) {
        collider_.reset();
        collider_ = std::make_unique<gl::DrawMesh>(gl::Mesh::decomposeObj(info_.object_path.c_str(), params_.quality));
        render_options_.show_collider = true;
    }
    ImGui::SliderFloat("Quality", &params_.quality, 0.f, 1.0f);
    ImGui::End();
    UI::endDraw();
}

void Core::drawCurrentObject() {
    if (draw_object_) {
        glPolygonMode(GL_FRONT_AND_BACK, render_options_.mesh_polygon_mode);
        gl::Graphics::drawMesh(draw_object_.get(), transform_);

        if (collider_ && render_options_.show_collider) {
            glPolygonMode(GL_FRONT_AND_BACK, render_options_.collider_polygon_mode);
            gl::Graphics::drawMesh(collider_.get(), transform_);
        }
    }
}

void Core::draw() {
    gl::Graphics::usePhongShader();
    gl::Graphics::setCameraUniforms(camera_.get());
    gl::Graphics::setLight(*light_);

    drawCurrentObject();

    gl::Graphics::useSkinnedShader();
    gl::Graphics::setCameraUniforms(camera_.get());
    gl::Graphics::setLight(*light_);
    skinned_mesh.skeleton.playCurrentAnimation(Window::getCurrentTime());
    gl::Graphics::drawSkinned(skinned_mesh.draw_mesh, skinned_mesh.skeleton, skinned_transform);
    glPolygonMode(GL_FRONT, GL_FILL);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);  // Disable depth writes
    glEnable(GL_BLEND);     // Already enabled in applyGLSettings, but making it explicit
    gl::Graphics::drawSkinned(skinned_mesh.collision_mesh, skinned_mesh.skeleton, skinned_transform);
    glDepthMask(GL_TRUE);
    drawGUI();
}

static glm::vec2 rotation(0.0f, 0.0f);
static auto last_mouse_pos = Window::getMousePosition();
void Core::update(double delta_time) {
    keyInputHandler(delta_time);




    if (!Window::isCursorVisible()) {
        auto mouse_pos = Window::getMousePosition();
        auto d_mouse = 0.1f*(last_mouse_pos - mouse_pos);
        last_mouse_pos = mouse_pos;
        rotation.x += d_mouse.y;
        rotation.y += d_mouse.x;
        rotation.x = glm::clamp(rotation.x, -89.0f, 89.0f);


        glm::vec3 newFront;
        newFront.x = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
        newFront.y = sin(glm::radians(rotation.x));
        newFront.z = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
        auto newLook = glm::normalize(newFront);

        camera_->setLook(newLook);
    }


}

void Core::keyInputHandler(double delta_time) {
    float mod = 4.f*delta_time;

    auto camXZ = [](glm::vec3 look) {
        auto cam_xz = look * glm::vec3(1,0,1);
        cam_xz = glm::normalize(cam_xz);
        return cam_xz;
    };
    if (Window::key(GLFW_KEY_W)) {
        camera_->setPosition(camera_->getPosition() + mod * camXZ(camera_->getLook()));
    }
    if (Window::key(GLFW_KEY_S)) {
        camera_->setPosition(camera_->getPosition() - mod * camXZ(camera_->getLook()));
    }
    if (Window::key(GLFW_KEY_A)) {
        camera_->setPosition(camera_->getPosition() - mod * camXZ(camera_ -> getRight()));
    }
    if (Window::key(GLFW_KEY_D)) {
        camera_->setPosition(camera_->getPosition() + mod * camXZ(camera_ -> getRight()));
    }
    if (Window::key(GLFW_KEY_SPACE)) {
        camera_->setPosition(camera_->getPosition() + mod * camera_->getUp());
    }
    if (Window::key(GLFW_KEY_LEFT_SHIFT)) {
        camera_->setPosition(camera_->getPosition() - mod * camera_ -> getUp());
    }

    if (Window::key(GLFW_KEY_ENTER)) {
        Window::hideMouse();
        last_mouse_pos = Window::getMousePosition();

    }
    if (Window::key(GLFW_KEY_ESCAPE)) {
        Window::showMouse();
    }
}

void Core::loadObject(const std::string& name) {

}
