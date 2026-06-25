// X-macro list of the modern (>GL 1.1) OpenGL functions we load at runtime via
// SDL_GL_GetProcAddress. GL 1.1 functions (glClear, glViewport, glGenTextures,
// glDrawArrays, ...) are exported directly by opengl32 and used without loading.
//
// Used by GlLoader.hpp (declarations) and GlLoader.cpp (definitions + loading).

GLFN(PFNGLCREATESHADERPROC,            glCreateShader)
GLFN(PFNGLSHADERSOURCEPROC,            glShaderSource)
GLFN(PFNGLCOMPILESHADERPROC,           glCompileShader)
GLFN(PFNGLGETSHADERIVPROC,             glGetShaderiv)
GLFN(PFNGLGETSHADERINFOLOGPROC,        glGetShaderInfoLog)
GLFN(PFNGLDELETESHADERPROC,            glDeleteShader)
GLFN(PFNGLCREATEPROGRAMPROC,           glCreateProgram)
GLFN(PFNGLATTACHSHADERPROC,            glAttachShader)
GLFN(PFNGLLINKPROGRAMPROC,             glLinkProgram)
GLFN(PFNGLGETPROGRAMIVPROC,            glGetProgramiv)
GLFN(PFNGLGETPROGRAMINFOLOGPROC,       glGetProgramInfoLog)
GLFN(PFNGLUSEPROGRAMPROC,              glUseProgram)
GLFN(PFNGLDELETEPROGRAMPROC,           glDeleteProgram)

GLFN(PFNGLGETUNIFORMLOCATIONPROC,      glGetUniformLocation)
GLFN(PFNGLUNIFORM1IPROC,               glUniform1i)
GLFN(PFNGLUNIFORM1FPROC,               glUniform1f)
GLFN(PFNGLUNIFORM2FVPROC,              glUniform2fv)
GLFN(PFNGLUNIFORM3FVPROC,              glUniform3fv)
GLFN(PFNGLUNIFORM4FVPROC,              glUniform4fv)
GLFN(PFNGLUNIFORMMATRIX3FVPROC,        glUniformMatrix3fv)
GLFN(PFNGLUNIFORMMATRIX4FVPROC,        glUniformMatrix4fv)

GLFN(PFNGLGENVERTEXARRAYSPROC,         glGenVertexArrays)
GLFN(PFNGLBINDVERTEXARRAYPROC,         glBindVertexArray)
GLFN(PFNGLDELETEVERTEXARRAYSPROC,      glDeleteVertexArrays)
GLFN(PFNGLGENBUFFERSPROC,              glGenBuffers)
GLFN(PFNGLBINDBUFFERPROC,              glBindBuffer)
GLFN(PFNGLBUFFERDATAPROC,              glBufferData)
GLFN(PFNGLDELETEBUFFERSPROC,           glDeleteBuffers)
GLFN(PFNGLVERTEXATTRIBPOINTERPROC,     glVertexAttribPointer)
GLFN(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray)
