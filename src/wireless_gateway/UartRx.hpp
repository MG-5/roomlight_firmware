#pragma once

#include "usart.h"

#include "Wifi.hpp"
#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

class UartRx : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    UartRx(UART_HandleTypeDef *uartPeripherie, Wifi &wifi)
        : TaskWithMemberFunctionBase("uartRxTask", 128, osPriorityNormal), //
          uartPeripherie(uartPeripherie),                                  //
          wifi(wifi){};

    void notifyRxTask()
    {
        auto higherPriorityTaskWoken = pdFALSE;
        notifyGiveFromISR(&higherPriorityTaskWoken);
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }

protected:
    [[noreturn]] void taskMain(void *) override
    {
        vTaskDelay(pdMS_TO_TICKS(250));

        __HAL_UART_ENABLE_IT(uartPeripherie, UART_IT_IDLE);
        HAL_UART_Receive_DMA(uartPeripherie, rxBuffer, RxBufferSize);
        // HAL_UARTEx_ReceiveToIdle_DMA(uartPeripherie, rxBuffer, RxBufferSize);

        while (true)
        {
            waitForRxComplete();
            checkRx();
        }
    }

private:
    UART_HandleTypeDef *uartPeripherie = nullptr;
    Wifi &wifi;

    static constexpr auto RxBufferSize = 256;
    uint8_t rxBuffer[RxBufferSize]{};

    void waitForRxComplete()
    {
        notifyTake(portMAX_DELAY);
    }

    void checkRx()
    {
        static size_t previousBufferPosition = 0;
        volatile auto dmaCounter = __HAL_DMA_GET_COUNTER(uartPeripherie->hdmarx);
        size_t bufferPosition = RxBufferSize - dmaCounter;

        if (bufferPosition == previousBufferPosition)
            return;

        if (bufferPosition > previousBufferPosition)
        {
            wifi.receiveData(rxBuffer + previousBufferPosition,
                             bufferPosition - previousBufferPosition);
        }
        else
        {
            wifi.receiveData(rxBuffer + previousBufferPosition,
                             RxBufferSize - previousBufferPosition);

            if (bufferPosition > 0)
                wifi.receiveData(rxBuffer, bufferPosition);
        }

        previousBufferPosition = bufferPosition;

        if (previousBufferPosition == RxBufferSize)
            previousBufferPosition = 0;
    }
};