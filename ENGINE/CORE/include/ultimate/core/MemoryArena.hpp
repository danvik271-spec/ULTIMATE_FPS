#pragma once
#include <cstddef>
#include <cstdint>
#include <utility>
#include <memory>
#include <new>
#include <stdexcept>
#include <vector>

namespace ultimate::core {
class MemoryArena {
public:
    explicit MemoryArena(std::size_t capacity) : storage_(capacity) {}
    void* allocate(std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) throw std::invalid_argument("alignment must be a power of two");
        const auto base = reinterpret_cast<std::uintptr_t>(storage_.data());
        const auto current = base + offset_;
        const auto aligned = (current + alignment - 1U) & ~(static_cast<std::uintptr_t>(alignment) - 1U);
        const auto padding = static_cast<std::size_t>(aligned - current);
        if (padding > storage_.size() - offset_ || bytes > storage_.size() - offset_ - padding) throw std::bad_alloc();
        offset_ += padding + bytes;
        return reinterpret_cast<void*>(aligned);
    }
    template<class T, class... Args> T* make(Args&&... args) {
        return ::new (allocate(sizeof(T), alignof(T))) T(static_cast<Args&&>(args)...);
    }
    void reset() noexcept { offset_ = 0; }
    [[nodiscard]] std::size_t used() const noexcept { return offset_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return storage_.size(); }
private:
    std::vector<std::byte> storage_;
    std::size_t offset_{0};
};
}
