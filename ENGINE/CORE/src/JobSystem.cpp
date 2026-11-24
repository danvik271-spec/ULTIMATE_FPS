#include "ultimate/core/JobSystem.hpp"
#include <algorithm>
#include <stdexcept>

namespace ultimate::core {
JobSystem::JobSystem(std::size_t workerCount) {
    if (workerCount == 0) {
        const auto hc = static_cast<std::size_t>(std::thread::hardware_concurrency());
        workerCount = std::max<std::size_t>(1, hc > 1 ? hc - 1 : 1);
    }
    workers_.reserve(workerCount);
    for (std::size_t i = 0; i < workerCount; ++i) workers_.emplace_back([this] { workerLoop(); });
}
JobSystem::~JobSystem() {
    { std::lock_guard lock(mutex_); stopping_ = true; }
    ready_.notify_all();
    for (auto& worker : workers_) if (worker.joinable()) worker.join();
}
void JobSystem::workerLoop() {
    for (;;) {
        std::function<void()> job;
        {
            std::unique_lock lock(mutex_);
            ready_.wait(lock, [this] { return stopping_ || !jobs_.empty(); });
            if (stopping_ && jobs_.empty()) return;
            job = std::move(jobs_.front()); jobs_.pop(); ++active_;
        }
        job();
        {
            std::lock_guard lock(mutex_);
            --active_;
            if (jobs_.empty() && active_ == 0) idle_.notify_all();
        }
    }
}
void JobSystem::waitIdle() {
    std::unique_lock lock(mutex_);
    idle_.wait(lock, [this] { return jobs_.empty() && active_ == 0; });
}
}
