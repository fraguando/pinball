#pragma once
#include "domain/elements/IBoardElement.hpp"

namespace pinball::domain {

// Marks where the ball starts/respawns. Rendered as a spring plunger: the
// simulation compresses it while the player charges, then releases the ball
// along the launch direction with a speed that scales with the charge.
class Spawn final : public IBoardElement {
public:
    ElementType type() const override { return ElementType::Spawn; }

    glm::vec3 position() const override { return position_; }
    void setPosition(const glm::vec3& p) override { position_ = p; }

    void rotate(float deltaRadians) override { yaw_ += deltaRadians; }
    bool raycastHit(const Ray& ray, float& t) const override;
    void appendCollisionShapes(std::vector<CollisionShape>&) const override {}
    void appendRenderItems(std::vector<RenderItem>& out) const override;
    void serialize(IArchive& ar) override;
    std::unique_ptr<IBoardElement> clone() const override;

    // Spring charge in [0,1]; drives the compression animation.
    void setCompression(float c);
    float compression() const { return compression_; }

    // Unit launch direction (up-table by default, rotated by yaw).
    glm::vec3 launchDir() const;
    // Launch velocity for a given charge in [0,1].
    glm::vec3 launchVelocity(float charge) const;
    glm::vec3 launchVelocity() const { return launchVelocity(0.0f); }

private:
    glm::vec3 position_{0.0f};
    float yaw_{0.0f};
    float compression_{0.0f}; // runtime, not serialized
    float minSpeed_{22.0f};
    float maxSpeed_{52.0f};
};

} // namespace pinball::domain
