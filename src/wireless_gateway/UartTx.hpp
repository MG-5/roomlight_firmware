#pragma once

#include "usart.h"

#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

class UartTx : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    UartTx(UART_HandleTypeDef *uartPeripherie, util::wrappers::StreamBuffer &txMessageBuffer)
        : TaskWithMemberFunctionBase("uartTxTask", 64, osPriorityNormal5), //
          uartPeripherie(uartPeripherie),                                  //
          txMessageBuffer(txMessageBuffer){};

    void notifyTxTask()
    {
        auto higherPriorityTaskWoken = pdFALSE;
        notifyGiveFromISR(&higherPriorityTaskWoken);
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }

protected:
    [[noreturn]] void taskMain(void *) override
    {
        while (true)
        {
            auto messageLength = txMessageBuffer.receive(sendBuffer, SendBufferSize, portMAX_DELAY);

            if (messageLength == 0)
            {
                // In case the message to be read out is to large for the receive buffer, we need to
                // reset the whole message buffer since otherwise the message will stay in the
                // buffer and block subsequent readouts.
                txMessageBuffer.reset();
                continue;
            }

            HAL_UART_Transmit_DMA(uartPeripherie, sendBuffer, messageLength);
            waitForTxComplete();
        }
    }

private:
    UART_HandleTypeDef *uartPeripherie = nullptr;
    util::wrappers::StreamBuffer &txMessageBuffer;

    static constexpr auto SendBufferSize = 32;
    uint8_t sendBuffer[SendBufferSize]{};

    void waitForTxComplete()
    {
        notifyTake(portMAX_DELAY);
    }
};