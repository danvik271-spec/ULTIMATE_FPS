#pragma once
#include "ultimate/math/Vec.hpp"

namespace ultimate::physics {
struct Aabb {
    math::Vec3 min{};
    math::Vec3 max{};
    [[nodiscard]] constexpr bool overlaps(const Aabb& b) const noexcept {
        return min.x < b.max.x && max.x > b.min.x && min.y < b.max.y && max.y > b.min.y && min.z < b.max.z && max.z > b.min.z;
    }
    [[nodiscard]] constexpr math::Vec3 center() const noexcept { return (min + max) * 0.5F; }
    [[nodiscard]] constexpr math::Vec3 extents() const noexcept { return (max - min) * 0.5F; }
    void translate(const math::Vec3& d) noexcept { min += d; max += d; }
};
}
