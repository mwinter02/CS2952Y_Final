#pragma once

namespace gl {
    class Transform {
    public:
        Transform();
        ~Transform() = default;

        void setPosition(const glm::vec3& pos);
        void setScale(const glm::vec3& scale);
        void setScale(float scale);
        void setRotation(const glm::mat4& rotation_matrix);
        void rotate(float angle, glm::vec3 axis);
        void rotateDegrees(float angle_degrees, glm::vec3 axis);

        glm::mat4 getModelMatrix() const;


    private:
        glm::vec3 position_;
        glm::vec3 scale_;
        glm::mat4 rotation_;

    };
}
