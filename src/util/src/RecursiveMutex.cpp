#include "wrappers/RecursiveMutex.hpp"
#include <core/SafeAssert.h>
#include <utility>

namespace util::wrappers
{
RecursiveMutex::RecursiveMutex() : mutexHandle(xSemaphoreCreateRecursiveMutex())
{
    SafeAssert(mutexHandle != nullptr);
}

RecursiveMutex::~RecursiveMutex()
{
    if (mutexHandle != nullptr)
    {
        vSemaphoreDelete(mutexHandle);
    }
}

void RecursiveMutex::lock()
{
    xSemaphoreTakeRecursive(mutexHandle, portMAX_DELAY);
}

bool RecursiveMutex::lockWithTimeout(const TickType_t timeToWait)
{
    return xSemaphoreTakeRecursive(mutexHandle, timeToWait) == pdPASS;
}

void RecursiveMutex::unlock()
{
    xSemaphoreGiveRecursive(mutexHandle);
}

RecursiveMutex::RecursiveMutex(RecursiveMutex &&other) noexcept
{
    (*this) = std::move(other);
}

RecursiveMutex &RecursiveMutex::operator=(RecursiveMutex &&other) noexcept
{
    mutexHandle = std::exchange(other.mutexHandle, nullptr);
    return *this;
}

} // namespace util::wrappers
