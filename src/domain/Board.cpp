#include "domain/Board.hpp"

#include "domain/elements/Spawn.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace pinball::domain {

Board& Board::operator=(const Board& other) {
    if (this == &other) return *this;
    halfWidth_ = other.halfWidth_;
    backZ_ = other.backZ_;
    frontZ_ = other.frontZ_;
    drainHalfWidth_ = other.drainHalfWidth_;
    wallHeight_ = other.wallHeight_;
    elements_.clear();
    elements_.reserve(other.elements_.size());
    for (const auto& e : other.elements_) elements_.push_back(e->clone());
    return *this;
}

IBoardElement* Board::add(std::unique_ptr<IBoardElement> element) {
    IBoardElement* raw = element.get();
    elements_.push_back(std::move(element));
    return raw;
}

void Board::remove(IBoardElement* element) {
    elements_.erase(
        std::remove_if(elements_.begin(), elements_.end(),
                       [element](const std::unique_ptr<IBoardElement>& e) {
                           return e.get() == element;
                       }),
        elements_.end());
}

std::vector<Board::BoxDef> Board::wallBoxes() const {
    const float h = wallHeight_;
    const float th = 0.5f; // wall half-thickness
    const float midZ = 0.5f * (backZ_ + frontZ_);
    const float halfDepth = 0.5f * (frontZ_ - backZ_);
    const float segLen = 0.5f * (halfWidth_ - drainHalfWidth_);
    const float segCenter = drainHalfWidth_ + segLen;

    return {
        // left, right
        {{-halfWidth_ - th, h * 0.5f, midZ}, {th, h * 0.5f, halfDepth}},
        {{halfWidth_ + th, h * 0.5f, midZ}, {th, h * 0.5f, halfDepth}},
        // back
        {{0, h * 0.5f, backZ_ - th}, {halfWidth_ + 2 * th, h * 0.5f, th}},
        // front split (drain gap in the middle)
        {{-segCenter, h * 0.5f, frontZ_ + th}, {segLen, h * 0.5f, th}},
        {{segCenter, h * 0.5f, frontZ_ + th}, {segLen, h * 0.5f, th}},
    };
}

void Board::appendTableShapes(std::vector<CollisionShape>& out) const {
    // Floor: gentle restitution so the ball rolls rather than bounces.
    out.push_back(CollisionShape::makePlane(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 0.15f));
    const glm::mat3 I(1.0f);
    for (const auto& w : wallBoxes())
        out.push_back(CollisionShape::makeBox(w.center, w.half, I));
}

void Board::appendTableRenderItems(std::vector<RenderItem>& out) const {
    // Floor slab.
    glm::mat4 floor = glm::scale(glm::mat4(1.0f), glm::vec3(2 * halfWidth_ + 1.5f, 1.0f,
                                                            (frontZ_ - backZ_) + 1.5f));
    floor[3] = glm::vec4(0.0f, -0.02f, 0.5f * (backZ_ + frontZ_), 1.0f);
    out.push_back(RenderItem{MeshId::Quad, floor, glm::vec3(0.10f, 0.16f, 0.13f)});

    // Boundary walls.
    for (const auto& w : wallBoxes()) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), w.center);
        m = glm::scale(m, w.half * 2.0f);
        out.push_back(RenderItem{MeshId::Cube, m, glm::vec3(0.30f, 0.33f, 0.40f)});
    }
}

glm::vec3 Board::spawnPoint() const {
    for (const auto& e : elements_) {
        if (e->type() == ElementType::Spawn) {
            return e->position() + glm::vec3(0, 0.4f, 0);
        }
    }
    // Default: front-right plunger lane.
    return glm::vec3(halfWidth_ - 1.2f, 0.4f, frontZ_ - 1.5f);
}

glm::vec3 Board::launchVelocity() const {
    for (const auto& e : elements_) {
        if (auto* spawn = dynamic_cast<const Spawn*>(e.get())) {
            return spawn->launchVelocity();
        }
    }
    return glm::vec3(0.0f, 0.0f, -16.0f);
}

} // namespace pinball::domain
