#include "domain/elements/Rail.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace pinball::domain {

namespace {
constexpr glm::vec3 kRailColor{0.7f, 0.75f, 0.85f};
constexpr glm::vec3 kEntryColor{0.4f, 0.95f, 0.5f};
constexpr glm::vec3 kExitColor{0.95f, 0.55f, 0.3f};
} // namespace

Rail::Rail() {
    nodes_ = {glm::vec3(-2, 0.4f, 0), glm::vec3(0, 0.4f, -2), glm::vec3(2, 0.4f, 0)};
}

glm::vec3 Rail::position() const {
    return nodes_.empty() ? glm::vec3(0.0f) : nodes_.front();
}

void Rail::setPosition(const glm::vec3& p) {
    if (nodes_.empty()) return;
    glm::vec3 delta = p - nodes_.front();
    for (auto& n : nodes_) n += delta;
}

bool Rail::raycastHit(const Ray& ray, float& t) const {
    float best = -1.0f;
    for (const auto& n : nodes_) {
        float tt;
        if (raySphere(ray, n, tubeRadius_ + 0.2f, tt)) {
            if (best < 0 || tt < best) best = tt;
        }
    }
    if (best < 0) return false;
    t = best;
    return true;
}

void Rail::appendCollisionShapes(std::vector<CollisionShape>&) const {
    // While riding the rail the ball is constrained by the simulation, so the
    // rail contributes no passive collision shapes.
}

void Rail::appendRenderItems(std::vector<RenderItem>& out) const {
    for (size_t i = 0; i + 1 < nodes_.size(); ++i) {
        out.push_back(RenderItem{MeshId::Cylinder,
                                 cylinderBetween(nodes_[i], nodes_[i + 1], tubeRadius_),
                                 kRailColor});
    }
    for (size_t i = 0; i < nodes_.size(); ++i) {
        glm::vec3 color = kRailColor;
        if (i == 0) color = kEntryColor;
        else if (i + 1 == nodes_.size()) color = kExitColor;
        glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), nodes_[i]),
                                 glm::vec3(tubeRadius_ * 1.1f));
        out.push_back(RenderItem{MeshId::Sphere, m, color});
    }
}

void Rail::serialize(IArchive& ar) {
    ar.field("nodes", nodes_);
    ar.field("tubeRadius", tubeRadius_);
    ar.field("travelSpeed", travelSpeed_);
    ar.field("captureRadius", captureRadius_);
}

std::unique_ptr<IBoardElement> Rail::clone() const {
    return std::make_unique<Rail>(*this);
}

} // namespace pinball::domain
