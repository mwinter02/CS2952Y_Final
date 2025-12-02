#include "Camera.h"
#include "../Window.h"

namespace gl {
    Camera::Camera() : position_(0.0f, 0.0f, 0.0f),
                     look_(0.0f, 0.0f, 1.0f),
                     up_(0.0f, 1.0f, 0.0f),
                     world_up_(0.0f, 1.0f, 0.0f),
                     near_(0.1f),
                     far_(100.0f),
                     fov_(glm::half_pi<float>()) {
    }

    glm::mat4 Camera::getViewMatrix() const {
        return glm::lookAt(position_, position_ + look_, up_);
    }

    glm::mat4 Camera::getProjection() const {
        return glm::perspective(fov_, Window::getAspectRatio(), near_, far_);
    }

    glm::mat4 Camera::getProjection(float aspect_ratio) const {
        return glm::perspective(fov_, aspect_ratio, near_, far_);
    }

    void Camera::setPosition(glm::vec3 position) {
        position_ = position;
    }

    glm::vec3 Camera::getPosition() const {
        return position_;
    }

    void Camera::setLook(const glm::vec3 look) {
        look_ = look;
    }

    glm::vec3 Camera::getLook() const {
        return look_;
    }

    glm::vec3 Camera::getRight() const {
        return glm::normalize(glm::cross(look_, world_up_));
    }

    glm::vec3 Camera::getUp() const {
        return up_;
    }
}

