#include "infrastructure/gl/GlRenderer.hpp"

#include "infrastructure/gl/Font5x7.hpp"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace pinball::gl {

namespace {
const glm::vec3 kLightDir = glm::normalize(glm::vec3(0.35f, 1.0f, 0.45f));
}

GlRenderer::GlRenderer(std::string assetsRoot) : assetsRoot_(std::move(assetsRoot)) {}

GlRenderer::~GlRenderer() {
    if (overlayVbo_) glDeleteBuffers(1, &overlayVbo_);
    if (overlayVao_) glDeleteVertexArrays(1, &overlayVao_);
}

bool GlRenderer::init() {
    if (!scene_.loadFromFiles(assetsRoot_ + "/shaders/basic.vert",
                              assetsRoot_ + "/shaders/basic.frag"))
        return false;
    if (!overlay_.loadFromFiles(assetsRoot_ + "/shaders/overlay.vert",
                                assetsRoot_ + "/shaders/overlay.frag"))
        return false;

    meshes_.build();

    glGenVertexArrays(1, &overlayVao_);
    glBindVertexArray(overlayVao_);
    glGenBuffers(1, &overlayVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, overlayVbo_);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(OverlayVertex),
                          reinterpret_cast<void*>(offsetof(OverlayVertex, pos)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(OverlayVertex),
                          reinterpret_cast<void*>(offsetof(OverlayVertex, color)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return true;
}

void GlRenderer::beginFrame(int widthPx, int heightPx, const glm::vec3& clearColor) {
    width_ = widthPx;
    height_ = heightPx;
    overlayVerts_.clear();

    glViewport(0, 0, width_, height_);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlRenderer::setCamera(const glm::mat4& view, const glm::mat4& proj,
                           const glm::vec3& cameraPos) {
    view_ = view;
    proj_ = proj;
    cameraPos_ = cameraPos;
}

void GlRenderer::draw(const domain::RenderItem& item) {
    scene_.use();
    glm::mat4 vp = proj_ * view_;
    scene_.setMat4("uViewProj", vp);
    scene_.setVec3("uLightDir", kLightDir);
    scene_.setVec3("uCameraPos", cameraPos_);
    scene_.setMat4("uModel", item.transform);
    scene_.setMat3("uNormalMat", glm::inverseTranspose(glm::mat3(item.transform)));
    scene_.setVec3("uColor", item.color);
    meshes_.get(item.mesh).draw();
}

void GlRenderer::draw(const std::vector<domain::RenderItem>& items) {
    scene_.use();
    glm::mat4 vp = proj_ * view_;
    scene_.setMat4("uViewProj", vp);
    scene_.setVec3("uLightDir", kLightDir);
    scene_.setVec3("uCameraPos", cameraPos_);
    for (const auto& item : items) {
        scene_.setMat4("uModel", item.transform);
        scene_.setMat3("uNormalMat", glm::inverseTranspose(glm::mat3(item.transform)));
        scene_.setVec3("uColor", item.color);
        meshes_.get(item.mesh).draw();
    }
}

void GlRenderer::pushQuad(float x, float y, float w, float h, const glm::vec4& color) {
    OverlayVertex a{{x, y}, color};
    OverlayVertex b{{x + w, y}, color};
    OverlayVertex c{{x + w, y + h}, color};
    OverlayVertex d{{x, y + h}, color};
    overlayVerts_.insert(overlayVerts_.end(), {a, b, c, a, c, d});
}

void GlRenderer::drawRect(float x, float y, float w, float h, const glm::vec4& color) {
    pushQuad(x, y, w, h, color);
}

void GlRenderer::drawText(const std::string& text, float x, float y, float pixelHeight,
                          const glm::vec4& color) {
    float cell = pixelHeight / static_cast<float>(Font5x7::kRows);
    float penX = x;
    for (char ch : text) {
        const auto& g = Font5x7::glyph(ch);
        for (int row = 0; row < Font5x7::kRows; ++row) {
            uint8_t bits = g[row];
            for (int col = 0; col < Font5x7::kCols; ++col) {
                if (bits & (1 << (Font5x7::kCols - 1 - col))) {
                    pushQuad(penX + col * cell, y + row * cell, cell, cell, color);
                }
            }
        }
        penX += (Font5x7::kCols + 1) * cell;
    }
}

float GlRenderer::measureText(const std::string& text, float pixelHeight) const {
    float cell = pixelHeight / static_cast<float>(Font5x7::kRows);
    return static_cast<float>(text.size()) * (Font5x7::kCols + 1) * cell;
}

void GlRenderer::flushOverlay() {
    if (overlayVerts_.empty()) return;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    overlay_.use();
    overlay_.setVec2("uScreenSize", glm::vec2((float)width_, (float)height_));

    glBindVertexArray(overlayVao_);
    glBindBuffer(GL_ARRAY_BUFFER, overlayVbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(overlayVerts_.size() * sizeof(OverlayVertex)),
                 overlayVerts_.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(overlayVerts_.size()));
    glBindVertexArray(0);
}

void GlRenderer::endFrame() {
    flushOverlay();
}

} // namespace pinball::gl
