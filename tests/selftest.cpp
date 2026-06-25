// Headless self-test for the non-visual logic: collision math, the element
// factory, and the JSON save/load round-trip. Built as the `pinball_selftest`
// target; returns non-zero if any check fails.
#include "application/editor/LevelEditor.hpp"
#include "domain/Board.hpp"
#include "domain/elements/ElementFactory.hpp"
#include "domain/elements/Rail.hpp"
#include "domain/physics/Collision3D.hpp"
#include "infrastructure/persistence/JsonLevelRepository.hpp"

#include <cstdio>

namespace {
int g_failures = 0;
void check(bool cond, const char* msg) {
    std::printf("%s %s\n", cond ? "[ ok ]" : "[FAIL]", msg);
    if (!cond) ++g_failures;
}
} // namespace

int main() {
    using namespace pinball::domain;

    // --- Element factory knows every built-in type ---
    auto factory = ElementFactory::makeDefault();
    const ElementType all[] = {ElementType::Wall,  ElementType::Bumper, ElementType::Flipper,
                               ElementType::Hole,  ElementType::Rail,   ElementType::Spawn};
    for (auto t : all) check(factory.create(t) != nullptr, toString(t));

    // --- Collision: sphere vs plane / sphere / box ---
    auto plane = CollisionShape::makePlane({0, 0, 0}, {0, 1, 0});
    check(collideSphere({0, 0.3f, 0}, 0.4f, plane).hit, "sphere-plane overlap");
    check(!collideSphere({0, 5.0f, 0}, 0.4f, plane).hit, "sphere-plane clear");

    auto sphere = CollisionShape::makeSphere({0, 0, 0}, 1.0f);
    check(collideSphere({1.2f, 0, 0}, 0.4f, sphere).hit, "sphere-sphere overlap");
    check(!collideSphere({3.0f, 0, 0}, 0.4f, sphere).hit, "sphere-sphere clear");

    glm::mat3 I(1.0f);
    auto box = CollisionShape::makeBox({0, 0, 0}, {1, 1, 1}, I);
    {
        Contact c = collideSphere({1.2f, 0, 0}, 0.4f, box);
        check(c.hit && c.penetration > 0.0f, "sphere-box overlap");
        check(glm::dot(c.normal, glm::vec3(1, 0, 0)) > 0.9f, "sphere-box normal points out");
    }
    check(!collideSphere({3.0f, 0, 0}, 0.4f, box).hit, "sphere-box clear");

    // Bumper material carries score/pop through the contact.
    {
        auto bumper = CollisionShape::makeSphere({0, 0, 0}, 0.9f);
        bumper.scoreOnHit = 100;
        bumper.bonusSpeed = 8.0f;
        Contact c = collideSphere({1.0f, 0, 0}, 0.4f, bumper);
        check(c.hit && c.scoreOnHit == 100 && c.bonusSpeed > 0.0f, "bumper material in contact");
    }

    // --- JSON round-trip of the demo board (incl. rail node arrays) ---
    pinball::app::LevelEditor editor; // constructs the demo board
    const size_t count = editor.board().elements().size();
    check(count > 0, "demo board populated");

    pinball::infra::JsonLevelRepository repo;
    const std::string path = "selftest_board.json";
    check(repo.save(editor.board(), path), "board saved");

    Board loaded;
    check(repo.load(loaded, path), "board loaded");
    check(loaded.elements().size() == count, "element count preserved");

    // Find a rail in each and compare node counts (exercises vector<vec3>).
    auto firstRail = [](const Board& b) -> const Rail* {
        for (const auto& e : b.elements())
            if (auto* r = dynamic_cast<const Rail*>(e.get())) return r;
        return nullptr;
    };
    const Rail* a = firstRail(editor.board());
    const Rail* b = firstRail(loaded);
    check(a && b && a->pathNodes().size() == b->pathNodes().size() && a->pathNodes().size() >= 2,
          "rail nodes preserved");

    std::printf("\n%s (%d failure(s))\n", g_failures == 0 ? "ALL PASS" : "FAILED", g_failures);
    return g_failures == 0 ? 0 : 1;
}
