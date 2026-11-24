#pragma once
#include "ultimate/assets/AssetMetadata.hpp"
#include <filesystem>

namespace ultimate::assets {
class TextureProcessor { public: [[nodiscard]] FileMetadata inspect(const std::filesystem::path& p) const; };
class AudioProcessor { public: [[nodiscard]] FileMetadata inspect(const std::filesystem::path& p) const; };
class ModelProcessor { public: [[nodiscard]] FileMetadata inspect(const std::filesystem::path& p) const; };
}
