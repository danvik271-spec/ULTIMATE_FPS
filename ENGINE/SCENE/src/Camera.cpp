#include "ultimate/scene/Camera.hpp"
#include <algorithm>
#include <cmath>

namespace ultimate::scene {
void Camera::setRotation(const float yawRadians, const float pitchRadians) noexcept {
    yaw_ = yawRadians;
    pitch_ = std::clamp(pitchRadians, -1.553343F, 1.553343F);
}
void Camera::rotate(const float yawDelta, const float pitchDelta) noexcept { setRotation(yaw_ + yawDelta, pitch_ + pitchDelta); }
math::Vec3 Camera::forward() const noexcept {
    const float cp = std::cos(pitch_);
    return math::normalized({std::sin(yaw_) * cp, std::sin(pitch_), -std::cos(yaw_) * cp});
}
math::Vec3 Camera::right() const noexcept { return math::normalized(math::cross(forward(), {0.0F, 1.0F, 0.0F})); }
math::Vec3 Camera::up() const noexcept { return math::normalized(math::cross(right(), forward())); }
}
