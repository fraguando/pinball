// Composition root: creates the SDL/OpenGL window and the concrete adapters,
// wires them into the AppController through the application ports (dependency
// inversion), and runs the main loop.
#include <SDL.h>

#include "application/AppController.hpp"
#include "infrastructure/gl/GlLoader.hpp"
#include "infrastructure/gl/GlRenderer.hpp"
#include "infrastructure/persistence/JsonLevelRepository.hpp"
#include "infrastructure/sdl/SdlClock.hpp"
#include "infrastructure/sdl/SdlInput.hpp"

#include <algorithm>
#include <cstdio>

int main(int /*argc*/, char* /*argv*/[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow(
        "3D Pinball Designer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext ctx = SDL_GL_CreateContext(window);
    if (!ctx) {
        std::fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_SetSwapInterval(1);

    if (!pinball::gl::loadGlFunctions()) {
        std::fprintf(stderr, "Failed to load OpenGL functions\n");
        return 1;
    }

    pinball::gl::GlRenderer renderer("assets");
    if (!renderer.init()) {
        std::fprintf(stderr, "Renderer init failed (shaders missing?)\n");
        return 1;
    }

    pinball::infra::SdlClock clock;
    pinball::infra::SdlInput input;
    pinball::infra::JsonLevelRepository repo;
    pinball::app::AppController app(clock, repo);

    const glm::vec3 clearColor(0.04f, 0.05f, 0.08f);
    double last = clock.now();
    bool running = true;
    while (running) {
        pinball::app::InputFrame frame = input.poll();
        if (frame.quit) running = false;

        double t = clock.now();
        float dt = static_cast<float>(t - last);
        last = t;
        dt = std::clamp(dt, 0.0f, 0.033f);

        int w = 0, h = 0;
        SDL_GL_GetDrawableSize(window, &w, &h);

        renderer.beginFrame(w, h, clearColor);
        app.frame(frame, dt, renderer);
        renderer.endFrame();

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
