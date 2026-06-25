#pragma once
#include "domain/render/RenderItem.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace pinball::app {

// Output port for all drawing. Implemented by the infrastructure (OpenGL) and
// consumed by the presentation layer, so views never touch OpenGL directly.
//
// 2D overlay coordinates are in pixels with the origin at the top-left.
class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void beginFrame(int widthPx, int heightPx, const glm::vec3& clearColor) = 0;
    virtual void endFrame() = 0;

    // --- 3D scene ---
    virtual void setCamera(const glm::mat4& view, const glm::mat4& proj,
                           const glm::vec3& cameraPos) = 0;
    virtual void draw(const domain::RenderItem& item) = 0;
    virtual void draw(const std::vector<domain::RenderItem>& items) = 0;

    // --- 2D overlay (HUD / editor UI) ---
    virtual void drawRect(float x, float y, float w, float h, const glm::vec4& color) = 0;
    virtual void drawText(const std::string& text, float x, float y, float pixelHeight,
                          const glm::vec4& color) = 0;
    virtual float measureText(const std::string& text, float pixelHeight) const = 0;

    virtual int widthPx() const = 0;
    virtual int heightPx() const = 0;
};

} // namespace pinball::app
