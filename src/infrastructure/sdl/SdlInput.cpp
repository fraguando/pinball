#include "infrastructure/sdl/SdlInput.hpp"

#include <SDL.h>

namespace pinball::infra {

app::InputFrame SdlInput::poll() {
    app::InputFrame f;

    // Event-driven edges (wheel, discrete key presses, quit).
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                f.quit = true;
                break;
            case SDL_MOUSEWHEEL:
                f.scroll += static_cast<float>(e.wheel.y);
                break;
            case SDL_KEYDOWN:
                if (e.key.repeat) break;
                switch (e.key.keysym.sym) {
                    case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
                    case SDLK_5: case SDLK_6: case SDLK_7:
                        f.toolHotkey = e.key.keysym.sym - SDLK_1 + 1;
                        break;
                    case SDLK_DELETE:
                    case SDLK_BACKSPACE:
                        f.deletePressed = true;
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        f.enterPressed = true;
                        break;
                    case SDLK_SPACE:
                        f.launchPressed = true;
                        break;
                    case SDLK_TAB:
                    case SDLK_p:
                        f.togglePlayPressed = true;
                        break;
                    case SDLK_ESCAPE:
                        f.escapePressed = true;
                        break;
                    case SDLK_s:
                        if (e.key.keysym.mod & KMOD_CTRL) f.savePressed = true;
                        break;
                    case SDLK_o:
                        if (e.key.keysym.mod & KMOD_CTRL) f.loadPressed = true;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    // Polled state (held keys/buttons + mouse position).
    int mx = 0, my = 0;
    Uint32 buttons = SDL_GetMouseState(&mx, &my);
    glm::vec2 mouse(static_cast<float>(mx), static_cast<float>(my));
    f.mousePos = mouse;
    f.mouseDelta = firstPoll_ ? glm::vec2(0.0f) : mouse - prevMouse_;

    bool left = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    bool right = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    f.leftDown = left;
    f.rightDown = right;
    f.leftPressed = left && !prevLeft_;
    f.leftReleased = !left && prevLeft_;
    f.rightPressed = right && !prevRight_;

    const Uint8* ks = SDL_GetKeyboardState(nullptr);
    f.leftFlipper = ks[SDL_SCANCODE_LEFT] != 0;
    f.rightFlipper = ks[SDL_SCANCODE_RIGHT] != 0;
    f.rotateLeft = ks[SDL_SCANCODE_Q] != 0;
    f.rotateRight = ks[SDL_SCANCODE_E] != 0;
    f.ctrl = (ks[SDL_SCANCODE_LCTRL] | ks[SDL_SCANCODE_RCTRL]) != 0;

    bool space = ks[SDL_SCANCODE_SPACE] != 0;
    f.launchHeld = space;
    f.launchReleased = !space && prevSpace_;

    prevLeft_ = left;
    prevRight_ = right;
    prevSpace_ = space;
    prevMouse_ = mouse;
    firstPoll_ = false;
    return f;
}

} // namespace pinball::infra
