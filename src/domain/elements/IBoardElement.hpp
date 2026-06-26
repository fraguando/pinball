#pragma once
#include "domain/elements/ElementType.hpp"
#include "domain/math/Geometry.hpp"
#include "domain/physics/CollisionShape.hpp"
#include "domain/render/RenderItem.hpp"
#include "domain/serialization/IArchive.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace pinball::domain {

// Base contract for every board element. Elements expose themselves as data
// (collision shapes + render items) so the simulation and renderer remain
// generic; special behaviour is added via the capability interfaces.
class IBoardElement {
public:
    virtual ~IBoardElement() = default;

    virtual ElementType type() const = 0;

    virtual glm::vec3 position() const = 0;
    virtual void setPosition(const glm::vec3& p) = 0;

    // Rotate the element about the up (Y) axis by `deltaRadians`. Elements that
    // have no meaningful orientation (round posts/holes) may ignore it.
    virtual void rotate(float deltaRadians) { (void)deltaRadians; }

    // Editor hit-testing: returns true (with hit distance) if the ray selects
    // this element. A bounding sphere is sufficient for editor picking.
    virtual bool raycastHit(const Ray& ray, float& t) const = 0;

    // Collision primitives the ball is tested against this step. Time-varying
    // elements (flippers) recompute these from their current state.
    virtual void appendCollisionShapes(std::vector<CollisionShape>& out) const = 0;

    // Visual representation, composed from procedural primitives.
    virtual void appendRenderItems(std::vector<RenderItem>& out) const = 0;

    // Read/write this element's fields through the archive (same call for both
    // directions). Type is handled by the factory, not here.
    virtual void serialize(IArchive& ar) = 0;

    // Deep copy (used when snapshotting a board into a simulation).
    virtual std::unique_ptr<IBoardElement> clone() const = 0;
};

} // namespace pinball::domain
