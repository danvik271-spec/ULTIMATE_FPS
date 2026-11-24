#pragma once
#include "ultimate/physics/Aabb.hpp"
#include <vector>

namespace ultimate::physics {
class CollisionWorld {
public:
    void addStaticCollider(Aabb bounds);
    bool removeStaticCollider(std::size_t index);
    [[nodiscard]] bool overlapsAny(const Aabb& bounds) const noexcept;
    [[nodiscard]] const std::vector<Aabb>& colliders() const noexcept { return colliders_; }
private:
    std::vector<Aabb> colliders_;
};
}
