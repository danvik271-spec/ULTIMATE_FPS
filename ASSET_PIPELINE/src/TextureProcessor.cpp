#include "ultimate/assets/Processors.hpp"
namespace ultimate::assets {
FileMetadata TextureProcessor::inspect(const std::filesystem::path& p) const {
    auto m = inspectFile(p);
    if (m.format == "WAV" || m.format == "ogg" || m.format == "flac" || m.format == "mp3") { m.recognized = false; m.description = "file is audio, not a texture"; }
    return m;
}
}
