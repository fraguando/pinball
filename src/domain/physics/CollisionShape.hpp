#pragma once
#include <glm/glm.hpp>

namespace pinball::domain {

// A collision primitive the ball (a sphere) is tested against. Elements emit
// these each step; the simulation resolves the ball against all of them
// generically, so adding a new element never touches the physics core.
enum class ShapeKind {
    Plane,    // infinite half-space: a = point on plane, normal = outward normal
    Sphere,   // a = center, radius
    Capsule,  // segment [a, b] with radius (used by walls-as-bars and flippers)
    Box,      // oriented box: a = center, halfExtents, basis columns in `rot`
};

struct CollisionShape {
    ShapeKind kind{ShapeKind::Plane};

    glm::vec3 a{0.0f};            // plane point / sphere center / capsule p0 / box center
    glm::vec3 b{0.0f};            // capsule p1
    glm::vec3 normal{0, 1, 0};   // plane normal
    glm::vec3 halfExtents{1.0f}; // box half-extents (in box-local axes)
    glm::mat3 rot{1.0f};         // box orientation (columns = local axes)
    float radius{0.5f};          // sphere / capsule radius

    float restitution{0.5f};
    // Velocity of the contact surface at the contact point (e.g. a swinging
    // flipper). Added into the collision response to "kick" the ball.
    glm::vec3 surfaceVel{0.0f};
    // Extra outward speed added on contact (bumpers "pop" the ball away).
    float bonusSpeed{0.0f};
    // Points awarded the first time the ball touches this surface in a contact.
    int scoreOnHit{0};

    static CollisionShape makePlane(const glm::vec3& point, const glm::vec3& n,
                                    float restitution = 0.4f) {
        CollisionShape s;
        s.kind = ShapeKind::Plane;
        s.a = point;
        s.normal = glm::normalize(n);
        s.restitution = restitution;
        return s;
    }
    static CollisionShape makeSphere(const glm::vec3& center, float r,
                                     float restitution = 0.7f) {
        CollisionShape s;
        s.kind = ShapeKind::Sphere;
        s.a = center;
        s.radius = r;
        s.restitution = restitution;
        return s;
    }
    static CollisionShape makeCapsule(const glm::vec3& p0, const glm::vec3& p1,
                                      float r, float restitution = 0.5f) {
        CollisionShape s;
        s.kind = ShapeKind::Capsule;
        s.a = p0;
        s.b = p1;
        s.radius = r;
        s.restitution = restitution;
        return s;
    }
    static CollisionShape makeBox(const glm::vec3& center, const glm::vec3& half,
                                  const glm::mat3& rot, float restitution = 0.4f) {
        CollisionShape s;
        s.kind = ShapeKind::Box;
        s.a = center;
        s.halfExtents = half;
        s.rot = rot;
        s.restitution = restitution;
        return s;
    }
};

} // namespace pinball::domain
