#pragma once
#include "application/ports/IInputProvider.hpp"

#include <glm/glm.hpp>

namespace pinball::infra {

// IInputProvider backed by SDL events + keyboard/mouse state. Computes edge
// events (pressed/released this frame) from frame-to-frame deltas.
class SdlInput final : public app::IInputProvider {
public:
    app::InputFrame poll() override;

private:
    bool prevLeft_{false};
    bool prevRight_{false};
    bool prevSpace_{false};
    glm::vec2 prevMouse_{0.0f};
    bool firstPoll_{true};
};

} // namespace pinball::infra
