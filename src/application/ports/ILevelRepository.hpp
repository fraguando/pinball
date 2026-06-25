#pragma once
#include "domain/Board.hpp"

#include <string>

namespace pinball::app {

// Persistence port for boards. The concrete adapter (JSON) lives in
// infrastructure, so the domain/application never depend on a file format.
class ILevelRepository {
public:
    virtual ~ILevelRepository() = default;

    // Returns false on I/O or parse failure.
    virtual bool save(const domain::Board& board, const std::string& path) = 0;
    virtual bool load(domain::Board& board, const std::string& path) = 0;
};

} // namespace pinball::app
