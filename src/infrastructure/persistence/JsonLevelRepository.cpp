#include "infrastructure/persistence/JsonLevelRepository.hpp"

#include "infrastructure/persistence/JsonArchive.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>

namespace pinball::infra {

using namespace pinball::domain;

JsonLevelRepository::JsonLevelRepository() : factory_(ElementFactory::makeDefault()) {}

bool JsonLevelRepository::save(const Board& board, const std::string& path) {
    nlohmann::json root;
    root["version"] = 1;
    root["elements"] = nlohmann::json::array();

    for (const auto& e : board.elements()) {
        nlohmann::json obj;
        obj["type"] = toString(e->type());
        JsonWriteArchive ar(obj);
        // The write archive only reads fields; const_cast is safe here.
        const_cast<IBoardElement*>(e.get())->serialize(ar);
        root["elements"].push_back(std::move(obj));
    }

    try {
        std::filesystem::path p(path);
        if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
        std::ofstream f(path);
        if (!f) return false;
        f << root.dump(2);
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "[level] save failed: %s\n", ex.what());
        return false;
    }
    return true;
}

bool JsonLevelRepository::load(Board& board, const std::string& path) {
    nlohmann::json root;
    try {
        std::ifstream f(path);
        if (!f) return false;
        f >> root;
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "[level] load failed: %s\n", ex.what());
        return false;
    }

    if (!root.contains("elements") || !root["elements"].is_array()) return false;

    board.clear();
    for (const auto& obj : root["elements"]) {
        if (!obj.contains("type")) continue;
        auto element = factory_.create(obj["type"].get<std::string>());
        if (!element) continue;
        JsonReadArchive ar(obj);
        element->serialize(ar);
        board.add(std::move(element));
    }
    return true;
}

} // namespace pinball::infra
