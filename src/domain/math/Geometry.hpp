#pragma once
#include <glm/glm.hpp>

namespace pinball::domain {

// A ray in world space. `dir` is expected to be normalized.
struct Ray {
    glm::vec3 origin{0.0f};
    glm::vec3 dir{0.0f, 0.0f, -1.0f};
};

// Intersect a ray with an (infinite) plane defined by a point and a normal.
// Returns true and writes the hit distance to `t` if the ray hits the front
// or back of the plane at t >= 0.
bool rayPlane(const Ray& ray, const glm::vec3& planePoint,
              const glm::vec3& planeNormal, float& t);

// Intersect a ray with a sphere. Returns true and writes the nearest positive
// hit distance to `t`.
bool raySphere(const Ray& ray, const glm::vec3& center, float radius, float& t);

// Closest point to `p` on the segment [a, b].
glm::vec3 closestPointOnSegment(const glm::vec3& p, const glm::vec3& a,
                                const glm::vec3& b);

// Model matrix that maps a unit cylinder (radius 1, spanning y in [-0.5, 0.5],
// aligned with +Y) onto the segment [a, b] with the given radius. Used to draw
// bars, flippers and rail tubes from the shared cylinder mesh.
glm::mat4 cylinderBetween(const glm::vec3& a, const glm::vec3& b, float radius);

// Rotation matrix about the +Y (up) axis.
glm::mat3 rotationY(float radians);

} // namespace pinball::domain
