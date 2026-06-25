#pragma once
#include "application/ports/IClock.hpp"

#include <cstdint>

namespace pinball::infra {

// IClock backed by SDL's high-resolution performance counter.
class SdlClock final : public app::IClock {
public:
    SdlClock();
    double now() const override;

private:
    uint64_t start_;
    double freq_;
};

} // namespace pinball::infra
