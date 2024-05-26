#pragma once
#include <coroutine>

namespace Net {
struct TVoidPromise;

struct TVoidTask : std::coroutine_handle<TVoidPromise>
{
    using promise_type = TVoidPromise;
};

struct TVoidPromise
{
    TVoidTask get_return_object() { return { TVoidTask::from_promise(*this) }; }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
};

struct TVoidSuspendedPromise;

struct TVoidSuspendedTask : std::coroutine_handle<TVoidSuspendedPromise>
{
    using promise_type = TVoidSuspendedPromise;
};

struct TVoidSuspendedPromise
{
    TVoidSuspendedTask get_return_object() { return { TVoidSuspendedTask::from_promise(*this) }; }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
};

inline auto SelfId() {
    struct Awaitable {
        bool await_ready() {
            return false;
        }

        bool await_suspend(std::coroutine_handle<> h) {
            H = h;
            return false;
        }

        auto await_resume() noexcept {
            return H;
        }

        std::coroutine_handle<> H;
    };

    return Awaitable{};
}

} // namespace NNet
