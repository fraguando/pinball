#include "domain/physics/Collision3D.hpp"
#include "domain/math/Geometry.hpp"

#include <algorithm>
#include <cmath>

namespace pinball::domain {

namespace {

Contact noHit() { return Contact{}; }

void stampMaterial(Contact& c, const CollisionShape& s) {
    c.restitution = s.restitution;
    c.surfaceVel = s.surfaceVel;
    c.bonusSpeed = s.bonusSpeed;
    c.scoreOnHit = s.scoreOnHit;
}

Contact fromClosestPoint(const glm::vec3& center, float radius,
                         const glm::vec3& closest, const CollisionShape& s) {
    glm::vec3 d = center - closest;
    float dist = glm::length(d);
    Contact c;
    stampMaterial(c, s);
    if (dist > radius) return c; // no overlap
    c.hit = true;
    if (dist > 1e-6f) {
        c.normal = d / dist;
    } else {
        c.normal = glm::vec3(0, 1, 0); // degenerate: push up
    }
    c.penetration = radius - dist;
    return c;
}

} // namespace

Contact collideSphere(const glm::vec3& center, float radius,
                      const CollisionShape& shape) {
    switch (shape.kind) {
        case ShapeKind::Plane: {
            float signedDist = glm::dot(center - shape.a, shape.normal);
            Contact c;
            stampMaterial(c, shape);
            if (signedDist > radius) return c;
            c.hit = true;
            c.normal = shape.normal;
            c.penetration = radius - signedDist;
            return c;
        }
        case ShapeKind::Sphere: {
            glm::vec3 d = center - shape.a;
            float dist = glm::length(d);
            Contact c;
            stampMaterial(c, shape);
            float sum = radius + shape.radius;
            if (dist > sum) return c;
            c.hit = true;
            c.normal = dist > 1e-6f ? d / dist : glm::vec3(0, 1, 0);
            c.penetration = sum - dist;
            return c;
        }
        case ShapeKind::Capsule: {
            glm::vec3 closest = closestPointOnSegment(center, shape.a, shape.b);
            // Treat the capsule as a swept sphere of `shape.radius`.
            return fromClosestPoint(center, radius + shape.radius, closest, shape);
        }
        case ShapeKind::Box: {
            // Transform the sphere center into box-local space (rot columns are
            // the box axes; rot is orthonormal so transpose == inverse).
            glm::vec3 local = glm::transpose(shape.rot) * (center - shape.a);
            glm::vec3 clamped = glm::clamp(local, -shape.halfExtents, shape.halfExtents);
            bool inside = (clamped == local);
            Contact c;
            stampMaterial(c, shape);
            if (inside) {
                // Center is inside the box: eject along the least-penetration
                // face with an outward-pointing normal.
                glm::vec3 dist = shape.halfExtents - glm::abs(local);
                int axis = 0;
                if (dist.y < dist.x) axis = 1;
                if (dist.z < dist[axis]) axis = 2;
                glm::vec3 localN(0.0f);
                localN[axis] = local[axis] >= 0 ? 1.0f : -1.0f;
                c.hit = true;
                c.normal = glm::normalize(shape.rot * localN);
                c.penetration = dist[axis] + radius;
                return c;
            }
            glm::vec3 closestWorld = shape.a + shape.rot * clamped;
            return fromClosestPoint(center, radius, closestWorld, shape);
        }
    }
    return noHit();
}

} // namespace pinball::domain
