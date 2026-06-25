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

private:
    glm::vec3 position_{0.0f}; // center on the floor
    float radius_{0.9f};
    float height_{0.9f};
};

} // namespace pinball::domain
