#pragma once

// Pull in the OpenGL headers via SDL. This provides the GL 1.1 declarations
// (linked from opengl32) plus the modern function-pointer typedefs (PFNGL...PROC)
// and enum constants, without declaring the modern function prototypes.
#include <SDL_opengl.h>

// Modern (>GL 1.1) function pointers, declared at global scope and named exactly
// like the GL functions (GLAD-style) so call sites read like normal OpenGL code.
// gl.h does not declare prototypes for these, so there is no clash.
#define GLFN(type, name) extern type name;
#include "GlFunctions.inl"
#undef GLFN

namespace pinball::gl {

// Resolves all modern GL function pointers through SDL_GL_GetProcAddress.
// Must be called once after a GL context is current. Returns false if any
// required function could not be resolved.
bool loadGlFunctions();

} // namespace pinball::gl
