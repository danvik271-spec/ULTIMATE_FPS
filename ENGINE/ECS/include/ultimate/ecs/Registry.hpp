#pragma once
#include "ultimate/ecs/Entity.hpp"
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace ultimate::ecs {
class Registry {
    struct IPool { virtual ~IPool() = default; virtual void erase(std::uint32_t id) = 0; };
    template<class T> struct Pool final : IPool {
        std::unordered_map<std::uint32_t, T> values;
        void erase(const std::uint32_t id) override { values.erase(id); }
    };
public:
    Entity create() {
        const std::uint32_t id = nextId_++;
        alive_.emplace(id, 1U);
        return {id, 1U};
    }
    bool destroy(const Entity entity) {
        if (!valid(entity)) return false;
        for (auto& [_, pool] : pools_) pool->erase(entity.id);
        alive_.erase(entity.id);
        return true;
    }
    [[nodiscard]] bool valid(const Entity entity) const {
        const auto it = alive_.find(entity.id);
        return entity.id != 0 && it != alive_.end() && it->second == entity.generation;
    }
    template<class T, class... Args> T& emplace(const Entity entity, Args&&... args) {
        requireValid(entity);
        auto& values = assurePool<T>().values;
        return values.insert_or_assign(entity.id, T{std::forward<Args>(args)...}).first->second;
    }
    template<class T> T* get(const Entity entity) {
        if (!valid(entity)) return nullptr;
        auto* p = findPool<T>();
        if (!p) return nullptr;
        const auto it = p->values.find(entity.id);
        return it == p->values.end() ? nullptr : &it->second;
    }
    template<class T> const T* get(const Entity entity) const {
        if (!valid(entity)) return nullptr;
        const auto* p = findPool<T>();
        if (!p) return nullptr;
        const auto it = p->values.find(entity.id);
        return it == p->values.end() ? nullptr : &it->second;
    }
    template<class T> bool remove(const Entity entity) {
        if (!valid(entity)) return false;
        auto* p = findPool<T>();
        return p && p->values.erase(entity.id) != 0;
    }
    template<class T, class F> void each(F&& fn) {
        auto* p = findPool<T>();
        if (!p) return;
        for (auto& [id, component] : p->values) {
            const auto it = alive_.find(id);
            if (it != alive_.end()) fn(Entity{id, it->second}, component);
        }
    }
    [[nodiscard]] std::size_t entityCount() const noexcept { return alive_.size(); }
private:
    void requireValid(const Entity e) const { if (!valid(e)) throw std::invalid_argument("entity is not alive"); }
    template<class T> Pool<T>& assurePool() {
        const auto key = std::type_index(typeid(T));
        auto it = pools_.find(key);
        if (it == pools_.end()) it = pools_.emplace(key, std::make_unique<Pool<T>>()).first;
        return *static_cast<Pool<T>*>(it->second.get());
    }
    template<class T> Pool<T>* findPool() {
        const auto it = pools_.find(std::type_index(typeid(T)));
        return it == pools_.end() ? nullptr : static_cast<Pool<T>*>(it->second.get());
    }
    template<class T> const Pool<T>* findPool() const {
        const auto it = pools_.find(std::type_index(typeid(T)));
        return it == pools_.end() ? nullptr : static_cast<const Pool<T>*>(it->second.get());
    }
    std::uint32_t nextId_{1};
    std::unordered_map<std::uint32_t, std::uint32_t> alive_;
    std::unordered_map<std::type_index, std::unique_ptr<IPool>> pools_;
};
}
