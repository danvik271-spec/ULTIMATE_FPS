#pragma once
#include <cstdint>

namespace ultimate::ecs {
struct Entity {
    std::uint32_t id{0};
    std::uint32_t generation{0};
    friend constexpr bool operator==(Entity, Entity) noexcept = default;
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return id != 0; }
};
}
