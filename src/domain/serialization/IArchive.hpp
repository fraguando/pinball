#pragma once
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace pinball::domain {

// A direction-agnostic serialization port. Elements implement a single
// `serialize(IArchive&)` that names their fields; concrete archives in the
// infrastructure layer either read or write through it. This keeps the domain
// free of any concrete format (JSON lives only in infrastructure).
class IArchive {
public:
    virtual ~IArchive() = default;

    // True when loading (fields are read into the element); false when saving.
    virtual bool isLoading() const = 0;

    virtual void field(const char* key, float& value) = 0;
    virtual void field(const char* key, int& value) = 0;
    virtual void field(const char* key, bool& value) = 0;
    virtual void field(const char* key, glm::vec3& value) = 0;
    virtual void field(const char* key, std::vector<glm::vec3>& value) = 0;
};

} // namespace pinball::domain
