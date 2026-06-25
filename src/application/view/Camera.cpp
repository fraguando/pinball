#include "application/view/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace pinball::app {

glm::vec3 Camera::eye() const {
    float cp = std::cos(pitch_);
    glm::vec3 dir(cp * std::cos(yaw_), std::sin(pitch_), cp * std::sin(yaw_));
    return target_ + dir * distance_;
}

glm::mat4 Camera::view() const {
    return glm::lookAt(eye(), target_, glm::vec3(0, 1, 0));
}

glm::mat4 Camera::projection(float aspect) const {
    return glm::perspective(fovY_, aspect, near_, far_);
}

void Camera::orbit(float dYaw, float dPitch) {
    yaw_ += dYaw;
    pitch_ = std::clamp(pitch_ + dPitch, glm::radians(15.0f), glm::radians(89.0f));
}

void Camera::zoom(float amount) {
    distance_ = std::clamp(distance_ - amount, 8.0f, 90.0f);
}

void Camera::set(float yaw, float pitch, float distance) {
    yaw_ = yaw;
    pitch_ = pitch;
    distance_ = distance;
}

domain::Ray Camera::screenToRay(const glm::vec2& pixel, int widthPx, int heightPx) const {
    float ndcX = 2.0f * pixel.x / static_cast<float>(widthPx) - 1.0f;
    float ndcY = 1.0f - 2.0f * pixel.y / static_cast<float>(heightPx);

    float aspect = static_cast<float>(widthPx) / static_cast<float>(std::max(1, heightPx));
    glm::mat4 invVP = glm::inverse(projection(aspect) * view());

    glm::vec4 nearP = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 farP = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
    nearP /= nearP.w;
    farP /= farP.w;

    domain::Ray ray;
    ray.origin = glm::vec3(nearP);
    ray.dir = glm::normalize(glm::vec3(farP - nearP));
    return ray;
}

} // namespace pinball::app
