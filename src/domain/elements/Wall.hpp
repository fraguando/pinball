#pragma once
#include "domain/elements/IBoardElement.hpp"

namespace pinball::domain {

// A static rectangular block the ball bounces off. Sits on the floor; can be
// rotated about the up axis.
class Wall final : public IBoardElement {
public:
    ElementType type() const override { return ElementType::Wall; }

    glm::vec3 position() const override { return position_; }
    void setPosition(const glm::vec3& p) override { position_ = p; }

    void rotate(float deltaRadians) override { yaw_ += deltaRadians; }
    bool raycastHit(const Ray& ray, float& t) const override;
    void appendCollisionShapes(std::vector<CollisionShape>& out) const override;
    void appendRenderItems(std::vector<RenderItem>& out) const override;
    void serialize(IArchive& ar) override;
    std::unique_ptr<IBoardElement> clone() const override;

    void setYaw(float yaw) { yaw_ = yaw; }
    float yaw() const { return yaw_; }
    void setLength(float l) { length_ = l; }

    // Size + orient the wall to span between two floor points (drag-to-create).
    void setFromEndpoints(const glm::vec3& a, const glm::vec3& b);

private:
    glm::vec3 position_{0.0f}; // center on the floor (y = base)
    float length_{4.0f};       // along local X
    float thickness_{0.5f};    // along local Z
    float height_{1.2f};       // along Y
    float yaw_{0.0f};
};

} // namespace pinball::domain
