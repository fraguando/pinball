#include "infrastructure/sdl/SdlClock.hpp"

#include <SDL.h>

namespace pinball::infra {

SdlClock::SdlClock()
    : start_(SDL_GetPerformanceCounter()),
      freq_(static_cast<double>(SDL_GetPerformanceFrequency())) {}

double SdlClock::now() const {
    return static_cast<double>(SDL_GetPerformanceCounter() - start_) / freq_;
}

} // namespace pinball::infra
