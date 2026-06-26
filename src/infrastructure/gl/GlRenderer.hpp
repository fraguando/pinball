#pragma once
#include "application/ports/IRenderer.hpp"
#include "infrastructure/gl/GlLoader.hpp"
#include "infrastructure/gl/Mesh.hpp"
#include "infrastructure/gl/MeshLibrary.hpp"
#include "infrastructure/gl/Shader.hpp"

#include <string>
#include <vector>

namespace pinball::gl {

// OpenGL implementation of the renderer port: a lit 3D pass for board geometry
// plus a batched 2D overlay pass for the HUD/editor UI.
class GlRenderer final : public app::IRenderer {
public:
    explicit GlRenderer(std::string assetsRoot = "assets");
    ~GlRenderer() override;

    // Must be called once with a current GL context. Returns false on failure.
    bool init();

    void beginFrame(int widthPx, int heightPx, const glm::vec3& clearColor) override;
    void endFrame() override;

    void setCamera(const glm::mat4& view, const glm::mat4& proj,
                   const glm::vec3& cameraPos) override;
    void draw(const domain::RenderItem& item) override;
    void draw(const std::vector<domain::RenderItem>& items) override;

    void drawRect(float x, float y, float w, float h, const glm::vec4& color) override;
    void drawText(const std::string& text, float x, float y, float pixelHeight,
                  const glm::vec4& color) override;
    float measureText(const std::string& text, float pixelHeight) const override;

    int widthPx() const override { return width_; }
    int heightPx() const override { return height_; }

private:
    struct OverlayVertex {
        glm::vec2 pos;
        glm::vec4 color;
    };
    void pushQuad(float x, float y, float w, float h, const glm::vec4& color);
    void flushOverlay();
    void drawMesh(const domain::RenderItem& item);

    std::string assetsRoot_;
    Shader scene_;
    Shader overlay_;
    MeshLibrary meshes_;

    glm::mat4 view_{1.0f};
    glm::mat4 proj_{1.0f};
    glm::vec3 cameraPos_{0.0f};

    int width_{0};
    int height_{0};

    GLuint overlayVao_{0};
    GLuint overlayVbo_{0};
    std::vector<OverlayVertex> overlayVerts_;
};

} // namespace pinball::gl
