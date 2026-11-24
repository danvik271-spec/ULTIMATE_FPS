#pragma once
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <utility>
#include <thread>
#include <type_traits>
#include <vector>

namespace ultimate::core {
class JobSystem {
public:
    explicit JobSystem(std::size_t workerCount = 0);
    ~JobSystem();
    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;
    template<class F, class... Args>
    auto submit(F&& fn, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using Result = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<Result()>>(
            [func = std::forward<F>(fn), ... captured = std::forward<Args>(args)]() mutable {
                return std::invoke(std::move(func), std::move(captured)...);
            });
        auto future = task->get_future();
        {
            std::lock_guard lock(mutex_);
            if (stopping_) throw std::runtime_error("job system is stopping");
            jobs_.emplace([task] { (*task)(); });
        }
        ready_.notify_one();
        return future;
    }
    void waitIdle();
    [[nodiscard]] std::size_t workerCount() const noexcept { return workers_.size(); }
private:
    void workerLoop();
    mutable std::mutex mutex_;
    std::condition_variable ready_;
    std::condition_variable idle_;
    std::queue<std::function<void()>> jobs_;
    std::vector<std::thread> workers_;
    std::size_t active_{0};
    bool stopping_{false};
};
}
