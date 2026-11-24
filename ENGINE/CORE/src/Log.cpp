#include "ultimate/core/Log.hpp"
#include <chrono>
#include <iostream>
#include <mutex>
#include <string_view>

namespace ultimate::core {
namespace { std::mutex g_logMutex; }
void log(const LogLevel level, const std::string_view message) {
    const char* label = "INFO";
    switch (level) {
        case LogLevel::Trace: label = "TRACE"; break;
        case LogLevel::Info: label = "INFO"; break;
        case LogLevel::Warning: label = "WARN"; break;
        case LogLevel::Error: label = "ERROR"; break;
    }
    const auto now = std::chrono::system_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::lock_guard lock(g_logMutex);
    std::clog << '[' << ms << "] [" << label << "] " << message << '\n';
}
} // namespace ultimate::core
