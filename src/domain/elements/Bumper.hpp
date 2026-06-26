#pragma once
#include "domain/elements/IBoardElement.hpp"

namespace pinball::domain {

// A round post that energetically bounces the ball away and scores on contact.
class Bumper final : public IBoardElement {
public:
    ElementType type() const override { return ElementType::Bumper; }

    glm::vec3 position() const override { return position_; }
    void setPosition(const glm::vec3& p) override { position_ = p; }

    bool raycastHit(const Ray& ray, float& t) const override;
    void appendCollisionShapes(std::vector<CollisionShape>& out) const override;
    void appendRenderItems(std::vector<RenderItem>& out) const override;
    void serialize(IArchive& ar) override;
    std::unique_ptr<IBoardElement> clone() const override;

    float radius() const { return radius_; }
    // Light up brightly (called by the simulation when the ball hits it).
    void triggerFlash() { flash_ = 1.0f; }
    // Fade the flash over time.
    void decay(float dt) { flash_ = flash_ > 0.0f ? flash_ - dt * 2.5f : 0.0f; }

private:
    glm::vec3 position_{0.0f}; // center on the floor
    float radius_{0.9f};
    float height_{0.9f};
    float flash_{0.0f};        // runtime hit-glow, not serialized
};

} // namespace pinball::domain
