#include "wrappers/EventGroup.hpp"
#include <utility>

namespace util::wrappers
{

EventGroup::EventGroup() : eventGroupHandle(xEventGroupCreate())
{
}
EventGroup::~EventGroup()
{
    if (eventGroupHandle != nullptr)
    {
        vEventGroupDelete(eventGroupHandle);
    }
}
EventBits_t EventGroup::clearBits(const EventBits_t uxBitsToClear)
{
    return xEventGroupClearBits(eventGroupHandle, uxBitsToClear);
}
bool EventGroup::clearBitsFromISR(const EventBits_t uxBitsToClear)
{
    return xEventGroupClearBitsFromISR(eventGroupHandle, uxBitsToClear) == pdPASS;
}
EventBits_t EventGroup::getBits()
{
    return xEventGroupGetBits(eventGroupHandle);
}
EventBits_t EventGroup::getBitsFromISR()
{
    return xEventGroupGetBitsFromISR(eventGroupHandle);
}
EventBits_t EventGroup::setBits(const EventBits_t bits)
{
    return xEventGroupSetBits(eventGroupHandle, bits);
}
EventBits_t EventGroup::setBitsFromISR(const EventBits_t bits,
                                       BaseType_t *pxHigherPriorityTaskWoken)
{
    return xEventGroupSetBitsFromISR(eventGroupHandle, bits, pxHigherPriorityTaskWoken);
}
EventBits_t EventGroup::sync(const EventBits_t bitsToSet, const EventBits_t bitsToWaitFor,
                             const TickType_t waitTime)
{
    return xEventGroupSync(eventGroupHandle, bitsToSet, bitsToWaitFor, waitTime);
}
EventBits_t EventGroup::waitBits(const EventBits_t bitsToWaitFor, const bool clearOnExit,
                                 const bool waitForAll, const TickType_t waitTime)
{
    return xEventGroupWaitBits(eventGroupHandle, bitsToWaitFor, clearOnExit ? pdTRUE : pdFALSE,
                               waitForAll ? pdTRUE : pdFALSE, waitTime);
}
EventGroup::EventGroup(EventGroup &&other) noexcept
{
    (*this) = std::forward<EventGroup>(other);
}
EventGroup &EventGroup::operator=(EventGroup &&other) noexcept
{
    eventGroupHandle = std::exchange(other.eventGroupHandle, nullptr);
    return *this;
}

} // namespace util::wrappers