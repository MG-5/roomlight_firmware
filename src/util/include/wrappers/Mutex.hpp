#pragma once
#include "wrappers/IMutex.hpp"
#include <FreeRTOS.h>
#include <semphr.h>

namespace util::wrappers
{
/**
 * Wrapper for FreeRTOS non-recursive mutex
 */
class Mutex : public IMutex
{
public:
    Mutex();
    ~Mutex() override;

    Mutex(const Mutex &) = delete;
    Mutex(Mutex &&other) noexcept;
    Mutex &operator=(const Mutex &) = delete;
    Mutex &operator=(Mutex &&other) noexcept;

    virtual void lock() override;
    virtual bool lockWithTimeout(TickType_t timeToWait) override;
    virtual void unlock() override;

private:
    SemaphoreHandle_t mutexHandle{nullptr};
};
} // namespace util::wrappers