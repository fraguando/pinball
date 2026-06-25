#include "application/simulation/PinballSimulation.hpp"

#include "domain/physics/Collision3D.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace pinball::app {

using namespace pinball::domain;

namespace {

// Tilted gravity: presses the ball onto the floor (-Y) and pulls it toward the
// front/drain (+Z), emulating an inclined pinball table.
const glm::vec3 kGravity{0.0f, -28.5f, 9.2f};
constexpr float kLinearDamping = 0.25f;  // rolling resistance per second
constexpr float kMaxSpeed = 60.0f;

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

float polylineLength(const std::vector<glm::vec3>& nodes) {
    float total = 0.0f;
    for (size_t i = 0; i + 1 < nodes.size(); ++i)
        total += glm::length(nodes[i + 1] - nodes[i]);
    return total;
}

} // namespace

PinballSimulation::PinballSimulation(const domain::Board& board, IClock& clock)
    : board_(board), clock_(clock) {
    respawn();
}

void PinballSimulation::respawn() {
    ball_.position = board_.spawnPoint();
    ball_.velocity = glm::vec3(0.0f);
    state_ = State::Launch;
    captureZone_ = nullptr;
    guide_ = nullptr;
}

void PinballSimulation::launch() {
    if (state_ != State::Launch) return;
    ball_.velocity = board_.launchVelocity();
    state_ = State::Free;
}

void PinballSimulation::update(float dt, bool leftFlipper, bool rightFlipper) {
    // Drive controllable elements (flippers) every step regardless of ball state.
    InputState in;
    in.leftFlipper = leftFlipper;
    in.rightFlipper = rightFlipper;
    in.dt = dt;
    for (const auto& e : board_.elements()) {
        if (auto* c = dynamic_cast<IControllable*>(e.get())) c->applyInput(in);
    }

    switch (state_) {
        case State::Launch:
            ball_.position = board_.spawnPoint();
            break;
        case State::Free:
            stepFree(dt);
            break;
        case State::Captured:
            if (captureZone_ && clock_.now() >= releaseTime_) {
                ball_.velocity = captureZone_->ejectVelocity();
                ball_.position += glm::vec3(0, 0.1f, 0);
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

void PinballSimulation::stepFree(float dt) {
    ball_.velocity += kGravity * dt;
    ball_.velocity *= std::max(0.0f, 1.0f - kLinearDamping * dt);
    float speed = glm::length(ball_.velocity);
    if (speed > kMaxSpeed) ball_.velocity *= kMaxSpeed / speed;

    ball_.position += ball_.velocity * dt;

    resolveCollisions();

    // Capture zones (holes).
    for (const auto& e : board_.elements()) {
        if (auto* zone = dynamic_cast<ICaptureZone*>(e.get())) {
            if (zone->shouldCapture(ball_)) {
                captureZone_ = zone;
                ball_.position = zone->capturePoint();
                ball_.velocity = glm::vec3(0.0f);
                releaseTime_ = clock_.now() + zone->holdSeconds();
                state_ = State::Captured;
                return;
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
                return;
            }
        }
    }

    if (board_.isDrained(ball_.position)) state_ = State::Lost;
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
                // Tangential friction.
                glm::vec3 vt = rel - glm::dot(rel, c.normal) * c.normal;
                rel -= 0.08f * vt;
                // Bumper "pop".
                if (c.bonusSpeed > 0.0f) rel += c.normal * c.bonusSpeed;
                ball_.velocity = rel + c.surfaceVel;
                score_ += c.scoreOnHit;
            }
        }
    }
}

void PinballSimulation::collectRenderItems(std::vector<domain::RenderItem>& out) const {
    board_.appendTableRenderItems(out);
    for (const auto& e : board_.elements()) e->appendRenderItems(out);

    glm::mat4 m = glm::translate(glm::mat4(1.0f), ball_.position);
    m = glm::scale(m, glm::vec3(ball_.radius));
    out.push_back(domain::RenderItem{domain::MeshId::Sphere, m, glm::vec3(0.9f, 0.92f, 1.0f)});
}

} // namespace pinball::app
