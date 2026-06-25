#pragma once
#include "domain/Ball.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace pinball::domain {

// Snapshot of player input relevant to controllable elements (the flippers).
struct InputState {
    bool leftFlipper{false};
    bool rightFlipper{false};
    float dt{0.0f};
};

// --- Capability interfaces (ISP) -----------------------------------------
// Elements opt into special behaviour by implementing one of these. The
// simulation discovers them via dynamic_cast, so its core loop stays generic
// and adding a new behaviour does not require touching unrelated elements.

// Reacts to player input (e.g. a flipper that swings while a key is held).
class IControllable {
public:
    virtual ~IControllable() = default;
    virtual void applyInput(const InputState& input) = 0;
};

// Captures the ball and holds it for a fixed time before ejecting it.
class ICaptureZone {
public:
    virtual ~ICaptureZone() = default;
    // True if the ball is over the zone and slow enough to be captured.
    virtual bool shouldCapture(const Ball& ball) const = 0;
    // World position the captured ball settles at.
    virtual glm::vec3 capturePoint() const = 0;
    virtual float holdSeconds() const = 0;
    // Velocity imparted to the ball when it is released.
    virtual glm::vec3 ejectVelocity() const = 0;
};

// Guides the ball along a fixed 3D path (a rail / slider).
class IGuidePath {
public:
    virtual ~IGuidePath() = default;
    virtual const std::vector<glm::vec3>& pathNodes() const = 0;
    // Radius around the entry node within which the ball is captured.
    virtual float captureRadius() const = 0;
    // Speed (units/sec) the ball travels along the path.
    virtual float travelSpeed() const = 0;
};

} // namespace pinball::domain
