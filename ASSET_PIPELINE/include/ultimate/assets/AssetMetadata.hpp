#pragma once
#include <filesystem>
#include <cstdint>
#include <string>

namespace ultimate::assets {
struct FileMetadata {
    std::string format;
    std::string resolution;
    std::uintmax_t sizeBytes{0};
    bool recognized{false};
    std::string description;
};
FileMetadata inspectFile(const std::filesystem::path& path);
}
