#include "ultimate/core/Clock.hpp"
#include <algorithm>

namespace ultimate::core {
Clock::Clock() noexcept { reset(); }
void Clock::reset() noexcept { start_ = previous_ = clock::now(); delta_ = 0.0F; }
float Clock::tick() noexcept {
    const auto now = clock::now();
    delta_ = std::chrono::duration<float>(now - previous_).count();
    previous_ = now;
    return delta_;
}
double Clock::elapsedSeconds() const noexcept {
    return std::chrono::duration<double>(clock::now() - start_).count();
}
} // namespace ultimate::core
