#pragma once
#include "domain/render/RenderItem.hpp"
#include "infrastructure/gl/Mesh.hpp"

#include <map>

namespace pinball::gl {

// Owns the procedural unit meshes referenced by domain::MeshId.
class MeshLibrary {
public:
    void build();
    const Mesh& get(domain::MeshId id) const;

private:
    std::map<domain::MeshId, Mesh> meshes_;
};

} // namespace pinball::gl
