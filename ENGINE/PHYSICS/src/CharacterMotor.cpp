#include "ultimate/physics/CharacterMotor.hpp"

namespace ultimate::physics {
namespace {
Aabb at(const Aabb& local, const math::Vec3& p) { return {local.min + p, local.max + p}; }
}
MoveResult CharacterMotor::move(math::Vec3& position, const math::Vec3& displacement, const CollisionWorld& world) const {
    MoveResult out{};
    auto tryAxis = [&](const float amount, const int axis, bool& hit) {
        if (amount == 0.0F) return;
        math::Vec3 delta{};
        if (axis == 0) delta.x = amount; else if (axis == 1) delta.y = amount; else delta.z = amount;
        Aabb candidate = at(localBounds_, position + delta);
        if (!world.overlapsAny(candidate)) {
            position += delta;
            if (axis == 0) out.applied.x += amount; else if (axis == 1) out.applied.y += amount; else out.applied.z += amount;
        } else hit = true;
    };
    tryAxis(displacement.x, 0, out.collidedX);
    tryAxis(displacement.y, 1, out.collidedY);
    tryAxis(displacement.z, 2, out.collidedZ);
    return out;
}
}
