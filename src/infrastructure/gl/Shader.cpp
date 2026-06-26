#include "infrastructure/gl/Shader.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

namespace pinball::gl {

namespace {

bool readFile(const std::string& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        std::fprintf(stderr, "[shader] cannot open %s\n", path.c_str());
        return false;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

GLuint compile(GLenum type, const std::string& src, const std::string& label) {
    GLuint sh = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(sh, 1, &c, nullptr);
    glCompileShader(sh);
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 1 ? len : 1);
        glGetShaderInfoLog(sh, len, nullptr, log.data());
        std::fprintf(stderr, "[shader] compile failed (%s):\n%s\n", label.c_str(), log.data());
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

} // namespace

Shader::~Shader() {
    if (program_) glDeleteProgram(program_);
}

bool Shader::loadFromFiles(const std::string& vertPath, const std::string& fragPath) {
    std::string vsrc, fsrc;
    if (!readFile(vertPath, vsrc) || !readFile(fragPath, fsrc)) return false;

    GLuint vs = compile(GL_VERTEX_SHADER, vsrc, vertPath);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsrc, fragPath);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);

    GLint ok = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 1 ? len : 1);
        glGetProgramInfoLog(program_, len, nullptr, log.data());
        std::fprintf(stderr, "[shader] link failed:\n%s\n", log.data());
        glDeleteProgram(program_);
        program_ = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program_ != 0;
}

void Shader::use() const { glUseProgram(program_); }

void Shader::setMat4(const char* name, const glm::mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(program_, name), 1, GL_FALSE, glm::value_ptr(m));
}
void Shader::setMat3(const char* name, const glm::mat3& m) const {
    glUniformMatrix3fv(glGetUniformLocation(program_, name), 1, GL_FALSE, glm::value_ptr(m));
}
void Shader::setVec3(const char* name, const glm::vec3& v) const {
    glUniform3fv(glGetUniformLocation(program_, name), 1, glm::value_ptr(v));
}
void Shader::setVec2(const char* name, const glm::vec2& v) const {
    glUniform2fv(glGetUniformLocation(program_, name), 1, glm::value_ptr(v));
}
void Shader::setFloat(const char* name, float v) const {
    glUniform1f(glGetUniformLocation(program_, name), v);
}

} // namespace pinball::gl
