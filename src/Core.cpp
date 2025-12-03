#include "Core.h"

#include <iostream>


#include "Debug.h"
#include "imgui.h"
#include "Window.h"

#include "render/Camera.h"
#include "render/Mesh.h"
#include "render/SkeletalMesh.h"


// static gl::SkinnedMesh skinned_mesh;
// static gl::Transform skinned_transform;

static gl::DrawMesh obj_mesh;
static gl::Transform obj_transform;
static gl::DrawMesh coacd_mesh;

Core::Core() : m_camera(std::make_shared<gl::Camera>()), m_light(std::make_shared<gl::Light>()) {

    m_light->position = glm::vec3(0, 5, 0);

    auto obj_path = "Resources/Models/Spider/spider.obj";

    obj_mesh = gl::Mesh::loadStaticMesh(obj_path);
    coacd_mesh = gl::Mesh::decomposeObj(obj_path, 0.9);
    obj_transform.setScale(glm::vec3(0.01f));

    // skinned_mesh = gl::SkeletalMesh::loadFbx("Resources/Models/walking.fbx");
    // skinned_mesh.skeleton.setCurrentAnimation("Take 001");
    // skinned_transform.rotateDegrees(30, glm::vec3(0,1,0));
    // skinned_transform.setScale(glm::vec3(0.01));

    // obj_mesh = gl::Mesh::loadStaticMesh("Resources/Models/sponza/sponza.obj");
    // obj_transform.setScale(glm::vec3(0.01));
}

void drawGUI() {
    UI::beginDraw(0,0, 300, 200);
    ImGui::Begin("GUI");
    ImGui::Text("Use WASD + Space/Shift to move camera");
    ImGui::Text("Press Enter to lock cursor");
    ImGui::Text("Press Escape to unlock cursor");
    ImGui::Text("Press O to open file explorer");


    if (ImGui::Button("Upload File")) {
        auto path = UI::openFileExplorer({{"obj file","obj"}});
        if (!path.empty()) {
            debug::print("Selected file: " + path);
            // Load the selected OBJ file
            obj_mesh = gl::Mesh::loadStaticMesh(path.c_str());
            obj_transform.setScale(glm::vec3(0.1f));
        }
    }
    ImGui::End();


    UI::endDraw();
}

void Core::draw() const {
    gl::Graphics::usePhongShader();
    gl::Graphics::setCameraUniforms(m_camera.get());
    gl::Graphics::setLight(*m_light);

    gl::Graphics::drawMesh(&obj_mesh, obj_transform);
    gl::Graphics::drawMesh(&coacd_mesh, obj_transform);

    gl::Graphics::setCameraUniforms(m_camera.get());
    gl::Graphics::setLight(*m_light);
    // gl::Graphics::useSkinnedShader();
    // skinned_mesh.skeleton.playCurrentAnimation(Window::getCurrentTime());
    // gl::Graphics::drawSkinned(&skinned_mesh, skinned_transform);
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

        m_camera->setLook(newLook);
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
        m_camera->setPosition(m_camera->getPosition() + mod * camXZ(m_camera->getLook()));
    }
    if (Window::key(GLFW_KEY_S)) {
        m_camera->setPosition(m_camera->getPosition() - mod * camXZ(m_camera->getLook()));
    }
    if (Window::key(GLFW_KEY_A)) {
        m_camera->setPosition(m_camera->getPosition() - mod * camXZ(m_camera -> getRight()));
    }
    if (Window::key(GLFW_KEY_D)) {
        m_camera->setPosition(m_camera->getPosition() + mod * camXZ(m_camera -> getRight()));
    }
    if (Window::key(GLFW_KEY_SPACE)) {
        m_camera->setPosition(m_camera->getPosition() + mod * m_camera->getUp());
    }
    if (Window::key(GLFW_KEY_LEFT_SHIFT)) {
        m_camera->setPosition(m_camera->getPosition() - mod * m_camera -> getUp());
    }

    if (Window::key(GLFW_KEY_ENTER)) {
        Window::hideMouse();
        last_mouse_pos = Window::getMousePosition();

    }
    if (Window::key(GLFW_KEY_ESCAPE)) {
        Window::showMouse();
    }
}
