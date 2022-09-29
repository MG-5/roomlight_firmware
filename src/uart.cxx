#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "message_buffer.h"
#include "task.h"
#include "usart.h"

#include "Wifi.hpp"
#include "main.h"
#include <array>
#include <cstring>

extern osThreadId uartRXHandle;
extern osThreadId uartTXHandle;

constexpr auto TxMessageBufferSize = 64;
constexpr auto SendBufferSize = 32;
constexpr auto RxBufferSize = 256;

auto txMessageBuffer = xMessageBufferCreate(TxMessageBufferSize);
uint8_t sendBuffer[SendBufferSize];
uint8_t rxBuffer[RxBufferSize];

void uartSendData(const uint8_t *data, size_t length)
{
    xMessageBufferSend(txMessageBuffer, data, length, 0);
}

void notifyUartRXTask()
{
    auto higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(uartRXHandle, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

extern "C" void uartIdleCallback()
{
    notifyUartRXTask();
}

extern "C" void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    notifyUartRXTask();
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    notifyUartRXTask();
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    auto higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(uartTXHandle, &higherPriorityTaskWoken);
}

void waitForRXComplete()
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void waitForTXComplete()
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void checkRX()
{
    static size_t previousBufferPosition = 0;
    volatile auto dmaCounter = __HAL_DMA_GET_COUNTER(huart2.hdmarx);
    size_t bufferPosition = RxBufferSize - dmaCounter;

    if (bufferPosition == previousBufferPosition)
        return;

    if (bufferPosition > previousBufferPosition)
    {
        Wifi::receiveData(rxBuffer + previousBufferPosition,
                          bufferPosition - previousBufferPosition);
    }
    else
    {

        Wifi::receiveData(rxBuffer + previousBufferPosition, RxBufferSize - previousBufferPosition);

        if (bufferPosition > 0)
        {

            Wifi::receiveData(rxBuffer, bufferPosition);
        }
    }

    previousBufferPosition = bufferPosition;

    if (previousBufferPosition == RxBufferSize)
    {
        previousBufferPosition = 0;
    }
}

extern "C" void uartRXTask(void *)
{
    vTaskSuspend(nullptr);
    vTaskDelay(pdMS_TO_TICKS(250));

    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart2, rxBuffer, RxBufferSize);

    while (true)
    {
        waitForRXComplete();
        checkRX();
    }
}

extern "C" void uartTXTask(void *)
{
    vTaskSuspend(nullptr);
    while (true)
    {
        auto messageLength =
            xMessageBufferReceive(txMessageBuffer, sendBuffer, SendBufferSize, portMAX_DELAY);

        if (messageLength == 0)
        {
            // In case the message to be read out is to large for the receive buffer, we need to
            // reset the whole message buffer since otherwise the message will stay in the buffer
            // and block subsequent readouts.
            xMessageBufferReset(txMessageBuffer);
            continue;
        }
        HAL_UART_Transmit_DMA(&huart2, sendBuffer, messageLength);
        waitForTXComplete();
    }
}
