#include "ultimate/physics/CollisionWorld.hpp"

namespace ultimate::physics {
void CollisionWorld::addStaticCollider(Aabb bounds) { colliders_.push_back(bounds); }
bool CollisionWorld::removeStaticCollider(const std::size_t index) {
    if (index >= colliders_.size()) return false;
    colliders_.erase(colliders_.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}
bool CollisionWorld::overlapsAny(const Aabb& bounds) const noexcept {
    for (const auto& collider : colliders_) if (bounds.overlaps(collider)) return true;
    return false;
}
}
