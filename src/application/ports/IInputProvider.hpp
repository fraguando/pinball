#pragma once
#include <glm/glm.hpp>

namespace pinball::app {

// Per-frame snapshot of player input, already mapped from raw keys/mouse to the
// actions the game cares about. Filled by the SDL adapter each frame.
struct InputFrame {
    glm::vec2 mousePos{0.0f};   // pixels, origin top-left
    glm::vec2 mouseDelta{0.0f};
    float scroll{0.0f};

    bool leftDown{false};       // left mouse button held
    bool leftPressed{false};    // pressed this frame (edge)
    bool leftReleased{false};   // released this frame (edge)
    bool rightDown{false};      // right mouse button held
    bool rightPressed{false};

    bool ctrl{false};

    // Held: flipper controls.
    bool leftFlipper{false};
    bool rightFlipper{false};

    // Edges: discrete actions.
    int toolHotkey{0};          // 1..7 selected this frame, else 0
    bool deletePressed{false};
    bool savePressed{false};    // Ctrl+S
    bool loadPressed{false};    // Ctrl+O
    bool enterPressed{false};   // finalize rail / confirm
    bool launchPressed{false};  // Space (plunger)
    bool togglePlayPressed{false}; // Tab / P (enter play)
    bool escapePressed{false};  // back to editor

    bool quit{false};           // window close requested
};

// Input port. The implementation polls the OS/window each frame.
class IInputProvider {
public:
    virtual ~IInputProvider() = default;
    // Pump events and return this frame's mapped input snapshot.
    virtual InputFrame poll() = 0;
};

} // namespace pinball::app
