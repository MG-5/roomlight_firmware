#pragma once
#include <FreeRTOS.h>
#include <core/SafeAssert.h>
#include <stream_buffer.h>

namespace util::wrappers
{
/// C++ wrapper for freertos StreamBuffer
class StreamBuffer
{
public:
    StreamBuffer(size_t BufferSize, size_t TriggerLevel);
    ~StreamBuffer();

    StreamBuffer(const StreamBuffer &) = delete;
    StreamBuffer(StreamBuffer &&) noexcept;
    void operator=(const StreamBuffer &) = delete;
    StreamBuffer &operator=(StreamBuffer &&) noexcept;

    bool isEmpty();
    bool isFull();
    size_t receive(uint8_t *buffer, size_t bufferSize, TickType_t ticksToWait);
    size_t receiveFromISR(uint8_t *buffer, size_t bufferSize,
                          BaseType_t *pxHigherPriorityTaskWoken);
    bool reset();
    size_t send(uint8_t *buffer, size_t bufferSize, TickType_t ticksToWait);
    size_t sendFromISR(uint8_t *buffer, size_t bufferSize, BaseType_t *pxHigherPriorityTaskWoken);
    bool setTriggerLevel(size_t);
    size_t getSpaceAvailable();
    size_t getBytesAvailable();

private:
    StreamBufferHandle_t streamHandle{nullptr};
};
} // namespace util::wrappers