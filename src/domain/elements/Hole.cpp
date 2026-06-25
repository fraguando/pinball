#include "domain/elements/Hole.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace pinball::domain {

namespace {
constexpr glm::vec3 kRimColor{0.2f, 0.75f, 0.85f};
constexpr glm::vec3 kPitColor{0.03f, 0.05f, 0.08f};
} // namespace

bool Hole::raycastHit(const Ray& ray, float& t) const {
    return raySphere(ray, position_, radius_ + 0.1f, t);
}

void Hole::appendCollisionShapes(std::vector<CollisionShape>&) const {
    // The hole is a capture zone, not a collider: the simulation intercepts the
    // ball directly, so it emits no collision shapes.
}

void Hole::appendRenderItems(std::vector<RenderItem>& out) const {
    // Rim ring (slightly raised, larger cylinder).
    glm::mat4 rim = glm::translate(glm::mat4(1.0f), position_ + glm::vec3(0, 0.03f, 0));
    rim = glm::scale(rim, glm::vec3(radius_ * 1.15f, 0.12f, radius_ * 1.15f));
    out.push_back(RenderItem{MeshId::Cylinder, rim, kRimColor});

    // Dark pit disc inside.
    glm::mat4 pit = glm::translate(glm::mat4(1.0f), position_ + glm::vec3(0, 0.05f, 0));
    pit = glm::scale(pit, glm::vec3(radius_ * 0.85f, 0.12f, radius_ * 0.85f));
    out.push_back(RenderItem{MeshId::Cylinder, pit, kPitColor});
}

bool Hole::shouldCapture(const Ball& ball) const {
    glm::vec2 d(ball.position.x - position_.x, ball.position.z - position_.z);
    bool over = glm::dot(d, d) <= radius_ * radius_;
    bool nearFloor = ball.position.y < ball.radius + 0.5f;
    return over && nearFloor;
}

glm::vec3 Hole::capturePoint() const {
    return position_ + glm::vec3(0, 0.2f, 0);
}

glm::vec3 Hole::ejectVelocity() const {
    // Pop up and back up-table (-Z) to return the ball to play.
    return glm::vec3(0.0f, 6.0f, -7.0f);
}

void Hole::serialize(IArchive& ar) {
    ar.field("position", position_);
    ar.field("radius", radius_);
    ar.field("holdSeconds", holdSeconds_);
}

std::unique_ptr<IBoardElement> Hole::clone() const {
    return std::make_unique<Hole>(*this);
}

} // namespace pinball::domain
