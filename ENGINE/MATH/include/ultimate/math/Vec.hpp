#pragma once
#include <cmath>

namespace ultimate::math {
struct Vec2 {
    float x{0.0F};
    float y{0.0F};
};

struct Vec3 {
    float x{0.0F};
    float y{0.0F};
    float z{0.0F};

    constexpr Vec3 operator+(const Vec3& rhs) const noexcept { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
    constexpr Vec3 operator-(const Vec3& rhs) const noexcept { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
    constexpr Vec3 operator*(float scalar) const noexcept { return {x * scalar, y * scalar, z * scalar}; }
    constexpr Vec3 operator/(float scalar) const noexcept { return {x / scalar, y / scalar, z / scalar}; }
    constexpr Vec3& operator+=(const Vec3& rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    constexpr Vec3& operator-=(const Vec3& rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
};

inline constexpr float dot(const Vec3& a, const Vec3& b) noexcept { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline constexpr Vec3 cross(const Vec3& a, const Vec3& b) noexcept {
    return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}
inline float length(const Vec3& v) noexcept { return std::sqrt(dot(v, v)); }
inline Vec3 normalized(const Vec3& v) noexcept {
    const float n = length(v);
    return n > 1.0e-6F ? v / n : Vec3{};
}
inline constexpr Vec3 clamp(const Vec3& v, const Vec3& lo, const Vec3& hi) noexcept {
    const auto c = [](float x, float a, float b) { return x < a ? a : (x > b ? b : x); };
    return {c(v.x, lo.x, hi.x), c(v.y, lo.y, hi.y), c(v.z, lo.z, hi.z)};
}
} // namespace ultimate::math
