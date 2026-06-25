#pragma once

namespace pinball::app {

// Monotonic time source, used for timed gameplay (e.g. the hole's 2s hold).
class IClock {
public:
    virtual ~IClock() = default;
    // Seconds since some fixed origin; only differences are meaningful.
    virtual double now() const = 0;
};

} // namespace pinball::app
