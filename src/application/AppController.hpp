#pragma once
#include "application/editor/LevelEditor.hpp"
#include "application/ports/IClock.hpp"
#include "application/ports/IInputProvider.hpp"
#include "application/ports/ILevelRepository.hpp"
#include "application/ports/IRenderer.hpp"
#include "application/simulation/PinballSimulation.hpp"
#include "application/view/Camera.hpp"

#include <memory>
#include <string>
#include <vector>

namespace pinball::app {

// Top-level coordinator: owns the editor, camera and (while playing) the
// simulation, switches between Edit and Play, maps input to actions, and drives
// rendering through the renderer port.
class AppController {
public:
    AppController(IClock& clock, ILevelRepository& repo,
                  std::string levelPath = "assets/levels/board.json");

    // Handle one frame of input + logic and draw it. Called between the
    // renderer's beginFrame/endFrame.
    void frame(const InputFrame& in, float dt, IRenderer& r);

private:
    enum class Mode { Edit, Play };

    struct Button {
        float x, y, w, h;
        std::string label;
        int id;
        bool active;
    };

    void updateEdit(const InputFrame& in, float dt, IRenderer& r,
                    const std::vector<Button>& buttons);
    void updatePlay(const InputFrame& in, float dt);

    void drawScene(IRenderer& r);
    void drawEditHud(IRenderer& r, const std::vector<Button>& buttons);
    void drawPlayHud(IRenderer& r);

    std::vector<Button> paletteButtons(IRenderer& r) const;
    void handleButton(int id);
    bool worldPointAt(const glm::vec2& pixel, IRenderer& r, glm::vec3& out) const;
    void enterPlay();
    void enterEdit();
    void setStatus(const std::string& s);

    LevelEditor editor_;
    Camera camera_;
    IClock& clock_;
    ILevelRepository& repo_;
    std::string levelPath_;

    Mode mode_{Mode::Edit};
    std::unique_ptr<PinballSimulation> sim_;

    bool draggingWorld_{false};
    bool cursorValid_{false};
    glm::vec3 cursorWorld_{0.0f};

    std::string status_;
    double statusUntil_{0.0};
};

} // namespace pinball::app
