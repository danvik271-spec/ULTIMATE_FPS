#pragma once
#include "ultimate/math/Vec.hpp"
#include <array>
#include <cstddef>

namespace ultimate::input {
enum class Key : std::size_t { Forward, Backward, Left, Right, Jump, Sprint, Interact, Escape, Count };
class InputState {
public:
    void set(Key key, bool down) noexcept { keys_[static_cast<std::size_t>(key)] = down; }
    [[nodiscard]] bool down(Key key) const noexcept { return keys_[static_cast<std::size_t>(key)]; }
    void setLookDelta(math::Vec2 delta) noexcept { lookDelta_ = delta; }
    [[nodiscard]] math::Vec2 lookDelta() const noexcept { return lookDelta_; }
    void endFrame() noexcept { lookDelta_ = {}; }
private:
    std::array<bool, static_cast<std::size_t>(Key::Count)> keys_{};
    math::Vec2 lookDelta_{};
};
}
