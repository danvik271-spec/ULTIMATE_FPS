#pragma once
#include "ultimate/assets/AssetRecord.hpp"
#include <filesystem>
#include <string>

namespace ultimate::assets {
struct DownloadResult { bool success{false}; std::string message; std::string sha256; };
class AssetDownloader {
public:
    DownloadResult download(const AssetRecord& record, const std::filesystem::path& destination) const;
};
}
