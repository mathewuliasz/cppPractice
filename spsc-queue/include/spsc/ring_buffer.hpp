#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <new>
#include <type_traits>

// template used for generic programming
template <typename T, std::size_t N>
class RingBuffer
{
    static_assert(((N & (N - 1)) == 0), "Buffer size must a power of 2");
    // static_assert(N > 0, "Buffer size must be > 0");

    // std::atomic<std::size_t> pushCursor_{0};
    //  std::atomic<std::size_t> popCursor_{0};
    alignas(64) std::atomic<std::size_t> pushCursor_{0};
    alignas(64) std::atomic<std::size_t> popCursor_{0};

    // local caching to reduce cross-core atomic reads
    std::size_t cachedPushCursor_{0};
    std::size_t cachedPopCursor_{0};

    alignas(T) std::byte ring_[sizeof(T) * N];

    T *slot(std::size_t idx)
    {
        return reinterpret_cast<T *>(&ring_[sizeof(T) * idx]);
    }

public:
    RingBuffer() = default;
    RingBuffer(const RingBuffer &) = delete;
    RingBuffer &operator=(const RingBuffer &) = delete;
    RingBuffer(RingBuffer &&) = delete;
    RingBuffer &operator=(RingBuffer &&) = delete;

    /* ~RingBuffer()
     {
         while (!empty())
         {
             slot(popCursor_.load() & (N - 1))->~T();
             popCursor_.store(popCursor_.load() + 1);
         }
     }
     */
    ~RingBuffer()
    {
        while (popCursor_.load(std::memory_order_relaxed) !=
               pushCursor_.load(std::memory_order_relaxed))
        {
            auto pop = popCursor_.load(std::memory_order_relaxed);
            slot(pop & (N - 1))->~T();
            popCursor_.store(pop + 1, std::memory_order_relaxed);
        }
    }

    /*std::size_t size() const
    {
        return pushCursor_.load() - popCursor_.load();
    }
    bool empty() const { return size() == 0; }
    bool full() const { return size() == N; }
    */
    std::size_t size() const
    {
        return pushCursor_.load(std::memory_order_relaxed) -
               popCursor_.load(std::memory_order_relaxed);
    }
    bool empty() const { return size() == 0; }
    bool full() const { return size() == N; }
    static constexpr std::size_t capacity() { return N; }

    /*bool try_push(const T &value)
    {
        if (full())
        {
            return false;
        }
        new (slot(pushCursor_.load() & (N - 1))) T(value);
        pushCursor_.store(pushCursor_.load() + 1);
        return true;
    }
    bool try_push(T &&value)
    {
        if (full())
        {
            return false;
        }
        new (slot(pushCursor_.load() & (N - 1))) T(std::move(value));
        pushCursor_.store(pushCursor_.load() + 1);
        return true;
    }
    bool try_pop(T &out)
    {
        if (empty())
        {
            return false;
        }
        out = std::move(*slot(popCursor_.load() & (N - 1)));
        slot(popCursor_.load() & (N - 1))->~T();
        popCursor_.store(popCursor_.load() + 1);
        return true;
    }
        */

    // These methods use cached local Cursors
    bool try_push(const T &value)
    {
        auto push = pushCursor_.load(std::memory_order_relaxed);
        if (push - cachedPopCursor_ == N)
        {
            cachedPopCursor_ = popCursor_.load(std::memory_order_acquire);
            if (push - cachedPopCursor_ == N)
                return false;
        }
        new (slot(push & (N - 1))) T(value);
        pushCursor_.store(push + 1, std::memory_order_release);
        return true;
    }
    bool try_pop(T &out)
    {
        auto pop = popCursor_.load(std::memory_order_relaxed);
        if (pop == cachedPushCursor_)
        {
            cachedPushCursor_ = pushCursor_.load(std::memory_order_acquire);
            if (pop == cachedPushCursor_)
                return false;
        }
        out = std::move(*slot(pop & (N - 1)));
        slot(pop & (N - 1))->~T();
        popCursor_.store(pop + 1, std::memory_order_release);
        return true;
    }
};
