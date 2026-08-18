/*  参考DiligentEngine:https://github.com/DiligentGraphics/DiligentEngine  */
#pragma once

#include <atomic>
#include <mutex>

#include "Basic/interface/DebugUtilities.hpp"

namespace Threading
{
    class SpinLock
    {
    public:
        // See https://rigtorp.se/spinlock/
        SpinLock() noexcept {}

        // clang-format off
        SpinLock(const SpinLock&) = delete;
        SpinLock& operator = (const SpinLock&) = delete;
        SpinLock(SpinLock&&) = delete;
        SpinLock& operator = (SpinLock&&) = delete;
        // clang-format on

        void Lock() noexcept
        {
            while (true)
            {
                // Assume that lock is free on the first try.
                const bool WasLocked = m_IsLocked.exchange(true, std::memory_order_acquire);
                if (!WasLocked)
                    return; // The lock was not acquired when this thread performed the exchange

                Wait();
            }
        }

        bool TryLock() noexcept
        {
            // First do a relaxed load to check if lock is free in order to prevent
            // unnecessary cache misses if someone does while (!TryLock()).
            if (IsLocked())
                return false;

            const bool WasLocked = m_IsLocked.exchange(true, std::memory_order_acquire);
            return !WasLocked;
        }

        void UnLock() noexcept
        {
            VERIFY(IsLocked(), "Attempting to unlock a spin lock that is not locked. This is a strong indication of a flawed logic.");
            m_IsLocked.store(false, std::memory_order_release);
        }

        bool IsLocked() const noexcept
        {
            // Use relaxed load as we only want to check the value.
            // To impose ordering, Lock()/TryLock() must be used.
            return m_IsLocked.load(std::memory_order_relaxed);
        }

    private:
        void Wait() noexcept;

    private:
        std::atomic<bool> m_IsLocked{ false };
    };

    using SpinLockGuard = std::lock_guard<SpinLock>;
}
