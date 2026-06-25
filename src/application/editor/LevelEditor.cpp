#include "application/editor/LevelEditor.hpp"

#include "domain/elements/Flipper.hpp"
#include "domain/elements/Rail.hpp"

#include <limits>

namespace pinball::app {

using namespace pinball::domain;

namespace {
ElementType toElementType(Tool t) {
    switch (t) {
        case Tool::Wall:    return ElementType::Wall;
        case Tool::Bumper:  return ElementType::Bumper;
        case Tool::Flipper: return ElementType::Flipper;
        case Tool::Hole:    return ElementType::Hole;
        case Tool::Rail:    return ElementType::Rail;
        case Tool::Spawn:   return ElementType::Spawn;
        case Tool::Select:  break;
    }
    return ElementType::Wall;
}
} // namespace

const char* toolName(Tool t) {
    switch (t) {
        case Tool::Select:  return "SELECT";
        case Tool::Wall:    return "WALL";
        case Tool::Bumper:  return "BUMPER";
        case Tool::Flipper: return "FLIPPER";
        case Tool::Hole:    return "HOLE";
        case Tool::Rail:    return "RAIL";
        case Tool::Spawn:   return "SPAWN";
    }
    return "?";
}

LevelEditor::LevelEditor() : factory_(ElementFactory::makeDefault()) {
    loadDemo();
}

void LevelEditor::setTool(Tool t) {
    // Switching tools abandons any half-built rail.
    if (buildingRail_ && t != Tool::Rail) {
        buildingRail_ = false;
        railNodes_.clear();
    }
    tool_ = t;
    selected_ = nullptr;
}

void LevelEditor::primaryClick(const glm::vec3& worldPoint, const Ray& ray) {
    if (tool_ == Tool::Select) {
        selected_ = pick(ray);
        return;
    }
    if (tool_ == Tool::Rail) {
        if (!buildingRail_) {
            buildingRail_ = true;
            railNodes_.clear();
        }
        railNodes_.push_back(glm::vec3(worldPoint.x, 0.4f, worldPoint.z));
        return;
    }
    placeAt(worldPoint);
}

void LevelEditor::placeAt(const glm::vec3& worldPoint) {
    auto element = factory_.create(toElementType(tool_));
    if (!element) return;
    element->setPosition(glm::vec3(worldPoint.x, 0.0f, worldPoint.z));
    if (auto* flipper = dynamic_cast<Flipper*>(element.get())) {
        flipper->setSide(nextFlipperLeft_);
        nextFlipperLeft_ = !nextFlipperLeft_; // alternate left/right placements
    }
    selected_ = board_.add(std::move(element));
}

void LevelEditor::dragSelectedTo(const glm::vec3& worldPoint) {
    if (!selected_) return;
    selected_->setPosition(glm::vec3(worldPoint.x, selected_->position().y, worldPoint.z));
}

void LevelEditor::confirm() {
    if (!buildingRail_) return;
    buildingRail_ = false;
    if (railNodes_.size() >= 2) {
        auto rail = std::make_unique<Rail>();
        rail->setNodes(railNodes_);
        selected_ = board_.add(std::move(rail));
    }
    railNodes_.clear();
}

void LevelEditor::deleteSelected() {
    if (!selected_) return;
    board_.remove(selected_);
    selected_ = nullptr;
}

void LevelEditor::clearBoard() {
    board_.clear();
    resetEditingState();
}

void LevelEditor::resetEditingState() {
    selected_ = nullptr;
    buildingRail_ = false;
    railNodes_.clear();
}

IBoardElement* LevelEditor::pick(const Ray& ray) {
    IBoardElement* best = nullptr;
    float bestT = std::numeric_limits<float>::max();
    for (const auto& e : board_.elements()) {
        float t;
        if (e->raycastHit(ray, t) && t < bestT) {
            bestT = t;
            best = e.get();
        }
    }
    return best;
}

void LevelEditor::loadDemo() {
    clearBoard();
    auto add = [&](ElementType type, const glm::vec3& pos) {
        auto e = factory_.create(type);
        e->setPosition(pos);
        return board_.add(std::move(e));
    };

    add(ElementType::Spawn, glm::vec3(6.8f, 0.0f, 11.0f));

    // Two flippers either side of the drain gap.
    {
        auto left = std::make_unique<Flipper>();
        left->setSide(true);
        left->setPosition(glm::vec3(-3.2f, 0.0f, 11.0f));
        board_.add(std::move(left));
        auto right = std::make_unique<Flipper>();
        right->setSide(false);
        right->setPosition(glm::vec3(3.2f, 0.0f, 11.0f));
        board_.add(std::move(right));
    }

    add(ElementType::Bumper, glm::vec3(-3.0f, 0.0f, -2.0f));
    add(ElementType::Bumper, glm::vec3(3.0f, 0.0f, -3.0f));
    add(ElementType::Bumper, glm::vec3(0.0f, 0.0f, 1.0f));
    add(ElementType::Hole, glm::vec3(-5.0f, 0.0f, -8.0f));

    {
        auto rail = std::make_unique<Rail>();
        rail->setNodes({glm::vec3(5.5f, 0.4f, -10.0f), glm::vec3(6.5f, 1.6f, -6.0f),
                        glm::vec3(5.0f, 0.4f, -2.0f)});
        board_.add(std::move(rail));
    }
}

} // namespace pinball::app
