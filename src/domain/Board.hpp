#pragma once
#include "domain/elements/IBoardElement.hpp"
#include "domain/physics/CollisionShape.hpp"
#include "domain/render/RenderItem.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace pinball::domain {

// The authored playfield: a collection of elements plus the fixed table
// boundary (floor + perimeter walls with a drain gap at the front). The "front"
// edge (+Z) is where the drain and flippers live; gravity is tilted toward it.
class Board {
public:
    Board() = default;
    Board(const Board& other) { *this = other; }
    Board& operator=(const Board& other);
    Board(Board&&) noexcept = default;
    Board& operator=(Board&&) noexcept = default;

    IBoardElement* add(std::unique_ptr<IBoardElement> element);
    void remove(IBoardElement* element);
    void clear() { elements_.clear(); }

    const std::vector<std::unique_ptr<IBoardElement>>& elements() const { return elements_; }

    // Floor plane + boundary walls (with the central front drain gap).
    void appendTableShapes(std::vector<CollisionShape>& out) const;
    // Visual geometry for the table (floor + boundary walls).
    void appendTableRenderItems(std::vector<RenderItem>& out) const;

    // Where the ball starts / respawns (from a Spawn element if present).
    glm::vec3 spawnPoint() const;
    glm::vec3 launchVelocity() const;

    // True when the ball has fallen out through the front drain gap.
    bool isDrained(const glm::vec3& ballPos) const { return ballPos.z > frontZ_ + 1.0f; }

    float halfWidth() const { return halfWidth_; }
    float backZ() const { return backZ_; }
    float frontZ() const { return frontZ_; }
    float wallHeight() const { return wallHeight_; }

private:
    struct BoxDef {
        glm::vec3 center;
        glm::vec3 half;
    };
    // The four boundary walls (front split into two), shared by collision and
    // rendering so they never drift apart.
    std::vector<BoxDef> wallBoxes() const;

    std::vector<std::unique_ptr<IBoardElement>> elements_;

    float halfWidth_{8.0f};      // playfield spans X in [-halfWidth, +halfWidth]
    float backZ_{-14.0f};        // far edge
    float frontZ_{13.0f};        // near edge (drain)
    float drainHalfWidth_{2.4f}; // half-width of the front gap
    float wallHeight_{1.6f};
};

} // namespace pinball::domain
