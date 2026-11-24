#include "ultimate/assets/Processors.hpp"
#include <algorithm>
namespace ultimate::assets {
FileMetadata AudioProcessor::inspect(const std::filesystem::path& p) const {
    auto m = inspectFile(p);
    const auto ext = p.extension().string();
    if (m.format != "WAV" && ext != ".ogg" && ext != ".flac" && ext != ".mp3") { m.recognized = false; m.description = "file is not a supported audio format"; }
    return m;
}
}
