#include "application/AppController.hpp"

#include "domain/math/Geometry.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace pinball::app {

using namespace pinball::domain;

namespace {
constexpr float kBarH = 40.0f;
constexpr float kBtnH = 28.0f;
constexpr float kTextH = 16.0f;
constexpr float kPad = 6.0f;

const glm::vec4 kBtnBg{0.15f, 0.17f, 0.22f, 0.85f};
const glm::vec4 kBtnActive{0.20f, 0.50f, 0.85f, 0.95f};
const glm::vec4 kBarBg{0.05f, 0.06f, 0.09f, 0.85f};
const glm::vec4 kWhite{0.95f, 0.97f, 1.0f, 1.0f};
const glm::vec4 kHint{0.65f, 0.7f, 0.8f, 1.0f};

enum ActionId { kPlay = 100, kSave = 101, kLoad = 102, kNew = 103, kDelete = 104 };

glm::vec3 toolColor(Tool t) {
    switch (t) {
        case Tool::Wall:    return {0.55f, 0.58f, 0.65f};
        case Tool::Bumper:  return {0.95f, 0.35f, 0.55f};
        case Tool::Flipper: return {0.95f, 0.85f, 0.25f};
        case Tool::Hole:    return {0.2f, 0.75f, 0.85f};
        case Tool::Rail:    return {0.7f, 0.75f, 0.85f};
        case Tool::Spawn:   return {0.5f, 0.9f, 1.0f};
        case Tool::Select:  return {1.0f, 1.0f, 1.0f};
    }
    return {1, 1, 1};
}
} // namespace

AppController::AppController(IClock& clock, ILevelRepository& repo, std::string levelPath)
    : clock_(clock), repo_(repo), levelPath_(std::move(levelPath)) {
    camera_.setTarget(glm::vec3(0.0f, 0.5f, -0.5f));
}

void AppController::frame(const InputFrame& in, float dt, IRenderer& r) {
    auto buttons = paletteButtons(r);

    if (mode_ == Mode::Edit)
        updateEdit(in, dt, r, buttons);
    else
        updatePlay(in, dt);

    drawScene(r);
    if (mode_ == Mode::Edit)
        drawEditHud(r, buttons);
    else
        drawPlayHud(r);
}

// --------------------------------------------------------------------------
// Editor
// --------------------------------------------------------------------------
void AppController::updateEdit(const InputFrame& in, float dt, IRenderer& r,
                               const std::vector<Button>& buttons) {
    (void)dt;
    // Camera control.
    if (in.rightDown) camera_.orbit(in.mouseDelta.x * 0.005f, -in.mouseDelta.y * 0.005f);
    if (in.scroll != 0.0f) camera_.zoom(in.scroll * 2.0f);

    // Tool hotkeys (1..7).
    if (in.toolHotkey >= 1 && in.toolHotkey <= kToolCount)
        editor_.setTool(static_cast<Tool>(in.toolHotkey - 1));

    if (in.deletePressed) editor_.deleteSelected();
    if (in.enterPressed) editor_.confirm();
    if (in.savePressed) handleButton(kSave);
    if (in.loadPressed) handleButton(kLoad);
    if (in.togglePlayPressed) { enterPlay(); return; }

    // Rotate the selected element with Q / E (held).
    float rot = (in.rotateLeft ? 1.0f : 0.0f) - (in.rotateRight ? 1.0f : 0.0f);
    if (rot != 0.0f) editor_.rotateSelected(rot * 1.8f * dt);

    // Track cursor world point for ghost preview / wall sizing.
    cursorValid_ = worldPointAt(in.mousePos, r, cursorWorld_);

    // Mouse press.
    if (in.leftPressed) {
        bool onUi = in.mousePos.y < kBarH;
        for (const auto& b : buttons) {
            if (in.mousePos.x >= b.x && in.mousePos.x <= b.x + b.w &&
                in.mousePos.y >= b.y && in.mousePos.y <= b.y + b.h) {
                handleButton(b.id);
                onUi = true;
                break;
            }
        }
        if (!onUi) {
            glm::vec3 wp;
            if (worldPointAt(in.mousePos, r, wp)) {
                if (editor_.tool() == Tool::Wall) {
                    // Begin dragging out a wall from this point.
                    wallDragging_ = true;
                    wallStart_ = wp;
                } else {
                    editor_.primaryClick(wp, camera_.screenToRay(in.mousePos, r.widthPx(),
                                                                 r.heightPx()));
                    draggingWorld_ = (editor_.tool() == Tool::Select && editor_.selected());
                }
            }
        }
    }

    // Mouse drag (move selection).
    if (in.leftDown && draggingWorld_ && editor_.tool() == Tool::Select) {
        glm::vec3 wp;
        if (worldPointAt(in.mousePos, r, wp)) editor_.dragSelectedTo(wp);
    }

    // Mouse release.
    if (in.leftReleased) {
        if (wallDragging_ && cursorValid_) editor_.placeWall(wallStart_, cursorWorld_);
        wallDragging_ = false;
        draggingWorld_ = false;
    }
}

void AppController::handleButton(int id) {
    if (id >= 0 && id < kToolCount) {
        editor_.setTool(static_cast<Tool>(id));
        return;
    }
    switch (id) {
        case kPlay: enterPlay(); break;
        case kSave: setStatus(repo_.save(editor_.board(), levelPath_) ? "SAVED" : "SAVE FAILED"); break;
        case kLoad:
            if (repo_.load(editor_.board(), levelPath_)) {
                editor_.resetEditingState();
                setStatus("LOADED");
            } else {
                setStatus("LOAD FAILED");
            }
            break;
        case kNew: editor_.clearBoard(); setStatus("NEW BOARD"); break;
        case kDelete: editor_.deleteSelected(); break;
        default: break;
    }
}

// --------------------------------------------------------------------------
// Play
// --------------------------------------------------------------------------
void AppController::updatePlay(const InputFrame& in, float dt) {
    if (in.escapePressed) { enterEdit(); return; }
    if (in.rightDown) camera_.orbit(in.mouseDelta.x * 0.005f, -in.mouseDelta.y * 0.005f);
    if (in.scroll != 0.0f) camera_.zoom(in.scroll * 2.0f);

    if (!sim_) return;
    sim_->update(dt, in.leftFlipper, in.rightFlipper, in.launchHeld, in.launchReleased);
}

void AppController::enterPlay() {
    sim_ = std::make_unique<PinballSimulation>(editor_.board(), clock_);
    mode_ = Mode::Play;
    setStatus("PRESS SPACE TO LAUNCH");
}

void AppController::enterEdit() {
    mode_ = Mode::Edit;
    sim_.reset();
    setStatus("");
}

// --------------------------------------------------------------------------
// Rendering
// --------------------------------------------------------------------------
void AppController::drawScene(IRenderer& r) {
    float aspect = static_cast<float>(r.widthPx()) / static_cast<float>(r.heightPx());
    r.setCamera(camera_.view(), camera_.projection(aspect), camera_.eye());

    std::vector<RenderItem> items;
    if (mode_ == Mode::Play && sim_) {
        sim_->collectRenderItems(items);
    } else {
        editor_.board().appendTableRenderItems(items);
        for (const auto& e : editor_.board().elements()) e->appendRenderItems(items);

        // Rail under construction.
        if (editor_.buildingRail()) {
            const auto& nodes = editor_.railNodes();
            for (const auto& n : nodes) {
                glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), n), glm::vec3(0.35f));
                items.push_back(RenderItem{MeshId::Sphere, m, glm::vec3(0.4f, 0.95f, 0.5f)});
            }
        }

        // Live wall preview while dragging out its size.
        if (wallDragging_ && cursorValid_) {
            glm::vec3 a = wallStart_, b = cursorWorld_;
            glm::vec3 mid = 0.5f * (a + b);
            float dx = b.x - a.x, dz = b.z - a.z;
            float len = std::max(0.6f, std::sqrt(dx * dx + dz * dz));
            float yaw = std::atan2(-dz, dx);
            glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(mid.x, 0.6f, mid.z));
            m = m * glm::mat4(rotationY(yaw));
            m = glm::scale(m, glm::vec3(len, 1.2f, 0.5f));
            items.push_back(RenderItem{MeshId::Cube, m, toolColor(Tool::Wall),
                                       glm::vec3(0.2f), 0.5f});
        } else if (cursorValid_ && editor_.tool() != Tool::Select) {
            // Translucent ghost at the cursor for placement tools.
            glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f),
                                                    cursorWorld_ + glm::vec3(0, 0.4f, 0)),
                                     glm::vec3(0.45f));
            items.push_back(RenderItem{MeshId::Sphere, m, toolColor(editor_.tool()),
                                       toolColor(editor_.tool()) * 0.4f, 0.55f});
        }

        // Selection marker.
        if (auto* sel = editor_.selected()) {
            glm::mat4 m = glm::scale(
                glm::translate(glm::mat4(1.0f), sel->position() + glm::vec3(0, 2.2f, 0)),
                glm::vec3(0.3f));
            items.push_back(RenderItem{MeshId::Sphere, m, glm::vec3(1.0f, 0.9f, 0.2f)});
        }
    }
    r.draw(items);
}

void AppController::drawEditHud(IRenderer& r, const std::vector<Button>& buttons) {
    r.drawRect(0, 0, static_cast<float>(r.widthPx()), kBarH, kBarBg);
    for (const auto& b : buttons) {
        r.drawRect(b.x, b.y, b.w, b.h, b.active ? kBtnActive : kBtnBg);
        r.drawText(b.label, b.x + 8, b.y + (b.h - kTextH) * 0.5f, kTextH, kWhite);
    }

    r.drawText("1-7 TOOLS   LCLICK PLACE   DRAG WALL TO SIZE   Q/E ROTATE   "
               "RDRAG ORBIT   WHEEL ZOOM   ENTER FINISH RAIL   DEL REMOVE   TAB PLAY",
               10, static_cast<float>(r.heightPx()) - 24, 13, kHint);

    if (clock_.now() < statusUntil_ && !status_.empty())
        r.drawText(status_, 10, kBarH + 8, 20, glm::vec4(0.5f, 0.95f, 0.6f, 1.0f));
}

void AppController::drawPlayHud(IRenderer& r) {
    r.drawRect(0, 0, static_cast<float>(r.widthPx()), kBarH, kBarBg);

    std::string score = sim_ ? "SCORE: " + std::to_string(sim_->score()) : "";
    r.drawText(score, 12, 10, 22, kWhite);

    std::string state;
    if (sim_) {
        switch (sim_->state()) {
            case PinballSimulation::State::Launch:
                state = "HOLD SPACE TO CHARGE SPRING   RELEASE TO LAUNCH";
                break;
            case PinballSimulation::State::Captured: state = "BALL CAPTURED"; break;
            case PinballSimulation::State::Guided:   state = "ON THE RAIL"; break;
            default: break;
        }
    }
    if (!state.empty()) {
        float w = r.measureText(state, 18);
        r.drawText(state, (r.widthPx() - w) * 0.5f, 12, 18, glm::vec4(1.0f, 0.85f, 0.4f, 1.0f));
    }

    // Spring charge meter while waiting to launch.
    if (sim_ && sim_->state() == PinballSimulation::State::Launch) {
        float charge = sim_->charge();
        float barW = 240.0f, barH = 14.0f;
        float bx = (r.widthPx() - barW) * 0.5f, by = kBarH + 10.0f;
        r.drawRect(bx, by, barW, barH, glm::vec4(0.1f, 0.1f, 0.13f, 0.85f));
        r.drawRect(bx, by, barW * charge, barH,
                   glm::vec4(1.0f, 0.55f + 0.4f * charge, 0.1f, 0.95f));
    }

    r.drawText("LEFT/RIGHT ARROWS FLIPPERS   HOLD+RELEASE SPACE LAUNCH   RDRAG ORBIT   ESC EDIT",
               10, static_cast<float>(r.heightPx()) - 24, 13, kHint);
}

// --------------------------------------------------------------------------
// Helpers
// --------------------------------------------------------------------------
std::vector<AppController::Button> AppController::paletteButtons(IRenderer& r) const {
    std::vector<Button> out;
    float x = 8.0f;
    const float y = (kBarH - kBtnH) * 0.5f;

    auto addBtn = [&](const std::string& label, int id, bool active) {
        float w = r.measureText(label, kTextH) + 16.0f;
        out.push_back(Button{x, y, w, kBtnH, label, id, active});
        x += w + kPad;
    };

    for (int i = 0; i < kToolCount; ++i) {
        Tool t = static_cast<Tool>(i);
        addBtn(toolName(t), i, editor_.tool() == t);
    }
    x += 16.0f;
    addBtn("PLAY", kPlay, false);
    addBtn("SAVE", kSave, false);
    addBtn("LOAD", kLoad, false);
    addBtn("NEW", kNew, false);
    addBtn("DELETE", kDelete, false);
    return out;
}

bool AppController::worldPointAt(const glm::vec2& pixel, IRenderer& r, glm::vec3& out) const {
    Ray ray = camera_.screenToRay(pixel, r.widthPx(), r.heightPx());
    float t;
    if (!rayPlane(ray, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), t)) return false;
    out = ray.origin + ray.dir * t;
    return true;
}

void AppController::setStatus(const std::string& s) {
    status_ = s;
    statusUntil_ = clock_.now() + 2.5;
}

} // namespace pinball::app
