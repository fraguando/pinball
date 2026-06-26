#pragma once
#include "application/ports/IClock.hpp"
#include "domain/Ball.hpp"
#include "domain/Board.hpp"
#include "domain/elements/Capabilities.hpp"
#include "domain/render/RenderItem.hpp"

#include <vector>

namespace pinball::domain { class Spawn; }

namespace pinball::app {

// Runs the 3D physics for a snapshot of a board. The ball moves through four
// states: waiting on the spring plunger, free flight, captured in a hole, or
// guided along a rail. Gravity is tilted toward the front so the ball rolls
// toward the flippers/drain.
class PinballSimulation {
public:
    enum class State { Launch, Free, Captured, Guided, Lost };

    PinballSimulation(const domain::Board& board, IClock& clock);

    // Advance one frame. Flipper flags + spring controls come from player input.
    void update(float dt, bool leftFlipper, bool rightFlipper,
                bool launchHeld, bool launchReleased);

    // Reset ball to the spawn point (used on drain / new game).
    void respawn();

    State state() const { return state_; }
    int score() const { return score_; }
    float charge() const { return charge_; }
    const domain::Ball& ball() const { return ball_; }

    // All geometry for this frame: table + elements + ball.
    void collectRenderItems(std::vector<domain::RenderItem>& out) const;

private:
    void integrate(float dt);       // gravity + damping + move
    void resolveCollisions();       // bounce off all collision shapes
    bool checkTransitions();        // capture / guide / drain; true if state changed
    void flashBumpers();            // light up bumpers the ball is touching

    domain::Board board_;
    domain::Ball ball_;
    IClock& clock_;
    domain::Spawn* spawn_{nullptr}; // spring plunger, if the board has one

    State state_{State::Launch};
    int score_{0};
    float charge_{0.0f};            // spring charge in [0,1]

    // Capture (hole) state.
    domain::ICaptureZone* captureZone_{nullptr};
    double releaseTime_{0.0};
    double captureCooldownUntil_{0.0}; // ignore capture briefly after an eject

    // Guide (rail) state.
    domain::IGuidePath* guide_{nullptr};
    float guideDistance_{0.0f};
    float guideSpeed_{0.0f};
};

} // namespace pinball::app
