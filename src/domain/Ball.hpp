#pragma once
#include <glm/glm.hpp>

namespace pinball::domain {

// The pinball: a sphere with a position and velocity in world space. The
// simulation owns its high-level state (free / captured / guided / lost).
struct Ball {
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    float radius{0.4f};
};

} // namespace pinball::domain
