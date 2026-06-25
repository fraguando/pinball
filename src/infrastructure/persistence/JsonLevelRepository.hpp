#pragma once
#include "application/ports/ILevelRepository.hpp"
#include "domain/elements/ElementFactory.hpp"

namespace pinball::infra {

// Saves/loads boards as JSON. Element fields are (de)serialized through the
// domain's IArchive, so this adapter is the only place that knows the format.
class JsonLevelRepository final : public app::ILevelRepository {
public:
    JsonLevelRepository();

    bool save(const domain::Board& board, const std::string& path) override;
    bool load(domain::Board& board, const std::string& path) override;

private:
    domain::ElementFactory factory_;
};

} // namespace pinball::infra
