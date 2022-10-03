#include "wrappers/StreamBuffer.hpp"
#include <utility>

namespace util::wrappers
{

StreamBuffer::StreamBuffer(const size_t BufferSize, const size_t TriggerLevel)
    : streamHandle(xStreamBufferCreate(BufferSize, TriggerLevel))
{
    SafeAssert(streamHandle != nullptr);
}

StreamBuffer::~StreamBuffer()
{
    if (streamHandle != nullptr)
    {
        vStreamBufferDelete(streamHandle);
    }
}

bool StreamBuffer::isEmpty()
{
    return xStreamBufferIsEmpty(streamHandle) == pdTRUE;
}

bool StreamBuffer::isFull()
{
    return xStreamBufferIsFull(streamHandle) == pdTRUE;
}

size_t StreamBuffer::receive(std::span<uint8_t> buffer, const TickType_t ticksToWait)
{
    return xStreamBufferReceive(streamHandle, buffer.data(), buffer.size(), ticksToWait);
}

size_t StreamBuffer::receiveFromISR(std::span<uint8_t> buffer,
                                    BaseType_t *pxHigherPriorityTaskWoken)
{
    return xStreamBufferReceiveFromISR(streamHandle, buffer.data(), buffer.size(),
                                       pxHigherPriorityTaskWoken);
}

bool StreamBuffer::reset()
{
    // this is not interrupt safe for some versions of freertos
    // https://forums.freertos.org/t/xstreambufferreset-is-not-interrupt-safe/7202/2
    portENTER_CRITICAL();
    const auto res = xStreamBufferReset(streamHandle) == pdPASS;
    portEXIT_CRITICAL();
    return res;
}

size_t StreamBuffer::send(uint8_t *buffer, const size_t bufferSize const TickType_t ticksToWait)
{
    return xStreamBufferSend(streamHandle, buffer, bufferSize, ticksToWait);
}

size_t StreamBuffer::sendFromISR(uint8_t *buffer, size_t bufferSize,
                                 BaseType_t *pxHigherPriorityTaskWoken)
{
    return xStreamBufferSendFromISR(streamHandle, buffer, bufferSize, pxHigherPriorityTaskWoken);
}

bool StreamBuffer::setTriggerLevel(const size_t triggerLevel)
{
    return xStreamBufferSetTriggerLevel(streamHandle, triggerLevel) == pdTRUE;
}

size_t StreamBuffer::getSpaceAvailable()
{
    return xStreamBufferSpacesAvailable(streamHandle);
}

StreamBuffer::StreamBuffer(StreamBuffer &&other) noexcept
{
    (*this) = std::forward<StreamBuffer>(other);
}

StreamBuffer &StreamBuffer::operator=(StreamBuffer &&other) noexcept
{
    streamHandle = std::exchange(other.streamHandle, nullptr);
    return *this;
}

size_t StreamBuffer::getBytesAvailable()
{
    return xStreamBufferBytesAvailable(streamHandle);
}

} // namespace util::wrappers