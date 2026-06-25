#pragma once
#include "domain/elements/ElementType.hpp"
#include "domain/elements/IBoardElement.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace pinball::domain {

// Creates board elements by type. New element types are added by registering a
// creator here (one line) without touching the simulation, renderer or editor.
class ElementFactory {
public:
    using Creator = std::function<std::unique_ptr<IBoardElement>()>;

    void registerType(ElementType type, Creator creator);

    std::unique_ptr<IBoardElement> create(ElementType type) const;
    std::unique_ptr<IBoardElement> create(const std::string& typeName) const;

    bool knows(ElementType type) const;

    // A factory pre-populated with all built-in element types.
    static ElementFactory makeDefault();

private:
    std::map<ElementType, Creator> creators_;
};

} // namespace pinball::domain
