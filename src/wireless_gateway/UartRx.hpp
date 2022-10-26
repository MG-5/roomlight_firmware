#pragma once

#include "usart.h"

#include "Wifi.hpp"
#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

class UartRx : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    UartRx(UART_HandleTypeDef *uartPeripherie, util::wrappers::StreamBuffer &rxStream)
        : TaskWithMemberFunctionBase("uartRxTask", 64, osPriorityNormal), //
          uartPeripherie(uartPeripherie),                                 //
          rxStream(rxStream)
    {
        // Code assumes that RX dma is circular
        SafeAssert(uartPeripherie->hdmarx != nullptr);
        SafeAssert(uartPeripherie->hdmarx->Init.Mode == DMA_CIRCULAR);
    };

    // This interrupt is called when:
    // - line goes to idle (only fires again when line goes active and inactive again)
    // - DMA is half-way through buffer
    // - DMA is completely through buffer
    void rxEventsFromISR(uint16_t position)
    {
        // indicates that lastIdleLineBufferPosition wasn't reset when reception was restarted or
        // a hardware failure (very rare)
        //
        // when buffer runs over DMA Complete interrupt (redirected to here) will always fire before
        // RxIdleLine could introduce a position lower than lastIdleLineBufferPosition as
        // it (IdleLine) requires a full byte time of silence
        SafeAssert(lastIdleLineBufferPosition <= position);

        if (lastIdleLineBufferPosition == position)
        {
            // on some occasions a transmission will end on full buffer or half buffer causing
            // two calls to this with same positions -> zero new data being available
            return;
        }

        BaseType_t woken = pdFALSE;
        rxStream.sendFromISR(rxRawBuffer + lastIdleLineBufferPosition,
                             position - lastIdleLineBufferPosition, &woken);

        lastIdleLineBufferPosition = position % RxRawBufferSize;

        portYIELD_FROM_ISR(woken);
    }

protected:
    [[noreturn]] void taskMain(void *) override
    {
        vTaskDelay(pdMS_TO_TICKS(250));

        HAL_UARTEx_ReceiveToIdle_DMA(uartPeripherie, rxRawBuffer, RxRawBufferSize);

        vTaskSuspend(nullptr);

        while (true)
        {
        }
    }

private:
    UART_HandleTypeDef *uartPeripherie = nullptr;
    util::wrappers::StreamBuffer &rxStream;

    static constexpr auto RxRawBufferSize = 128;
    uint8_t rxRawBuffer[RxRawBufferSize]{};

    volatile uint16_t lastIdleLineBufferPosition = 0;
};