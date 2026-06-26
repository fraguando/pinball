#pragma once
#include "domain/elements/Capabilities.hpp"
#include "domain/elements/IBoardElement.hpp"

namespace pinball::domain {

// A slider / rail: a 3D polyline the ball latches onto at the entry node and
// rides to the exit node, then is released along the path tangent. Nodes may
// vary in height, so a rising rail launches the ball into the air.
//
// Rendered as an open wireform: three thin rods (one below the ball, one on
// each side) guide it, leaving the top open so the ball is always visible.
class Rail final : public IBoardElement, public IGuidePath {
public:
    Rail();

    ElementType type() const override { return ElementType::Rail; }

    glm::vec3 position() const override;
    void setPosition(const glm::vec3& p) override;

    void rotate(float deltaRadians) override;
    bool raycastHit(const Ray& ray, float& t) const override;
    void appendCollisionShapes(std::vector<CollisionShape>& out) const override;
    void appendRenderItems(std::vector<RenderItem>& out) const override;
    void serialize(IArchive& ar) override;
    std::unique_ptr<IBoardElement> clone() const override;

    const std::vector<glm::vec3>& pathNodes() const override { return nodes_; }
    float captureRadius() const override { return captureRadius_; }
    float travelSpeed() const override { return travelSpeed_; }

    void setNodes(std::vector<glm::vec3> nodes) { nodes_ = std::move(nodes); }
    void addNode(const glm::vec3& n) { nodes_.push_back(n); }
    bool valid() const { return nodes_.size() >= 2; }

private:
    std::vector<glm::vec3> nodes_;
    float rodRadius_{0.09f};   // thickness of each guide rod
    float sideOffset_{0.5f};   // half-gap between the two side rods
    float bottomDrop_{0.42f};  // how far the bottom rod sits below the ball line
    float travelSpeed_{16.0f}; // fast ride to match the livelier ball
    float captureRadius_{0.9f};
};

} // namespace pinball::domain
