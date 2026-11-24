#pragma once
#include <string_view>

namespace ultimate::core {
enum class LogLevel { Trace, Info, Warning, Error };
void log(LogLevel level, std::string_view message);
}
