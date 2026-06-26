#pragma once
#include "domain/Board.hpp"
#include "domain/elements/ElementFactory.hpp"
#include "domain/math/Geometry.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace pinball::app {

// Editor tools. The first is selection; the rest place a corresponding element.
enum class Tool { Select, Wall, Bumper, Flipper, Hole, Rail, Spawn };

constexpr int kToolCount = 7;
const char* toolName(Tool t);

// Owns the board being authored and all editing operations. Works purely in
// world coordinates; ray-casting/placement points are supplied by the view.
class LevelEditor {
public:
    LevelEditor();

    domain::Board& board() { return board_; }
    const domain::Board& board() const { return board_; }

    Tool tool() const { return tool_; }
    void setTool(Tool t);

    domain::IBoardElement* selected() const { return selected_; }

    // Left click at a floor point (with the ray for precise picking).
    void primaryClick(const glm::vec3& worldPoint, const domain::Ray& ray);
    // Drag the selected element to follow the cursor.
    void dragSelectedTo(const glm::vec3& worldPoint);
    // Create a wall spanning two floor points (drag-to-size).
    void placeWall(const glm::vec3& a, const glm::vec3& b);
    // Rotate the selected element about the up axis.
    void rotateSelected(float deltaRadians);
    // Finish the in-progress rail (Enter).
    void confirm();
    void deleteSelected();
    void clearBoard();
    void loadDemo();
    // Reset transient editing state (after loading a board from disk).
    void resetEditingState();

    bool buildingRail() const { return buildingRail_; }
    const std::vector<glm::vec3>& railNodes() const { return railNodes_; }
    bool nextFlipperLeft() const { return nextFlipperLeft_; }

private:
    domain::IBoardElement* pick(const domain::Ray& ray);
    void placeAt(const glm::vec3& worldPoint);

    domain::Board board_;
    domain::ElementFactory factory_;
    Tool tool_{Tool::Select};
    domain::IBoardElement* selected_{nullptr};

    bool buildingRail_{false};
    std::vector<glm::vec3> railNodes_;
    bool nextFlipperLeft_{true};
};

} // namespace pinball::app
