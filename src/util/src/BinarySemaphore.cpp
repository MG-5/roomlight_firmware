#include "wrappers/BinarySemaphore.hpp"
#include <core/SafeAssert.h>
#include <utility>

namespace util::wrappers
{

BinarySemaphore::BinarySemaphore() : semaphoreHandle(xSemaphoreCreateBinary())
{
    SafeAssert(semaphoreHandle != nullptr);
}

BinarySemaphore::~BinarySemaphore()
{
    if (semaphoreHandle != nullptr)
    {
        vSemaphoreDelete(semaphoreHandle);
    }
}

BaseType_t BinarySemaphore::take(const TickType_t blocktime)
{
    return xSemaphoreTake(semaphoreHandle, blocktime);
}

BaseType_t BinarySemaphore::giveFromISR(BaseType_t *pxHigherPriorityTaskWoken)
{
    return xSemaphoreGiveFromISR(semaphoreHandle, pxHigherPriorityTaskWoken);
}

BaseType_t BinarySemaphore::give()
{
    return xSemaphoreGive(semaphoreHandle);
}

BinarySemaphore::BinarySemaphore(BinarySemaphore &&other) noexcept
{
    (*this) = std::forward<BinarySemaphore>(other);
}

BinarySemaphore &BinarySemaphore::operator=(BinarySemaphore &&other) noexcept
{
    semaphoreHandle = std::exchange(other.semaphoreHandle, nullptr);
    return *this;
}

} // namespace util::wrappers