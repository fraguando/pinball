#pragma once
#include <string>

namespace pinball::domain {

enum class ElementType {
    Wall,
    Bumper,
    Flipper,
    Hole,
    Rail,
    Spawn,
};

inline const char* toString(ElementType t) {
    switch (t) {
        case ElementType::Wall:    return "wall";
        case ElementType::Bumper:  return "bumper";
        case ElementType::Flipper: return "flipper";
        case ElementType::Hole:    return "hole";
        case ElementType::Rail:    return "rail";
        case ElementType::Spawn:   return "spawn";
    }
    return "unknown";
}

} // namespace pinball::domain
