#pragma once

namespace gl {
    class Camera {
    public:
        Camera();
        ~Camera() = default;


        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjection() const;
        glm::mat4 getProjection(float aspect_ratio) const;
        void setPosition(glm::vec3 position);
        glm::vec3 getPosition() const;
        void setLook(glm::vec3 look);
        glm::vec3 getLook() const;
        glm::vec3 getRight() const;
        glm::vec3 getUp() const;

    private:
        glm::vec3 position_;
        glm::vec3 look_;
        glm::vec3 up_;
        glm::vec3 world_up_;
        float near_;
        float far_;
        float fov_;

    };
};
