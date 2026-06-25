#include "GlLoader.hpp"

#include <SDL.h>

#include <cstdio>

// Definitions of the global function pointers (initialised to null).
#define GLFN(type, name) type name = nullptr;
#include "GlFunctions.inl"
#undef GLFN

namespace pinball::gl {

bool loadGlFunctions() {
    bool ok = true;

    auto load = [&ok](const char* name) -> void* {
        void* p = SDL_GL_GetProcAddress(name);
        if (!p) {
            std::fprintf(stderr, "[gl] failed to load function: %s\n", name);
            ok = false;
        }
        return p;
    };

#define GLFN(type, name) name = reinterpret_cast<type>(load(#name));
#include "GlFunctions.inl"
#undef GLFN

    return ok;
}

} // namespace pinball::gl
