#pragma once
#include <chrono>

namespace ultimate::core {
class Clock {
public:
    using clock = std::chrono::steady_clock;
    Clock() noexcept;
    float tick() noexcept;
    [[nodiscard]] float deltaSeconds() const noexcept { return delta_; }
    [[nodiscard]] double elapsedSeconds() const noexcept;
    void reset() noexcept;
private:
    clock::time_point start_;
    clock::time_point previous_;
    float delta_{0.0F};
};
}
