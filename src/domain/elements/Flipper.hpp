#pragma once
#include "domain/elements/Capabilities.hpp"
#include "domain/elements/IBoardElement.hpp"

namespace pinball::domain {

// A rotating bat ("hitting pad") that pivots about the up axis while its control
// key is held and falls back to rest otherwise. Transfers its swing velocity to
// the ball on contact.
class Flipper final : public IBoardElement, public IControllable {
public:
    ElementType type() const override { return ElementType::Flipper; }

    glm::vec3 position() const override { return pivot_; }
    void setPosition(const glm::vec3& p) override { pivot_ = p; }

    void rotate(float deltaRadians) override;
    bool raycastHit(const Ray& ray, float& t) const override;
    void appendCollisionShapes(std::vector<CollisionShape>& out) const override;
    void appendRenderItems(std::vector<RenderItem>& out) const override;
    void serialize(IArchive& ar) override;
    std::unique_ptr<IBoardElement> clone() const override;

    void applyInput(const InputState& input) override;

    // Configure as a left- or right-hand flipper (sets default angles + control).
    void setSide(bool left);
    bool controlledByLeft() const { return controlLeft_; }

private:
    glm::vec3 tip() const;

    glm::vec3 pivot_{0.0f};
    float length_{2.4f};
    float barRadius_{0.28f};
    float restAngle_{0.45f};    // radians, bar direction from +X in the XZ plane
    float activeAngle_{-0.55f};
    bool controlLeft_{true};

    // Runtime swing state (not serialized).
    float currentAngle_{0.45f};
    float angularVel_{0.0f};

    static constexpr float kHeight = 0.4f; // bar height above the floor
};

} // namespace pinball::domain
