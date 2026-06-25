#pragma once
#include "domain/elements/Capabilities.hpp"
#include "domain/elements/IBoardElement.hpp"

namespace pinball::domain {

// A cup the ball drops into. The simulation holds the ball here for
// `holdSeconds` then ejects it back into play.
class Hole final : public IBoardElement, public ICaptureZone {
public:
    ElementType type() const override { return ElementType::Hole; }

    glm::vec3 position() const override { return position_; }
    void setPosition(const glm::vec3& p) override { position_ = p; }

    bool raycastHit(const Ray& ray, float& t) const override;
    void appendCollisionShapes(std::vector<CollisionShape>& out) const override;
    void appendRenderItems(std::vector<RenderItem>& out) const override;
    void serialize(IArchive& ar) override;
    std::unique_ptr<IBoardElement> clone() const override;

    bool shouldCapture(const Ball& ball) const override;
    glm::vec3 capturePoint() const override;
    float holdSeconds() const override { return holdSeconds_; }
    glm::vec3 ejectVelocity() const override;

private:
    glm::vec3 position_{0.0f}; // center on the floor
    float radius_{0.9f};
    float holdSeconds_{2.0f};
};

} // namespace pinball::domain
