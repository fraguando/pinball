#pragma once
#include "domain/elements/IBoardElement.hpp"

namespace pinball::domain {

// Marks where the ball starts and respawns after draining. Launches up-table.
class Spawn final : public IBoardElement {
public:
    ElementType type() const override { return ElementType::Spawn; }

    glm::vec3 position() const override { return position_; }
    void setPosition(const glm::vec3& p) override { position_ = p; }

    bool raycastHit(const Ray& ray, float& t) const override;
    void appendCollisionShapes(std::vector<CollisionShape>&) const override {}
    void appendRenderItems(std::vector<RenderItem>& out) const override;
    void serialize(IArchive& ar) override;
    std::unique_ptr<IBoardElement> clone() const override;

    // Initial velocity given to the ball (a plunger kick up-table).
    glm::vec3 launchVelocity() const { return glm::vec3(0.0f, 0.0f, -16.0f); }

private:
    glm::vec3 position_{0.0f};
};

} // namespace pinball::domain
