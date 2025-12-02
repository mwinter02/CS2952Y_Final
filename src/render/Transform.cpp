//
// Created by Marcus Winter on 11/15/25.
//

#include "Transform.h"

namespace gl {
    Transform::Transform() : position_(0.0f, 0.0f, 0.0f), scale_(1.0f, 1.0f, 1.0f), rotation_(1.0f) {

    }

    void Transform::setPosition(const glm::vec3& pos) {
        position_ = pos;
    }

    void Transform::setScale(const glm::vec3& scale) {
        scale_ = scale;
    }

    void Transform::setRotation(const glm::mat4& rotation_matrix) {
        rotation_ = rotation_matrix;
    }

    /**
     * Rotate the transform by the given angle in Radians around the given axis
     * @param angle - rotation angle in radians
     * @param axis - axis to rotate around
     */
    void Transform::rotate(const float angle, const glm::vec3 axis) {
        rotation_ = glm::rotate(rotation_, angle, axis);
    }

    /**
     * Rotate the transform by the given angle in degrees around the given axis
     * @param angle_degrees rotation angle in degrees
     * @param axis - axis to rotate around
     */
    void Transform::rotateDegrees(float angle_degrees, glm::vec3 axis) {
        rotate(glm::radians(angle_degrees), axis);
    }

    glm::mat4 Transform::getModelMatrix() const {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, position_);
        model = model * rotation_;
        model = glm::scale(model, scale_);
        return model;
    }

}
