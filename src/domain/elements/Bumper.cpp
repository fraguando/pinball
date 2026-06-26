#include "domain/elements/Bumper.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace pinball::domain {

namespace {
constexpr glm::vec3 kBumperColor{0.95f, 0.35f, 0.55f};
constexpr glm::vec3 kGlowColor{1.0f, 0.5f, 0.7f};
}

bool Bumper::raycastHit(const Ray& ray, float& t) const {
    glm::vec3 c = position_ + glm::vec3(0, height_ * 0.5f, 0);
    return raySphere(ray, c, radius_ + 0.1f, t);
}

void Bumper::appendCollisionShapes(std::vector<CollisionShape>& out) const {
    // Vertical capsule acts as a cylinder the ball hits from the side; high
    // restitution plus a bonus pop give the bumper its kick, and it scores.
    glm::vec3 p0 = position_ + glm::vec3(0, 0.05f, 0);
    glm::vec3 p1 = position_ + glm::vec3(0, height_, 0);
    CollisionShape s = CollisionShape::makeCapsule(p0, p1, radius_, 1.05f);
    s.bonusSpeed = 15.0f; // strong "pop" away from the bumper
    s.scoreOnHit = 100;
    out.push_back(s);
}

void Bumper::appendRenderItems(std::vector<RenderItem>& out) const {
    // Base glow always on; flares up briefly when hit.
    float glow = 0.3f + 0.9f * flash_;
    glm::vec3 emissive = kGlowColor * glow;

    glm::vec3 mid = position_ + glm::vec3(0, height_ * 0.5f, 0);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), mid);
    m = glm::scale(m, glm::vec3(radius_, height_, radius_));
    out.push_back(RenderItem{MeshId::Cylinder, m, kBumperColor, emissive, 1.0f});

    // Glowing cap "light".
    glm::mat4 cap = glm::translate(glm::mat4(1.0f), position_ + glm::vec3(0, height_, 0));
    cap = glm::scale(cap, glm::vec3(radius_));
    out.push_back(RenderItem{MeshId::Sphere, cap, kBumperColor * 1.1f, emissive * 1.3f, 1.0f});

    // Translucent halo that swells on impact (flashy light bloom).
    float haloScale = radius_ * (1.5f + 0.6f * flash_);
    glm::mat4 halo = glm::translate(glm::mat4(1.0f), position_ + glm::vec3(0, height_ * 0.6f, 0));
    halo = glm::scale(halo, glm::vec3(haloScale));
    out.push_back(RenderItem{MeshId::Sphere, halo, kGlowColor, kGlowColor * (0.6f + flash_),
                             0.12f + 0.35f * flash_});
}

void Bumper::serialize(IArchive& ar) {
    ar.field("position", position_);
    ar.field("radius", radius_);
    ar.field("height", height_);
}

std::unique_ptr<IBoardElement> Bumper::clone() const {
    return std::make_unique<Bumper>(*this);
}

} // namespace pinball::domain
