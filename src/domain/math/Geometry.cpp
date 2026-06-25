#include "domain/math/Geometry.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace pinball::domain {

bool rayPlane(const Ray& ray, const glm::vec3& planePoint,
              const glm::vec3& planeNormal, float& t) {
    float denom = glm::dot(planeNormal, ray.dir);
    if (std::fabs(denom) < 1e-6f) return false; // parallel
    t = glm::dot(planePoint - ray.origin, planeNormal) / denom;
    return t >= 0.0f;
}

bool raySphere(const Ray& ray, const glm::vec3& center, float radius, float& t) {
    glm::vec3 oc = ray.origin - center;
    float b = glm::dot(oc, ray.dir);
    float c = glm::dot(oc, oc) - radius * radius;
    float disc = b * b - c;
    if (disc < 0.0f) return false;
    float sq = std::sqrt(disc);
    float t0 = -b - sq;
    float t1 = -b + sq;
    if (t0 >= 0.0f) { t = t0; return true; }
    if (t1 >= 0.0f) { t = t1; return true; }
    return false;
}

glm::vec3 closestPointOnSegment(const glm::vec3& p, const glm::vec3& a,
                                const glm::vec3& b) {
    glm::vec3 ab = b - a;
    float len2 = glm::dot(ab, ab);
    if (len2 < 1e-12f) return a;
    float t = glm::dot(p - a, ab) / len2;
    t = std::clamp(t, 0.0f, 1.0f);
    return a + t * ab;
}

glm::mat4 cylinderBetween(const glm::vec3& a, const glm::vec3& b, float radius) {
    glm::vec3 axis = b - a;
    float len = glm::length(axis);
    glm::vec3 mid = 0.5f * (a + b);
    glm::mat4 m = glm::translate(glm::mat4(1.0f), mid);

    if (len > 1e-6f) {
        glm::vec3 dir = axis / len;
        glm::vec3 up(0, 1, 0);
        float d = glm::dot(up, dir);
        if (d < -0.9999f) {
            // Antiparallel: rotate 180 degrees about X.
            m = glm::rotate(m, glm::pi<float>(), glm::vec3(1, 0, 0));
        } else if (d < 0.9999f) {
            glm::vec3 rotAxis = glm::normalize(glm::cross(up, dir));
            float angle = std::acos(glm::clamp(d, -1.0f, 1.0f));
            m = glm::rotate(m, angle, rotAxis);
        }
    }
    m = glm::scale(m, glm::vec3(radius, len, radius));
    return m;
}

glm::mat3 rotationY(float radians) {
    float c = std::cos(radians), s = std::sin(radians);
    // Columns are the rotated basis vectors.
    return glm::mat3(glm::vec3(c, 0, -s), glm::vec3(0, 1, 0), glm::vec3(s, 0, c));
}

} // namespace pinball::domain
