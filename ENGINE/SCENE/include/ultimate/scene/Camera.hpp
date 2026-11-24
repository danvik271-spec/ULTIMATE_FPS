#pragma once
#include "ultimate/math/Vec.hpp"

namespace ultimate::scene {
class Camera {
public:
    void setPosition(math::Vec3 position) noexcept { position_ = position; }
    void setRotation(float yawRadians, float pitchRadians) noexcept;
    void rotate(float yawDelta, float pitchDelta) noexcept;
    [[nodiscard]] math::Vec3 position() const noexcept { return position_; }
    [[nodiscard]] math::Vec3 forward() const noexcept;
    [[nodiscard]] math::Vec3 right() const noexcept;
    [[nodiscard]] math::Vec3 up() const noexcept;
    [[nodiscard]] float yaw() const noexcept { return yaw_; }
    [[nodiscard]] float pitch() const noexcept { return pitch_; }
private:
    math::Vec3 position_{};
    float yaw_{0.0F};
    float pitch_{0.0F};
};
}
