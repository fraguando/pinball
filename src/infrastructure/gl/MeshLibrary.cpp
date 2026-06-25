#include "infrastructure/gl/MeshLibrary.hpp"

#include <glm/gtc/constants.hpp>

#include <cmath>

namespace pinball::gl {

namespace {

using domain::MeshId;

Mesh makeCube() {
    // Unit cube spanning [-0.5, 0.5]^3 with per-face normals.
    const glm::vec3 n[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    std::vector<Vertex> verts;
    std::vector<unsigned> idx;
    for (int f = 0; f < 6; ++f) {
        glm::vec3 normal = n[f];
        // Two tangent axes orthogonal to the normal.
        glm::vec3 t1 = (std::fabs(normal.y) < 0.9f) ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
        glm::vec3 u = glm::normalize(glm::cross(t1, normal));
        glm::vec3 v = glm::normalize(glm::cross(normal, u));
        glm::vec3 c = normal * 0.5f;
        unsigned base = static_cast<unsigned>(verts.size());
        verts.push_back({c - u * 0.5f - v * 0.5f, normal});
        verts.push_back({c + u * 0.5f - v * 0.5f, normal});
        verts.push_back({c + u * 0.5f + v * 0.5f, normal});
        verts.push_back({c - u * 0.5f + v * 0.5f, normal});
        idx.insert(idx.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }
    Mesh m; m.upload(verts, idx); return m;
}

Mesh makeQuad() {
    // Unit quad in the XZ plane (y = 0), normal +Y.
    std::vector<Vertex> verts = {
        {{-0.5f, 0, -0.5f}, {0, 1, 0}},
        {{ 0.5f, 0, -0.5f}, {0, 1, 0}},
        {{ 0.5f, 0,  0.5f}, {0, 1, 0}},
        {{-0.5f, 0,  0.5f}, {0, 1, 0}},
    };
    std::vector<unsigned> idx = {0, 2, 1, 0, 3, 2};
    Mesh m; m.upload(verts, idx); return m;
}

Mesh makeCylinder(int segments = 24) {
    // Radius 1, height 1 (y in [-0.5, 0.5]), aligned with +Y, with end caps.
    std::vector<Vertex> verts;
    std::vector<unsigned> idx;
    const float half = 0.5f;
    for (int i = 0; i < segments; ++i) {
        float a0 = (float)i / segments * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segments * glm::two_pi<float>();
        glm::vec3 d0(std::cos(a0), 0, std::sin(a0));
        glm::vec3 d1(std::cos(a1), 0, std::sin(a1));
        unsigned base = (unsigned)verts.size();
        verts.push_back({d0 * 1.0f + glm::vec3(0, -half, 0), d0});
        verts.push_back({d1 * 1.0f + glm::vec3(0, -half, 0), d1});
        verts.push_back({d1 * 1.0f + glm::vec3(0,  half, 0), d1});
        verts.push_back({d0 * 1.0f + glm::vec3(0,  half, 0), d0});
        idx.insert(idx.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }
    // Caps.
    for (int cap = 0; cap < 2; ++cap) {
        float y = cap == 0 ? -half : half;
        glm::vec3 nrm(0, cap == 0 ? -1.0f : 1.0f, 0);
        unsigned center = (unsigned)verts.size();
        verts.push_back({glm::vec3(0, y, 0), nrm});
        unsigned start = (unsigned)verts.size();
        for (int i = 0; i <= segments; ++i) {
            float a = (float)i / segments * glm::two_pi<float>();
            verts.push_back({glm::vec3(std::cos(a), y, std::sin(a)), nrm});
        }
        for (int i = 0; i < segments; ++i) {
            if (cap == 0)
                idx.insert(idx.end(), {center, start + i + 1u, start + i});
            else
                idx.insert(idx.end(), {center, start + i, start + i + 1u});
        }
    }
    Mesh m; m.upload(verts, idx); return m;
}

Mesh makeSphere(int stacks = 16, int slices = 24) {
    std::vector<Vertex> verts;
    std::vector<unsigned> idx;
    for (int i = 0; i <= stacks; ++i) {
        float phi = glm::pi<float>() * (float)i / stacks; // 0..pi
        for (int j = 0; j <= slices; ++j) {
            float theta = glm::two_pi<float>() * (float)j / slices;
            glm::vec3 p(std::sin(phi) * std::cos(theta),
                       std::cos(phi),
                       std::sin(phi) * std::sin(theta));
            verts.push_back({p, p});
        }
    }
    int cols = slices + 1;
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned a = i * cols + j;
            unsigned b = a + cols;
            idx.insert(idx.end(), {a, b, a + 1u, a + 1u, b, b + 1u});
        }
    }
    Mesh m; m.upload(verts, idx); return m;
}

} // namespace

void MeshLibrary::build() {
    meshes_[MeshId::Cube] = makeCube();
    meshes_[MeshId::Quad] = makeQuad();
    meshes_[MeshId::Cylinder] = makeCylinder();
    meshes_[MeshId::Sphere] = makeSphere();
}

const Mesh& MeshLibrary::get(domain::MeshId id) const {
    return meshes_.at(id);
}

} // namespace pinball::gl
