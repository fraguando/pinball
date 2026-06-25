#pragma once
#include "infrastructure/gl/GlLoader.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace pinball::gl {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

// An indexed triangle mesh stored in GPU buffers (VAO/VBO/EBO).
class Mesh {
public:
    Mesh() = default;
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void upload(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices);
    void draw() const;

private:
    void release();

    GLuint vao_{0};
    GLuint vbo_{0};
    GLuint ebo_{0};
    GLsizei indexCount_{0};
};

} // namespace pinball::gl
