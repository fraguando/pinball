#include "application/simulation/PinballSimulation.hpp"

#include "domain/elements/Bumper.hpp"
#include "domain/elements/Spawn.hpp"
#include "domain/physics/Collision3D.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace pinball::app {

using namespace pinball::domain;

namespace {

// Tilted gravity: presses the ball onto the floor (-Y) and pulls it hard toward
// the front/drain (+Z). Strong values give the lively, fast pinball feel.
const glm::vec3 kGravity{0.0f, -45.0f, 18.0f};
constexpr float kLinearDamping = 0.04f;  // low rolling resistance => keeps speed
constexpr float kMaxSpeed = 130.0f;
constexpr float kFriction = 0.04f;       // tangential friction on bounces
constexpr float kChargeTime = 0.55f;     // seconds to fully charge the spring
constexpr float kCaptureCooldown = 0.6f; // seconds before a hole can re-capture

// Sample a polyline by arc length. Writes the point and (normalized) forward
// tangent at distance `d`. Returns true while still on the path.
bool samplePolyline(const std::vector<glm::vec3>& nodes, float d,
                    glm::vec3& point, glm::vec3& tangent) {
    if (nodes.size() < 2) return false;
    for (size_t i = 0; i + 1 < nodes.size(); ++i) {
        glm::vec3 seg = nodes[i + 1] - nodes[i];
        float len = glm::length(seg);
        if (d <= len || i + 2 == nodes.size()) {
            glm::vec3 dir = len > 1e-6f ? seg / len : glm::vec3(0, 0, -1);
            if (d > len) { point = nodes[i + 1]; tangent = dir; return false; }
            point = nodes[i] + dir * d;
            tangent = dir;
            return true;
        }
        d -= len;
    }
    point = nodes.back();
    tangent = glm::vec3(0, 0, -1);
    return false;
}

} // namespace

PinballSimulation::PinballSimulation(const domain::Board& board, IClock& clock)
    : board_(board), clock_(clock) {
    for (const auto& e : board_.elements()) {
        if (auto* s = dynamic_cast<Spawn*>(e.get())) { spawn_ = s; break; }
    }
    respawn();
}

void PinballSimulation::respawn() {
    ball_.position = board_.spawnPoint();
    ball_.velocity = glm::vec3(0.0f);
    state_ = State::Launch;
    captureZone_ = nullptr;
    guide_ = nullptr;
    charge_ = 0.0f;
    if (spawn_) spawn_->setCompression(0.0f);
}

void PinballSimulation::update(float dt, bool leftFlipper, bool rightFlipper,
                               bool launchHeld, bool launchReleased) {
    // Drive controllable elements (flippers) and fade bumper lights every frame.
    InputState in;
    in.leftFlipper = leftFlipper;
    in.rightFlipper = rightFlipper;
    in.dt = dt;
    for (const auto& e : board_.elements()) {
        if (auto* c = dynamic_cast<IControllable*>(e.get())) c->applyInput(in);
        if (auto* b = dynamic_cast<Bumper*>(e.get())) b->decay(dt);
    }

    switch (state_) {
        case State::Launch: {
            ball_.position = board_.spawnPoint();
            // Charge the spring while the launch key is held; fire on release.
            if (launchHeld) charge_ = std::min(1.0f, charge_ + dt / kChargeTime);
            if (spawn_) spawn_->setCompression(charge_);
            if (launchReleased) {
                ball_.velocity = spawn_ ? spawn_->launchVelocity(charge_)
                                        : board_.launchVelocity();
                if (spawn_) spawn_->setCompression(0.0f);
                charge_ = 0.0f;
                state_ = State::Free;
            }
            break;
        }
        case State::Free: {
            // Substep so a fast ball can't tunnel through thin walls/flippers.
            float speed = glm::length(ball_.velocity);
            int steps = std::clamp(
                static_cast<int>(std::ceil(speed * dt / (ball_.radius * 0.5f))), 1, 8);
            float sub = dt / steps;
            for (int i = 0; i < steps; ++i) {
                integrate(sub);
                resolveCollisions();
                flashBumpers();
                if (checkTransitions()) break;
            }
            break;
        }
        case State::Captured:
            if (captureZone_ && clock_.now() >= releaseTime_) {
                ball_.velocity = captureZone_->ejectVelocity();
                ball_.position += glm::vec3(0, 0.15f, 0);
                captureCooldownUntil_ = clock_.now() + kCaptureCooldown;
                state_ = State::Free;
                captureZone_ = nullptr;
            }
            break;
        case State::Guided: {
            const auto& nodes = guide_->pathNodes();
            guideDistance_ += guideSpeed_ * dt;
            glm::vec3 p, tan;
            bool onPath = samplePolyline(nodes, guideDistance_, p, tan);
            ball_.position = p;
            ball_.velocity = tan * guideSpeed_;
            if (!onPath) {
                state_ = State::Free; // released along the exit tangent (airtime)
                guide_ = nullptr;
            }
            break;
        }
        case State::Lost:
            respawn();
            break;
    }
}

void PinballSimulation::integrate(float dt) {
    ball_.velocity += kGravity * dt;
    ball_.velocity *= std::max(0.0f, 1.0f - kLinearDamping * dt);
    float speed = glm::length(ball_.velocity);
    if (speed > kMaxSpeed) ball_.velocity *= kMaxSpeed / speed;
    ball_.position += ball_.velocity * dt;
}

void PinballSimulation::resolveCollisions() {
    std::vector<CollisionShape> shapes;
    board_.appendTableShapes(shapes);
    for (const auto& e : board_.elements()) e->appendCollisionShapes(shapes);

    // A few iterations let the ball settle in tight corners.
    for (int iter = 0; iter < 4; ++iter) {
        for (const auto& s : shapes) {
            Contact c = collideSphere(ball_.position, ball_.radius, s);
            if (!c.hit) continue;

            ball_.position += c.normal * c.penetration;

            glm::vec3 rel = ball_.velocity - c.surfaceVel;
            float vn = glm::dot(rel, c.normal);
            if (vn < 0.0f) {
                rel -= (1.0f + c.restitution) * vn * c.normal;
                glm::vec3 vt = rel - glm::dot(rel, c.normal) * c.normal;
                rel -= kFriction * vt;
                if (c.bonusSpeed > 0.0f) rel += c.normal * c.bonusSpeed;
                ball_.velocity = rel + c.surfaceVel;
                score_ += c.scoreOnHit;
            }
        }
    }
}

bool PinballSimulation::checkTransitions() {
    // Capture zones (holes) — suppressed briefly after an eject.
    if (clock_.now() >= captureCooldownUntil_) {
        for (const auto& e : board_.elements()) {
            if (auto* zone = dynamic_cast<ICaptureZone*>(e.get())) {
                if (zone->shouldCapture(ball_)) {
                    captureZone_ = zone;
                    ball_.position = zone->capturePoint();
                    ball_.velocity = glm::vec3(0.0f);
                    releaseTime_ = clock_.now() + zone->holdSeconds();
                    state_ = State::Captured;
                    return true;
                }
            }
        }
    }

    // Guide paths (rails): latch on near the entry node.
    for (const auto& e : board_.elements()) {
        if (auto* path = dynamic_cast<IGuidePath*>(e.get())) {
            const auto& nodes = path->pathNodes();
            if (nodes.size() < 2) continue;
            if (glm::distance(ball_.position, nodes.front()) <= path->captureRadius()) {
                guide_ = path;
                guideDistance_ = 0.0f;
                guideSpeed_ = std::max(glm::length(ball_.velocity), path->travelSpeed());
                state_ = State::Guided;
                return true;
            }
        }
    }

    if (board_.isDrained(ball_.position)) {
        state_ = State::Lost;
        return true;
    }
    return false;
}

void PinballSimulation::flashBumpers() {
    for (const auto& e : board_.elements()) {
        auto* b = dynamic_cast<Bumper*>(e.get());
        if (!b) continue;
        glm::vec3 p = b->position();
        glm::vec2 d(ball_.position.x - p.x, ball_.position.z - p.z);
        if (glm::length(d) <= b->radius() + ball_.radius + 0.15f) b->triggerFlash();
    }
}

void PinballSimulation::collectRenderItems(std::vector<domain::RenderItem>& out) const {
    board_.appendTableRenderItems(out);
    for (const auto& e : board_.elements()) e->appendRenderItems(out);

    glm::mat4 m = glm::translate(glm::mat4(1.0f), ball_.position);
    m = glm::scale(m, glm::vec3(ball_.radius));
    out.push_back(domain::RenderItem{domain::MeshId::Sphere, m, glm::vec3(0.9f, 0.92f, 1.0f),
                                     glm::vec3(0.15f, 0.16f, 0.2f), 1.0f});
}

} // namespace pinball::app
