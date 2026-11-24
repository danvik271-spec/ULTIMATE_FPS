#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace ultimate::core {
class Config {
public:
    bool load(const std::filesystem::path& path, std::string* error = nullptr);
    bool save(const std::filesystem::path& path, std::string* error = nullptr) const;
    [[nodiscard]] std::string get(std::string_view key, std::string fallback = {}) const;
    [[nodiscard]] int getInt(std::string_view key, int fallback) const;
    [[nodiscard]] float getFloat(std::string_view key, float fallback) const;
    void set(std::string key, std::string value);
private:
    std::unordered_map<std::string, std::string> values_;
};
}
