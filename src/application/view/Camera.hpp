#pragma once
#include "domain/math/Geometry.hpp"

#include <glm/glm.hpp>

namespace pinball::app {

// An orbit camera around a target point. Used both for the editor (interactive
// orbit/zoom) and play (fixed angle). Pure math: produces view/projection
// matrices and unprojects screen pixels into world rays.
class Camera {
public:
    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    glm::vec3 eye() const;

    // Build a ray from the camera through a screen pixel (origin top-left).
    domain::Ray screenToRay(const glm::vec2& pixel, int widthPx, int heightPx) const;

    void orbit(float dYaw, float dPitch);
    void zoom(float amount);

    void setTarget(const glm::vec3& t) { target_ = t; }
    void set(float yaw, float pitch, float distance);

private:
    glm::vec3 target_{0.0f, 0.0f, 0.0f};
    float yaw_{glm::radians(90.0f)};   // around +Y
    float pitch_{glm::radians(55.0f)}; // above the table
    float distance_{34.0f};
    float fovY_{glm::radians(45.0f)};
    float near_{0.1f};
    float far_{300.0f};
};

} // namespace pinball::app
