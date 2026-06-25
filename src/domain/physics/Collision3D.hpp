#pragma once
#include "domain/physics/CollisionShape.hpp"

#include <glm/glm.hpp>

namespace pinball::domain {

// Result of testing a sphere against a collision shape.
struct Contact {
    bool hit{false};
    glm::vec3 normal{0, 1, 0}; // unit normal pointing from shape toward the sphere
    float penetration{0.0f};   // overlap depth along the normal (>= 0 when hit)
    glm::vec3 surfaceVel{0.0f};
    float restitution{0.5f};
    float bonusSpeed{0.0f};
    int scoreOnHit{0};
};

// Narrow-phase test of a sphere (center, radius) against one shape.
Contact collideSphere(const glm::vec3& center, float radius,
                      const CollisionShape& shape);

} // namespace pinball::domain
