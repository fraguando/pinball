#include "domain/elements/Spawn.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace pinball::domain {

namespace {
constexpr glm::vec3 kSpawnColor{0.5f, 0.9f, 1.0f};
}

bool Spawn::raycastHit(const Ray& ray, float& t) const {
    return raySphere(ray, position_ + glm::vec3(0, 0.2f, 0), 0.6f, t);
}

void Spawn::appendRenderItems(std::vector<RenderItem>& out) const {
    // A flat ring marker plus a small ball showing the start point.
    glm::mat4 ring = glm::translate(glm::mat4(1.0f), position_ + glm::vec3(0, 0.02f, 0));
    ring = glm::scale(ring, glm::vec3(0.7f, 0.06f, 0.7f));
    out.push_back(RenderItem{MeshId::Cylinder, ring, kSpawnColor});

    glm::mat4 dot = glm::translate(glm::mat4(1.0f), position_ + glm::vec3(0, 0.25f, 0));
    dot = glm::scale(dot, glm::vec3(0.22f));
    out.push_back(RenderItem{MeshId::Sphere, dot, kSpawnColor});
}

void Spawn::serialize(IArchive& ar) {
    ar.field("position", position_);
}

std::unique_ptr<IBoardElement> Spawn::clone() const {
    return std::make_unique<Spawn>(*this);
}

} // namespace pinball::domain
