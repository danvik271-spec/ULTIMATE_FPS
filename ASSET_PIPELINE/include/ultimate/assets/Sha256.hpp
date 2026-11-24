#pragma once
#include <filesystem>
#include <string>

namespace ultimate::assets {
std::string sha256File(const std::filesystem::path& path, std::string* error = nullptr);
}
