#include "ultimate/assets/Processors.hpp"
#include <algorithm>
namespace ultimate::assets {
FileMetadata ModelProcessor::inspect(const std::filesystem::path& p) const {
    auto m = inspectFile(p);
    const auto ext = p.extension().string();
    if (ext != ".gltf" && ext != ".glb" && ext != ".obj" && ext != ".fbx" && ext != ".ply" && ext != ".stl") { m.recognized = false; m.description = "file is not a supported model format"; }
    return m;
}
}
