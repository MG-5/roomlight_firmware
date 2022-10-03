#include "wrappers/Mutex.hpp"
#include <core/SafeAssert.h>
#include <utility>

namespace util::wrappers
{
Mutex::Mutex() : mutexHandle(xSemaphoreCreateMutex())
{
    SafeAssert(mutexHandle != nullptr);
}

Mutex::~Mutex()
{
    if (mutexHandle != nullptr)
    {
        vSemaphoreDelete(mutexHandle);
    }
}

void Mutex::lock()
{
    xSemaphoreTake(mutexHandle, portMAX_DELAY);
}

bool Mutex::lockWithTimeout(const TickType_t timeToWait)
{
    return xSemaphoreTake(mutexHandle, timeToWait) == pdPASS;
}

void Mutex::unlock()
{
    xSemaphoreGive(mutexHandle);
}

Mutex::Mutex(Mutex &&other) noexcept
{
    (*this) = std::move(other);
}

Mutex &Mutex::operator=(Mutex &&other) noexcept
{
    mutexHandle = std::exchange(other.mutexHandle, nullptr);
    return *this;
}

} // namespace util::wrappers
