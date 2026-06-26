#include "domain/elements/Spawn.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace pinball::domain {

namespace {
constexpr glm::vec3 kMetal{0.7f, 0.72f, 0.78f};
constexpr glm::vec3 kChargeGlow{1.0f, 0.55f, 0.1f};
constexpr float kRestLength = 2.2f;
constexpr int kCoils = 6;
} // namespace

void Spawn::setCompression(float c) {
    compression_ = std::clamp(c, 0.0f, 1.0f);
}

glm::vec3 Spawn::launchDir() const {
    return glm::normalize(rotationY(yaw_) * glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 Spawn::launchVelocity(float charge) const {
    charge = std::clamp(charge, 0.0f, 1.0f);
    return launchDir() * (minSpeed_ + charge * (maxSpeed_ - minSpeed_));
}

bool Spawn::raycastHit(const Ray& ray, float& t) const {
    return raySphere(ray, position_ + glm::vec3(0, 0.4f, 0), 1.2f, t);
}

void Spawn::appendRenderItems(std::vector<RenderItem>& out) const {
    const glm::vec3 axis = launchDir();           // launch direction (forward)
    const glm::vec3 ballPos = position_ + glm::vec3(0, 0.42f, 0);
    const float length = kRestLength * (1.0f - 0.55f * compression_);
    const glm::vec3 back = ballPos - axis * length; // spring sits behind the ball

    const glm::vec3 glow = kChargeGlow * (0.15f + 0.95f * compression_);

    auto disk = [&](const glm::vec3& center, float radius, const glm::vec3& col,
                    const glm::vec3& emis) {
        out.push_back(RenderItem{MeshId::Cylinder,
                                 cylinderBetween(center - axis * 0.05f, center + axis * 0.05f, radius),
                                 col, emis, 1.0f});
    };

    // Back plate + front pusher.
    disk(back, 0.6f, kMetal * 0.8f, glow);
    disk(ballPos, 0.55f, kMetal, glow);

    // Coil discs evenly spaced along the (compressed) spring.
    for (int i = 1; i < kCoils; ++i) {
        float f = static_cast<float>(i) / kCoils;
        disk(back + axis * (length * f), 0.5f, kMetal, glow);
    }

    // Bright start marker so the spawn point reads clearly in the editor.
    glm::mat4 dot = glm::scale(glm::translate(glm::mat4(1.0f), ballPos), glm::vec3(0.18f));
    out.push_back(RenderItem{MeshId::Sphere, dot, glm::vec3(0.6f, 0.95f, 1.0f),
                             glm::vec3(0.2f, 0.4f, 0.5f), 1.0f});
}

void Spawn::serialize(IArchive& ar) {
    ar.field("position", position_);
    ar.field("yaw", yaw_);
}

std::unique_ptr<IBoardElement> Spawn::clone() const {
    return std::make_unique<Spawn>(*this);
}

} // namespace pinball::domain
