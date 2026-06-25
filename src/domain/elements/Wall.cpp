#include "domain/elements/Wall.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace pinball::domain {

namespace {
constexpr glm::vec3 kWallColor{0.55f, 0.58f, 0.65f};

glm::vec3 boxCenter(const glm::vec3& pos, float height) {
    return glm::vec3(pos.x, pos.y + height * 0.5f, pos.z);
}
} // namespace

bool Wall::raycastHit(const Ray& ray, float& t) const {
    float boundR = 0.5f * std::max(length_, std::max(height_, thickness_)) + 0.2f;
    return raySphere(ray, boxCenter(position_, height_), boundR, t);
}

void Wall::appendCollisionShapes(std::vector<CollisionShape>& out) const {
    glm::mat3 rot = rotationY(yaw_);
    glm::vec3 half(length_ * 0.5f, height_ * 0.5f, thickness_ * 0.5f);
    out.push_back(CollisionShape::makeBox(boxCenter(position_, height_), half, rot, 0.45f));
}

void Wall::appendRenderItems(std::vector<RenderItem>& out) const {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), boxCenter(position_, height_));
    m = m * glm::mat4(rotationY(yaw_));
    m = glm::scale(m, glm::vec3(length_, height_, thickness_));
    out.push_back(RenderItem{MeshId::Cube, m, kWallColor});
}

void Wall::serialize(IArchive& ar) {
    ar.field("position", position_);
    ar.field("length", length_);
    ar.field("thickness", thickness_);
    ar.field("height", height_);
    ar.field("yaw", yaw_);
}

std::unique_ptr<IBoardElement> Wall::clone() const {
    return std::make_unique<Wall>(*this);
}

} // namespace pinball::domain
