#pragma once
#include <glm/glm.hpp>

namespace pinball::domain {

// Reusable procedural meshes the renderer knows how to draw. Elements compose
// their appearance out of these primitives, so the domain describes *what* to
// draw as data without referencing OpenGL.
enum class MeshId {
    Sphere,
    Cube,
    Cylinder,
    Quad,
};

// One thing to draw: a unit mesh transformed into the world and tinted.
struct RenderItem {
    MeshId mesh{MeshId::Cube};
    glm::mat4 transform{1.0f};
    glm::vec3 color{1.0f};
};

} // namespace pinball::domain
