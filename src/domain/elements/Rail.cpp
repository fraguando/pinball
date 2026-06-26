#include "domain/elements/Rail.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace pinball::domain {

namespace {
constexpr glm::vec3 kRailColor{0.72f, 0.78f, 0.9f};
constexpr glm::vec3 kRailGlow{0.15f, 0.2f, 0.35f};
constexpr glm::vec3 kEntryColor{0.4f, 0.95f, 0.5f};
constexpr glm::vec3 kExitColor{0.95f, 0.55f, 0.3f};

// Horizontal "side" direction for a path segment (perpendicular to its forward
// direction in the XZ plane).
glm::vec3 sideDir(const glm::vec3& a, const glm::vec3& b) {
    glm::vec3 fwd = b - a;
    glm::vec3 flat(fwd.x, 0.0f, fwd.z);
    if (glm::dot(flat, flat) < 1e-8f) return glm::vec3(1, 0, 0);
    glm::vec3 s = glm::cross(glm::normalize(flat), glm::vec3(0, 1, 0));
    return glm::normalize(s);
}
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

void Rail::rotate(float deltaRadians) {
    if (nodes_.empty()) return;
    glm::vec3 c(0.0f);
    for (const auto& n : nodes_) c += n;
    c /= static_cast<float>(nodes_.size());
    glm::mat3 rot = rotationY(deltaRadians);
    for (auto& n : nodes_) n = c + rot * (n - c);
}

bool Rail::raycastHit(const Ray& ray, float& t) const {
    float best = -1.0f;
    for (const auto& n : nodes_) {
        float tt;
        if (raySphere(ray, n, sideOffset_ + 0.3f, tt)) {
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
    if (nodes_.size() < 2) return; // a rail needs at least an entry and an exit

    auto rod = [&](const glm::vec3& a, const glm::vec3& b) {
        out.push_back(RenderItem{MeshId::Cylinder, cylinderBetween(a, b, rodRadius_),
                                 kRailColor, kRailGlow, 1.0f});
    };
    auto joint = [&](const glm::vec3& p, const glm::vec3& color, const glm::vec3& glow) {
        glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), p), glm::vec3(rodRadius_ * 1.4f));
        out.push_back(RenderItem{MeshId::Sphere, m, color, glow, 1.0f});
    };

    const glm::vec3 down(0.0f, -bottomDrop_, 0.0f);

    // Three rods per segment: bottom (under the ball) + left + right.
    for (size_t i = 0; i + 1 < nodes_.size(); ++i) {
        const glm::vec3& a = nodes_[i];
        const glm::vec3& b = nodes_[i + 1];
        glm::vec3 s = sideDir(a, b) * sideOffset_;

        rod(a + down, b + down);     // bottom rail
        rod(a + s, b + s);           // right rail
        rod(a - s, b - s);           // left rail
    }

    // Smooth the joints + colour the entry (green) / exit (orange) ends.
    for (size_t i = 0; i < nodes_.size(); ++i) {
        glm::vec3 col = kRailColor, glow = kRailGlow;
        if (i == 0) { col = kEntryColor; glow = kEntryColor * 0.5f; }
        else if (i + 1 == nodes_.size()) { col = kExitColor; glow = kExitColor * 0.5f; }

        joint(nodes_[i] + down, col, glow);
        // Side joints use the adjacent segment's side direction.
        size_t seg = (i + 1 < nodes_.size()) ? i : i - 1;
        glm::vec3 s = sideDir(nodes_[seg], nodes_[seg + 1]) * sideOffset_;
        joint(nodes_[i] + s, col, glow);
        joint(nodes_[i] - s, col, glow);
    }
}

void Rail::serialize(IArchive& ar) {
    ar.field("nodes", nodes_);
    ar.field("rodRadius", rodRadius_);
    ar.field("sideOffset", sideOffset_);
    ar.field("bottomDrop", bottomDrop_);
    ar.field("travelSpeed", travelSpeed_);
    ar.field("captureRadius", captureRadius_);
}

std::unique_ptr<IBoardElement> Rail::clone() const {
    return std::make_unique<Rail>(*this);
}

} // namespace pinball::domain
