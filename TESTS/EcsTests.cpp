#include "TestHarness.hpp"
#include "ultimate/ecs/Registry.hpp"
#include "ultimate/math/Vec.hpp"

TEST(RegistryOwnsComponentsAndDestroysEntities) {
    struct Transform { ultimate::math::Vec3 position; };
    ultimate::ecs::Registry registry;
    const auto e = registry.create();
    registry.emplace<Transform>(e, Transform{{1,2,3}});
    REQUIRE(registry.valid(e)); REQUIRE(registry.get<Transform>(e) != nullptr);
    REQUIRE(registry.get<Transform>(e)->position.y == 2.0F);
    REQUIRE(registry.destroy(e)); REQUIRE(!registry.valid(e)); REQUIRE(registry.get<Transform>(e) == nullptr);
}
