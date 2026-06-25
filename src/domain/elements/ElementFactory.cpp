#include "domain/elements/ElementFactory.hpp"

#include "domain/elements/Bumper.hpp"
#include "domain/elements/Flipper.hpp"
#include "domain/elements/Hole.hpp"
#include "domain/elements/Rail.hpp"
#include "domain/elements/Spawn.hpp"
#include "domain/elements/Wall.hpp"

namespace pinball::domain {

void ElementFactory::registerType(ElementType type, Creator creator) {
    creators_[type] = std::move(creator);
}

std::unique_ptr<IBoardElement> ElementFactory::create(ElementType type) const {
    auto it = creators_.find(type);
    if (it == creators_.end()) return nullptr;
    return it->second();
}

std::unique_ptr<IBoardElement> ElementFactory::create(const std::string& typeName) const {
    for (const auto& [type, creator] : creators_) {
        if (typeName == toString(type)) return creator();
    }
    return nullptr;
}

bool ElementFactory::knows(ElementType type) const {
    return creators_.find(type) != creators_.end();
}

ElementFactory ElementFactory::makeDefault() {
    ElementFactory f;
    // Built-in registrations. Adding a new element type means adding one line.
    f.registerType(ElementType::Wall, [] { return std::make_unique<Wall>(); });
    f.registerType(ElementType::Bumper, [] { return std::make_unique<Bumper>(); });
    f.registerType(ElementType::Flipper, [] { return std::make_unique<Flipper>(); });
    f.registerType(ElementType::Hole, [] { return std::make_unique<Hole>(); });
    f.registerType(ElementType::Rail, [] { return std::make_unique<Rail>(); });
    f.registerType(ElementType::Spawn, [] { return std::make_unique<Spawn>(); });
    return f;
}

} // namespace pinball::domain
