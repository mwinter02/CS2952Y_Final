#include "Core.h"

#include <iostream>


#include "Debug.h"
#include "imgui.h"
#include "Window.h"
#include "../External/assimp/code/AssetLib/FBX/FBXExporter.h"

#include "render/Camera.h"
#include "render/Mesh.h"
#include "render/SkeletalMesh.h"


constexpr glm::vec3 initial_camera_position = glm::vec3(0.f,  15.f, 10.f);
constexpr glm::vec2 initial_camera_rotation = glm::vec2(-45.f, 180.0f);



Core::Core() : camera_(std::make_shared<gl::Camera>()), light_(std::make_shared<gl::Light>())
// , skinned_mesh_(std::make_unique<gl::SkinnedMesh>(gl::SkeletalMesh::loadFbx("Resources/Models/walking.fbx")))
 {
    light_->position = glm::vec3(0, 20, 5);
    camera_->setPosition(initial_camera_position);

    Window::setClearColor(glm::vec3(.5f));

    gl::SkeletalMesh::loadFbx("Resources/Models/Dragon/Dragon 2.5_fbx.fbx");


    // obj_mesh = gl::Mesh::loadStaticMesh("Resources/Models/sponza/sponza.obj");
    // obj_transform.setScale(glm::vec3(0.01));

}


static float s_scale;
glm::vec2 s_scale_bounds;
static glm::vec3 s_position;

static glm::vec3 s_rotation = glm::vec3(0,0,0); // x y z rotation in degrees



static glm::vec2 s_camera_rotation(-45.f, 180.0f);
void Core::resetCamera() {
    s_camera_rotation = initial_camera_rotation;
    camera_->setPosition(initial_camera_position);
}

glm::mat4 getRotation() {
    auto rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, glm::radians(s_rotation.x), {1,0,0});
    rotation = glm::rotate(rotation, glm::radians(s_rotation.y), {0,1,0});
    rotation = glm::rotate(rotation, glm::radians(s_rotation.z), {0,0,1});
    return rotation;
}

void updateScale(gl::DrawMesh* mesh) {
    glm::vec3 size = mesh->max - mesh->min;
    float max_extent = std::max({size.x, size.y, size.z});
    auto scale = 10.0f / max_extent;
    s_scale = scale;
    s_scale_bounds = glm::vec2(scale * 0.1f, scale * 10.0f);
}

void Core::guiTransform() {
    if (skinned_mesh_ || static_mesh_) {
        ImGui::SliderFloat3("Position", glm::value_ptr(s_position), -20, 20);
        ImGui::SameLine(); if (ImGui::Button("Reset##Position")) s_position = {0,0,0};

        ImGui::SliderFloat("Scale   ", &s_scale, s_scale_bounds.x, s_scale_bounds.y);
        ImGui::SameLine(); if (ImGui::Button("Reset##Scale")) s_scale = (s_scale_bounds.x + s_scale_bounds.y) / 10.f;

        ImGui::SliderFloat3("Rotation", glm::value_ptr(s_rotation), 0, 360);
        ImGui::SameLine(); if (ImGui::Button("Reset##Rotation")) s_rotation = {0,0,0};
    } else {
        ImGui::Text("Upload a mesh to see transform options.");
    }

}

void Core::loadNewMesh(const std::string& path) {
    info_.object_path = path;
    skinned_mesh_.reset();
    static_mesh_.reset();
    collider_.reset();
}

void Core::updateTransform() {
    transform_.setScale(s_scale);
    transform_.setPosition(s_position);
    transform_.setRotation(getRotation());
}


void Core::guiStatic() {
    if (ImGui::Button("Upload OBJ File")) {
        auto path = UI::openFileExplorer({{"obj file","obj"}});
        if (!path.empty()) {
            loadNewMesh(path);
            static_mesh_ = std::make_unique<gl::DrawMesh>(gl::Mesh::loadStaticMesh(path.c_str()));
            updateScale(static_mesh_.get());
            render_options_.is_skeletal = false;
            // Load the selected OBJ file

        }
    }

    if (!static_mesh_) {
        ImGui::Text("Upload Static Mesh (.obj) to see more options.");
        return;
    }

    ImGui::SeparatorText("Decomposition settings");
    if (static_mesh_) {
        ImGui::SliderFloat("Quality", &params_.quality, 0.f, 1.0f);
        if (ImGui::Button("Decompose Mesh")) {
            collider_.reset();
            collider_ = std::make_unique<gl::DrawMesh>(gl::Mesh::decomposeObj(info_.object_path.c_str(), params_.quality));
            render_options_.show_collider = true;
        }
    }
}

static int current_animation = 0;

void Core::setAnimation(int index) {
    if (skinned_mesh_ && index >=0 && index < skinned_mesh_->skeleton.animation_list_.size()) {
        if (skinned_mesh_->skeleton.current_animation_ != &skinned_mesh_->skeleton.animations_[skinned_mesh_->skeleton.animation_list_[index]]) {
            skinned_mesh_->skeleton.setCurrentAnimation(skinned_mesh_->skeleton.animation_list_[index]);
        }
    }
    if (index == -1 && skinned_mesh_) {
        skinned_mesh_->skeleton.current_animation_ = nullptr;
        skinned_mesh_->skeleton.resetToBindPose();
    }
}

int decomp_mode = gl::BoneDecompositionMode::MULTI_CHILDREN;
void Core::guiSkeletal() {
    if (ImGui::Button("Upload FBX File")) {
        auto path = UI::openFileExplorer({{"fbx file","fbx"}});
        if (!path.empty()) {
            loadNewMesh(path);
            skinned_mesh_ = std::make_unique<gl::SkinnedMesh>(gl::SkeletalMesh::loadFbx(path.c_str()));
            updateScale(&skinned_mesh_->draw_mesh);
            render_options_.is_skeletal = true;
        }
    }

    if (!skinned_mesh_) {
        ImGui::Text("Upload Skeletal Mesh (.fbx) to see more options.");
        return;
    }

    ImGui::Text("Decomposition mode");
    ImGui::RadioButton("Multi Children", &decomp_mode, gl::BoneDecompositionMode::MULTI_CHILDREN);
    ImGui::RadioButton("All Bones", &decomp_mode, gl::BoneDecompositionMode::ALL_BONES);
    ImGui::RadioButton("Custom Bones", &decomp_mode, gl::BoneDecompositionMode::CUSTOM_BONES);


    if (ImGui::Button("Decompose")) {
        collider_.reset();
        collider_ = std::make_unique<gl::DrawMesh>(gl::SkeletalMesh::decomposeSkeleton(
            skinned_mesh_->skeleton, info_.object_path.c_str(),
            (gl::BoneDecompositionMode) decomp_mode,
            {}));
        render_options_.show_collider = true;
    }

    ImGui::SeparatorText("Animations");
    ImGui::Text("Select Animation:");
    if (skinned_mesh_) {
        ImGui::RadioButton("Off", &current_animation, -1);
        for (int i=0; i<skinned_mesh_->skeleton.animation_list_.size(); i++) {
            const auto& anim_name = skinned_mesh_->skeleton.animation_list_[i];
            ImGui::RadioButton(anim_name.c_str(), &current_animation, i);
        }
        setAnimation(current_animation);
    }
    ImGui::Checkbox("Play animation", &render_options_.play_animation);
}

void Core::guiRenderOptions() {
    if (!skinned_mesh_ && !static_mesh_) {
        ImGui::Text("Upload a mesh to see render options."); return;
    }

    ImGui::Checkbox("Show Mesh      ", &render_options_.show_mesh); ImGui::SameLine();
    ImGui::Checkbox("Wireframe Mesh", &render_options_.mesh_wireframe);
    if (!collider_) {
        ImGui::Text("Decompose mesh to see collider options"); return;
    }
    ImGui::Checkbox("Show Collider  ", &render_options_.show_collider); ImGui::SameLine();
    ImGui::Checkbox("Wireframe Collider", &render_options_.collider_wireframe);

}

void Core::guiCameraControls() {
    if (ImGui::Button("Reset Camera")) {
        resetCamera();
    }
}

void Core::drawGUI() {
    UI::beginDraw(0,0, 400, 600);
    ImGui::Begin("Mesh Decomposer");
    ImGui::Text("Use WASD + Space/Shift to move camera");
    ImGui::Text("Press Enter to lock cursor");
    ImGui::Text("Press Escape to unlock cursor");

    ImGui::SeparatorText("Render Options");
    guiRenderOptions();
    ImGui::SeparatorText("Choose mesh type:");
    ImGui::Spacing();
    if (ImGui::BeginTabBar("MyTabBar")) {
        if (ImGui::BeginTabItem("Static .obj")) {
            guiStatic();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Skeletal .fbx")) {
            guiSkeletal();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::SeparatorText("Object Transform");
    guiTransform();

    ImGui::SeparatorText("Camera Controls");
    guiCameraControls();


    ImGui::Separator();


    // ImGui::Begin("GUI");

    ImGui::End();
    UI::endDraw();
}


static double previous_time = Window::getCurrentTime();
void Core::drawCurrentObject() {
    updateTransform();
    double delta_time = Window::getCurrentTime() - previous_time;
    previous_time = Window::getCurrentTime();
    if (render_options_.is_skeletal) {
        gl::Graphics::useSkinnedShader();
        gl::Graphics::setCameraUniforms(camera_.get());
        gl::Graphics::setLight(*light_);
        if (skinned_mesh_) {
            if (render_options_.play_animation) {
                skinned_mesh_->skeleton.playCurrentAnimation(delta_time);
            }
            if (render_options_.show_mesh) {
                glPolygonMode(GL_FRONT_AND_BACK, render_options_.mesh_wireframe ? GL_LINE : GL_FILL);
                gl::Graphics::drawSkinned(&skinned_mesh_->draw_mesh, skinned_mesh_->skeleton, transform_);
            }
            if (render_options_.show_collider && collider_) {
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, render_options_.collider_wireframe ? GL_LINE : GL_FILL);
                gl::Graphics::drawSkinned(collider_.get(), skinned_mesh_->skeleton, transform_);
                glEnable(GL_CULL_FACE);
            }
        }
    }
    else {
        if (static_mesh_) {
            gl::Graphics::usePhongShader();
            gl::Graphics::setCameraUniforms(camera_.get());
            gl::Graphics::setLight(*light_);

            if (render_options_.show_mesh) {
                GLenum mesh_mode = render_options_.mesh_wireframe ? GL_LINE : GL_FILL;
                glPolygonMode(GL_FRONT_AND_BACK, mesh_mode);
                gl::Graphics::drawMesh(static_mesh_.get(), transform_);
            }

            if (collider_ && render_options_.show_collider) {
                GLenum collider_mode = render_options_.collider_wireframe ? GL_LINE : GL_FILL;
                glPolygonMode(GL_FRONT_AND_BACK, collider_mode);
                gl::Graphics::drawMesh(collider_.get(), transform_);
            }
        }
    }
}

void Core::draw() {
    drawCurrentObject();
    drawGUI();
}


static auto last_mouse_pos = Window::getMousePosition();
void Core::update(double delta_time) {
    keyInputHandler(delta_time);




    if (!Window::isCursorVisible()) {
        auto mouse_pos = Window::getMousePosition();
        auto d_mouse = 0.1f*(last_mouse_pos - mouse_pos);
        last_mouse_pos = mouse_pos;
        s_camera_rotation.x += d_mouse.y;
        s_camera_rotation.y += d_mouse.x;
        s_camera_rotation.x = glm::clamp(s_camera_rotation.x, -89.0f, 89.0f);

    }
    glm::vec3 newFront;
    newFront.x = sin(glm::radians(s_camera_rotation.y)) * cos(glm::radians(s_camera_rotation.x));
    newFront.y = sin(glm::radians(s_camera_rotation.x));
    newFront.z = cos(glm::radians(s_camera_rotation.y)) * cos(glm::radians(s_camera_rotation.x));
    auto newLook = glm::normalize(newFront);

    camera_->setLook(newLook);
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
