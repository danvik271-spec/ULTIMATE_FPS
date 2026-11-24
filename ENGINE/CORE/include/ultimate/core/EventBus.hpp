#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ultimate::core {
class EventBus {
    struct Slot { std::uint64_t id; std::function<void(const void*)> callback; };
public:
    template<class Event, class F> std::uint64_t subscribe(F&& fn) {
        const auto id = nextId_++;
        auto& slots = listeners_[std::type_index(typeid(Event))];
        slots.push_back({id, [callback = std::forward<F>(fn)](const void* e) { callback(*static_cast<const Event*>(e)); }});
        return id;
    }
    template<class Event> void publish(const Event& event) const {
        const auto it = listeners_.find(std::type_index(typeid(Event)));
        if (it == listeners_.end()) return;
        for (const auto& slot : it->second) slot.callback(&event);
    }
    template<class Event> bool unsubscribe(const std::uint64_t id) {
        const auto it = listeners_.find(std::type_index(typeid(Event)));
        if (it == listeners_.end()) return false;
        auto& slots = it->second;
        const auto oldSize = slots.size();
        std::erase_if(slots, [id](const Slot& slot) { return slot.id == id; });
        return slots.size() != oldSize;
    }
private:
    std::uint64_t nextId_{1};
    std::unordered_map<std::type_index, std::vector<Slot>> listeners_;
};
}
