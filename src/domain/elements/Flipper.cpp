#include "domain/elements/Flipper.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace pinball::domain {

namespace {
constexpr glm::vec3 kFlipperColor{0.95f, 0.85f, 0.25f};
constexpr float kAngularSpeed = 26.0f; // rad/s — snappy, hard-hitting flip
} // namespace

glm::vec3 Flipper::tip() const {
    glm::vec3 dir(std::cos(currentAngle_), 0.0f, std::sin(currentAngle_));
    glm::vec3 base(pivot_.x, kHeight, pivot_.z);
    return base + dir * length_;
}

void Flipper::rotate(float deltaRadians) {
    restAngle_ += deltaRadians;
    activeAngle_ += deltaRadians;
    currentAngle_ += deltaRadians;
}

void Flipper::setSide(bool left) {
    controlLeft_ = left;
    if (left) {
        restAngle_ = 0.45f;    // bar points right & down-table
        activeAngle_ = -0.55f; // swings up-table
    } else {
        restAngle_ = glm::pi<float>() - 0.45f;    // bar points left & down-table
        activeAngle_ = glm::pi<float>() + 0.55f;  // swings up-table
    }
    currentAngle_ = restAngle_;
}

void Flipper::applyInput(const InputState& input) {
    bool pressed = controlLeft_ ? input.leftFlipper : input.rightFlipper;
    float target = pressed ? activeAngle_ : restAngle_;

    float prev = currentAngle_;
    float maxStep = kAngularSpeed * input.dt;
    float diff = target - currentAngle_;
    diff = std::clamp(diff, -maxStep, maxStep);
    currentAngle_ += diff;
    angularVel_ = input.dt > 1e-6f ? (currentAngle_ - prev) / input.dt : 0.0f;
}

bool Flipper::raycastHit(const Ray& ray, float& t) const {
    glm::vec3 base(pivot_.x, kHeight, pivot_.z);
    return raySphere(ray, base + (tip() - base) * 0.5f, length_ * 0.6f, t);
}

void Flipper::appendCollisionShapes(std::vector<CollisionShape>& out) const {
    glm::vec3 base(pivot_.x, kHeight, pivot_.z);
    glm::vec3 t = tip();
    CollisionShape s = CollisionShape::makeCapsule(base, t, barRadius_, 0.45f);
    // Linear velocity near the bar tip due to the swing: perpendicular to the
    // bar in the XZ plane. The 2.4*length factor makes the kick ~3x as strong.
    glm::vec3 perp(-std::sin(currentAngle_), 0.0f, std::cos(currentAngle_));
    s.surfaceVel = perp * (angularVel_ * length_ * 2.4f);
    out.push_back(s);
}

void Flipper::appendRenderItems(std::vector<RenderItem>& out) const {
    glm::vec3 base(pivot_.x, kHeight, pivot_.z);
    glm::vec3 t = tip();
    out.push_back(RenderItem{MeshId::Cylinder, cylinderBetween(base, t, barRadius_), kFlipperColor});

    glm::mat4 pivotBall = glm::scale(
        glm::translate(glm::mat4(1.0f), base), glm::vec3(barRadius_ * 1.3f));
    out.push_back(RenderItem{MeshId::Sphere, pivotBall, kFlipperColor * 0.8f});

    glm::mat4 tipBall = glm::scale(
        glm::translate(glm::mat4(1.0f), t), glm::vec3(barRadius_));
    out.push_back(RenderItem{MeshId::Sphere, tipBall, kFlipperColor});
}

void Flipper::serialize(IArchive& ar) {
    ar.field("pivot", pivot_);
    ar.field("length", length_);
    ar.field("barRadius", barRadius_);
    ar.field("restAngle", restAngle_);
    ar.field("activeAngle", activeAngle_);
    ar.field("controlLeft", controlLeft_);
    if (ar.isLoading()) currentAngle_ = restAngle_;
}

std::unique_ptr<IBoardElement> Flipper::clone() const {
    return std::make_unique<Flipper>(*this);
}

} // namespace pinball::domain
