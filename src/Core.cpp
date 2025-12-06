#include "Core.h"

#include <iostream>


#include "Debug.h"
#include "imgui.h"
#include "Util.h"
#include "Window.h"
#include "../External/assimp/code/AssetLib/FBX/FBXExporter.h"

#include "render/Camera.h"
#include "render/Mesh.h"
#include "render/SkeletalMesh.h"


constexpr glm::vec3 initial_camera_position = glm::vec3(0.f, 15.f, 10.f);
constexpr glm::vec2 initial_camera_rotation = glm::vec2(-45.f, 180.0f);

// Orbit camera controls (spherical coordinates)
static float orbit_azimuth = 0.0f; // Angle around Y axis (latitude, in degrees)
static float orbit_elevation = 45.0f; // Angle from XZ plane (longitude, in degrees)
static float orbit_distance = 20.0f; // Distance from origin
constexpr glm::vec3 orbit_target = glm::vec3(0.0f, 0.0f, 0.0f); // Always look at origin

Core::Core() : camera_(std::make_shared<gl::Camera>()), light_(std::make_shared<gl::Light>())
// , skinned_mesh_(std::make_unique<gl::SkinnedMesh>(gl::SkeletalMesh::loadFbx("Resources/Models/walking.fbx")))
{
    light_->position = glm::vec3(0, 20, 5);

    // Initialize orbit camera position using spherical coordinates
    float azimuth_rad = glm::radians(orbit_azimuth);
    float elevation_rad = glm::radians(orbit_elevation);
    glm::vec3 pos = glm::vec3(
        orbit_distance * cos(elevation_rad) * sin(azimuth_rad),
        orbit_distance * sin(elevation_rad),
        orbit_distance * cos(elevation_rad) * cos(azimuth_rad)
    );
    camera_->setPosition(pos);
    camera_->setLook(glm::normalize(orbit_target - pos));
    Window::setClearColor(glm::vec3(.5f));
}


static float s_scale;
glm::vec2 s_scale_bounds;
static glm::vec3 s_position;

static glm::vec3 s_rotation = glm::vec3(0, 0, 0); // x y z rotation in degrees

void Core::resetCamera() {
    orbit_azimuth = 0.0f;
    orbit_elevation = 45.0f;
    orbit_distance = 20.0f;

    // Update camera position using spherical coordinates
    float azimuth_rad = glm::radians(orbit_azimuth);
    float elevation_rad = glm::radians(orbit_elevation);
    glm::vec3 pos = glm::vec3(
        orbit_distance * cos(elevation_rad) * sin(azimuth_rad),
        orbit_distance * sin(elevation_rad),
        orbit_distance * cos(elevation_rad) * cos(azimuth_rad)
    );
    camera_->setPosition(pos);
    camera_->setLook(glm::normalize(orbit_target - pos));
}


glm::mat4 getRotation() {
    auto rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, glm::radians(s_rotation.x), {1, 0, 0});
    rotation = glm::rotate(rotation, glm::radians(s_rotation.y), {0, 1, 0});
    rotation = glm::rotate(rotation, glm::radians(s_rotation.z), {0, 0, 1});
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
        ImGui::SameLine();
        if (ImGui::Button("Reset##Position")) s_position = {0, 0, 0};

        ImGui::SliderFloat("Scale   ", &s_scale, s_scale_bounds.x, s_scale_bounds.y);
        ImGui::SameLine();
        if (ImGui::Button("Reset##Scale")) s_scale = (s_scale_bounds.x + s_scale_bounds.y) / 10.f;

        ImGui::SliderFloat3("Rotation", glm::value_ptr(s_rotation), 0, 360);
        ImGui::SameLine();
        if (ImGui::Button("Reset##Rotation")) s_rotation = {0, 0, 0};
    }
    else {
        ImGui::Text("Upload a mesh to see transform options.");
    }
}

void Core::loadNewMesh(const std::string& path) {
    info_.object_path = path;
    skinned_mesh_.reset();
    static_mesh_.reset();
    collider_.reset();
    render_options_ = RenderOptions();
}

void Core::updateTransform() {
    transform_.setScale(s_scale);
    transform_.setPosition(s_position);
    transform_.setRotation(getRotation());
}

constexpr ImVec4 DECOMPOSE_BUTTON_COLOR = ImVec4(0.2f, 0.5f, 0.2f, 1.0f);

void Core::guiColliderOutput() {
    if (collider_) {
        auto collider_path = info_.object_path;
        auto extension = util::removeExtension(collider_path);
        auto stem = util::getStem(info_.object_path);
        auto directory = util::getDirectory(info_.object_path);
        collider_path = directory + "/Colliders/" + stem + "_collider" + extension;
        ImGui::Text("Collider outputted to:");
        ImGui::TextColored({0.2, 0.7, 0.2, 1}, "%s", collider_path.c_str());
    }
}

static int s_preset = 0;


void Core::guiCoacdParams() {
    if (ImGui::BeginTabBar("Coacd Parameters")) {
        if (ImGui::BeginTabItem("Presets")) {
            ImGui::RadioButton("Fast", &s_preset, 0); ImGui::SameLine();
            ImGui::RadioButton("Balanced", &s_preset, 1); ImGui::SameLine();
            ImGui::RadioButton("Accurate", &s_preset, 2);
            switch (s_preset) {
            default: {
                params_.threshold = 0.5f;
                params_.resolution = 1000;
                params_.max_convex_hull = 10;
                params_.extrude = 0.0f;
                break;
            }
            case 1: {
                params_.threshold = 0.3f;
                params_.resolution = 3000;
                params_.max_convex_hull = 20;
                params_.extrude = 0.0f;
                break;
            }
            case 2: {
                params_.threshold = 0.1f;
                params_.resolution = 8000;
                params_.max_convex_hull = -1;
                params_.extrude = 0.0f;
                break;
            }
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Custom settings")) {
            ImGui::SliderFloat("Threshold", &params_.threshold, 0.01f, 1.f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip(
                "Concavity threshold. Lower values produce more accurate colliders but result in more convex pieces and is slower");
            ImGui::SliderInt("Resolution", &params_.resolution, 100, 10000);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip(
                "Surface sampling resolution. Higher values create smoother convex hulls with better surface detail");

            // Custom display for Max Convex Hulls slider to show infinity symbol
            ImGui::SliderInt("Max Convex Hulls", &params_.max_convex_hull, -1, 100,
                             params_.max_convex_hull == -1 ? "Unlimited" : "%d");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip(
                "Limits the number of generated convex hulls. Useful for performance budgets in game engines");

            ImGui::SliderFloat("Extrude", &params_.extrude, -.5f, .5f);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip(
                "Extrudes the convex hulls outward (positive values) or inward (negative values). 0.1 = 10%% larger");
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();
}

void Core::guiStatic() {
    if (ImGui::Button("Upload OBJ File")) {
        auto path = UI::openFileExplorer({{"obj file", "obj"}});
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
        guiCoacdParams();
        ImGui::PushStyleColor(ImGuiCol_Button, DECOMPOSE_BUTTON_COLOR);
        if (ImGui::Button("Decompose Static Mesh")) {
            collider_.reset();
            collider_ = std::make_unique<gl::DrawMesh>(gl::Mesh::decomposeObj(info_.object_path.c_str(), params_));
            render_options_.show_collider = true;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Checkbox("AABB mode", &params_.aab_mode);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip(
            "Use axis-aligned bounding boxes instead of convex hulls. Faster physics but less accurate collision");


        if (collider_) {
            guiColliderOutput();
        }
    }
}

static int current_animation = 0;

void Core::setAnimation(int index) {
    if (skinned_mesh_ && index >= 0 && index < skinned_mesh_->skeleton.animation_list_.size()) {
        if (skinned_mesh_->skeleton.current_animation_ != &skinned_mesh_->skeleton.animations_[skinned_mesh_->skeleton.
            animation_list_[index]]) {
            skinned_mesh_->skeleton.setCurrentAnimation(skinned_mesh_->skeleton.animation_list_[index]);
        }
    }
    if (index == -1 && skinned_mesh_) {
        skinned_mesh_->skeleton.current_animation_ = nullptr;
        skinned_mesh_->skeleton.resetToBindPose();
    }
}

bool s_bone_selection[1024] = {false};

std::vector<unsigned int> Core::getCustomBones() {
    std::vector<unsigned int> selected_bones;
    for (int i = 0; i < skinned_mesh_->skeleton.bones_.size(); i++) {
        if (s_bone_selection[i]) {
            selected_bones.push_back(i);
        }
    }
    return selected_bones;
}

void Core::guiCustomBones() {
    for (int i = 0; i < skinned_mesh_->skeleton.bones_.size(); i++) {
        const auto& bone = skinned_mesh_->skeleton.bones_[i];
        ImGui::Checkbox(bone.name.c_str(), &s_bone_selection[i]);
    }
}

int decomp_mode = gl::BoneDecompositionMode::IMPORTANT_BONES;

void Core::guiSkeletal() {
    if (ImGui::Button("Upload FBX File")) {
        auto path = UI::openFileExplorer({{"fbx file", "fbx"}});
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
    ImGui::RadioButton("Important Bones", &decomp_mode, gl::BoneDecompositionMode::IMPORTANT_BONES);
    ImGui::RadioButton("All Bones", &decomp_mode, gl::BoneDecompositionMode::ALL_BONES);
    ImGui::RadioButton("Custom Bones", &decomp_mode, gl::BoneDecompositionMode::CUSTOM_BONES);

    if (decomp_mode == gl::BoneDecompositionMode::CUSTOM_BONES) {
        ImGui::SeparatorText("Select Bones");
        guiCustomBones();
    }

    ImGui::PushStyleColor(ImGuiCol_Button, DECOMPOSE_BUTTON_COLOR);
    if (ImGui::Button("Decompose Skeletal Mesh")) {
        collider_.reset();
        collider_ = std::make_unique<gl::DrawMesh>(gl::SkeletalMesh::decomposeSkeleton(
            skinned_mesh_->skeleton, info_.object_path.c_str(),
            (gl::BoneDecompositionMode)decomp_mode,
            getCustomBones(), params_.aab_mode));
        render_options_.show_collider = true;
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Checkbox("AABB mode", &params_.aab_mode);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(
        "Use axis-aligned bounding boxes instead of convex hulls. Faster physics but less accurate collision");


    if (collider_) {
        guiColliderOutput();
    }

    ImGui::SeparatorText("Animations");
    ImGui::Text("Select Animation:");
    if (skinned_mesh_) {
        ImGui::RadioButton("Off", &current_animation, -1);
        for (int i = 0; i < skinned_mesh_->skeleton.animation_list_.size(); i++) {
            const auto& anim_name = skinned_mesh_->skeleton.animation_list_[i];
            ImGui::RadioButton(anim_name.c_str(), &current_animation, i);
        }
        setAnimation(current_animation);
    }
    ImGui::Checkbox("Play animation", &render_options_.play_animation);
}

void Core::guiRenderOptions() {
    if (!skinned_mesh_ && !static_mesh_) {
        ImGui::Text("Upload a mesh to see render options.");
        return;
    }

    ImGui::Checkbox("Show Mesh      ", &render_options_.show_mesh);
    ImGui::SameLine();
    ImGui::Checkbox("Wireframe Mesh", &render_options_.mesh_wireframe);
    if (!collider_) {
        ImGui::Text("Decompose mesh to see collider options");
        return;
    }
    ImGui::Checkbox("Show Collider  ", &render_options_.show_collider);
    ImGui::SameLine();
    ImGui::Checkbox("Wireframe Collider", &render_options_.collider_wireframe);
}

void Core::guiCameraControls() {
    if (ImGui::Button("Reset Camera")) {
        resetCamera();
    }
}

static glm::vec2 s_ui_window_size = glm::vec2(400, 600);

void Core::drawGUI() {
    UI::beginDraw(0, 0, 400, 600);
    ImGui::Begin("Mesh Decomposer");
    ImGui::Text("Controls:");
    ImGui::Text("  Click + Drag mouse to orbit around mesh");
    ImGui::Text("  Scroll to zoom in/out");

    s_ui_window_size = glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

    ImGui::SeparatorText("Render Options");
    guiRenderOptions();
    ImGui::SeparatorText("Choose mesh type:");
    ImGui::Spacing();

    // Style the tab bar with custom colors
    ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.3f, 0.3f, 0.3f, 1.0f)); // Inactive tab
    ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.7f, 0.3f, 0.3f, 1.0f)); // Hovered tab
    ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.5f, 0.2f, 0.2f, 1.0f)); // Active tab (reddish)
    ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImVec4(0.25f, 0.25f, 0.25f, 1.0f)); // Unfocused tab
    ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImVec4(0.5f, 0.15f, 0.15f, 1.0f)); // Unfocused active tab

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

    ImGui::PopStyleColor(5); // Pop all 5 colors we pushed

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

static bool withinUIWindow(const glm::vec2& mouse_pos) {
    return mouse_pos.x >= 0 && mouse_pos.x <= s_ui_window_size.x + 10 &&
        mouse_pos.y >= 0 && mouse_pos.y <= s_ui_window_size.y + 10;
}

void Core::onScroll(double xoffset, double yoffset) {
    if (withinUIWindow(Window::getMousePosition())) {
        return;
    }
    float mod = 1.f * yoffset;
    orbit_distance -= mod;
    orbit_distance = glm::clamp(orbit_distance, 1.0f, 100.0f);
}


static auto last_mouse_pos = Window::getMousePosition();
static bool s_mouse_button_down_ = false;

void Core::onMouseButton(int button, int action, int mods) {
    const auto pos = Window::getMousePosition();
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && !withinUIWindow(pos)) {
            s_mouse_button_down_ = true;
            last_mouse_pos = pos;
        }
        else if (action == GLFW_RELEASE) {
            s_mouse_button_down_ = false;
        }
    }
}

void Core::update(double delta_time) {
    keyInputHandler(delta_time);

    if (s_mouse_button_down_) {
        auto mouse_pos = Window::getMousePosition();
        auto d_mouse = last_mouse_pos - mouse_pos;
        last_mouse_pos = mouse_pos;

        // Horizontal mouse movement (X) = azimuth (latitude, around Y axis)
        orbit_azimuth += d_mouse.x * 0.5f; // 0.5 sensitivity

        // Vertical mouse movement (Y) = elevation (longitude, angle from XZ plane)
        orbit_elevation -= d_mouse.y * 0.5f; // 0.5 sensitivity
        orbit_elevation = glm::clamp(orbit_elevation, -89.0f, 89.0f); // Prevent gimbal lock
    }

    float azimuth_rad = glm::radians(orbit_azimuth);
    float elevation_rad = glm::radians(orbit_elevation);
    glm::vec3 pos = glm::vec3(
        orbit_distance * cos(elevation_rad) * sin(azimuth_rad),
        orbit_distance * sin(elevation_rad),
        orbit_distance * cos(elevation_rad) * cos(azimuth_rad)
    );
    camera_->setPosition(pos);
    camera_->setLook(glm::normalize(orbit_target - pos));
}

void Core::keyInputHandler(double delta_time) {
    float mod = 10.f * delta_time; // Speed for orbit distance changes

    // W/S keys adjust orbit distance (zoom in/out)
    if (Window::key(GLFW_KEY_W)) {
        orbit_distance -= mod;
        orbit_distance = glm::max(orbit_distance, 1.0f); // Minimum distance
    }
    if (Window::key(GLFW_KEY_S)) {
        orbit_distance += mod;
        orbit_distance = glm::min(orbit_distance, 100.0f); // Maximum distance
    }

    // A/D keys orbit left/right (azimuth / latitude)
    if (Window::key(GLFW_KEY_A)) {
        orbit_azimuth -= mod * 5.0f;
    }
    if (Window::key(GLFW_KEY_D)) {
        orbit_azimuth += mod * 5.0f;
    }

    // Space/Shift adjust elevation (longitude)
    if (Window::key(GLFW_KEY_SPACE)) {
        orbit_elevation += mod * 2.0f;
        orbit_elevation = glm::min(orbit_elevation, 89.0f);
    }
    if (Window::key(GLFW_KEY_LEFT_SHIFT)) {
        orbit_elevation -= mod * 2.0f;
        orbit_elevation = glm::max(orbit_elevation, -89.0f);
    }
}

void Core::loadObject(const std::string& name) {
}
