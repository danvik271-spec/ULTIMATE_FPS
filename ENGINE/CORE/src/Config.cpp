#include "ultimate/core/Config.hpp"
#include <charconv>
#include <fstream>
#include <string_view>

namespace ultimate::core {
namespace {
std::string trim(std::string s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}
}
bool Config::load(const std::filesystem::path& path, std::string* error) {
    std::ifstream in(path);
    if (!in) { if (error) *error = "cannot open config: " + path.string(); return false; }
    std::unordered_map<std::string, std::string> loaded;
    std::string line;
    std::size_t lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        const auto clean = trim(line);
        if (clean.empty() || clean.front() == '#' || clean.front() == ';') continue;
        const auto sep = clean.find('=');
        if (sep == std::string::npos) { if (error) *error = "missing '=' at line " + std::to_string(lineNo); return false; }
        auto key = trim(clean.substr(0, sep));
        auto value = trim(clean.substr(sep + 1));
        if (key.empty()) { if (error) *error = "empty key at line " + std::to_string(lineNo); return false; }
        loaded[std::move(key)] = std::move(value);
    }
    values_ = std::move(loaded);
    return true;
}
bool Config::save(const std::filesystem::path& path, std::string* error) const {
    std::ofstream out(path);
    if (!out) { if (error) *error = "cannot write config: " + path.string(); return false; }
    for (const auto& [key, value] : values_) out << key << " = " << value << '\n';
    return static_cast<bool>(out);
}
std::string Config::get(const std::string_view key, std::string fallback) const {
    const auto it = values_.find(std::string(key));
    return it == values_.end() ? std::move(fallback) : it->second;
}
int Config::getInt(const std::string_view key, const int fallback) const {
    const auto value = get(key);
    int parsed{};
    const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
    return ec == std::errc{} && ptr == value.data() + value.size() ? parsed : fallback;
}
float Config::getFloat(const std::string_view key, const float fallback) const {
    const auto value = get(key);
    try { std::size_t used = 0; const float parsed = std::stof(value, &used); return used == value.size() ? parsed : fallback; }
    catch (...) { return fallback; }
}
void Config::set(std::string key, std::string value) { values_[std::move(key)] = std::move(value); }
}
