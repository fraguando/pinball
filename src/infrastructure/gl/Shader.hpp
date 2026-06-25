#pragma once
#include "infrastructure/gl/GlLoader.hpp"

#include <glm/glm.hpp>

#include <string>

namespace pinball::gl {

// Compiles and links a vertex+fragment program and exposes uniform setters.
class Shader {
public:
    Shader() = default;
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Loads and compiles from files. Returns false (and logs) on failure.
    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);

    void use() const;
    GLuint id() const { return program_; }

    void setMat4(const char* name, const glm::mat4& m) const;
    void setMat3(const char* name, const glm::mat3& m) const;
    void setVec3(const char* name, const glm::vec3& v) const;
    void setVec2(const char* name, const glm::vec2& v) const;

private:
    GLuint program_{0};
};

} // namespace pinball::gl
