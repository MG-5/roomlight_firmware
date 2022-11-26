#include "FreeRTOS.h"
#include "main.h"
#include "task.h"

#include "Application.hpp"
#include "core/SafeAssert.h"
#include "wrappers/Task.hpp"

#include <memory>

extern "C" void StartDefaultTask(void *) // NOLINT
{
    static auto app = std::make_unique<Application>();
    app->run();

    SafeAssert(false); // this line should be never reached
}

//--------------------------------------------------------------------------------------------------
Application::Application()
{
    SafeAssert(instance == nullptr);
    instance = this;

    HAL_StatusTypeDef result = HAL_OK;

    // adc stuff
    result = HAL_ADC_RegisterCallback(AdcPeripherie, HAL_ADC_CONVERSION_COMPLETE_CB_ID,
                                      [](ADC_HandleTypeDef *)
                                      { instance->analogToDigital.conversionCompleteCallback(); });
    SafeAssert(result == HAL_OK);

    // addressable LED stuff
    result = HAL_DMA_RegisterCallback(DmaLedTimerChannel1, HAL_DMA_XFER_HALFCPLT_CB_ID,
                                      [](DMA_HandleTypeDef *)
                                      { instance->addressableLeds.transferHalfHandler(); });
    SafeAssert(result == HAL_OK);

    result = HAL_DMA_RegisterCallback(DmaLedTimerChannel1, HAL_DMA_XFER_CPLT_CB_ID,
                                      [](DMA_HandleTypeDef *)
                                      { instance->addressableLeds.transferCompleteHandler(); });
    SafeAssert(result == HAL_OK);

    // uart stuff
    result = HAL_UART_RegisterCallback(EspUartPeripherie, HAL_UART_TX_COMPLETE_CB_ID,
                                       [](UART_HandleTypeDef *) //
                                       { instance->uartTx.notifyTxTask(); });
    SafeAssert(result == HAL_OK);

    result = HAL_UART_RegisterCallback(EspUartPeripherie, HAL_UART_ERROR_CB_ID,
                                       [](UART_HandleTypeDef *) //
                                       { instance->uartRx.uartErrorFromISR(); });
    SafeAssert(result == HAL_OK);

    result = HAL_UART_RegisterRxEventCallback(EspUartPeripherie,                     //
                                              [](UART_HandleTypeDef *, uint16_t pos) //
                                              { instance->uartRx.rxEventsFromISR(pos); });
    SafeAssert(result == HAL_OK);
}

//--------------------------------------------------------------------------------------------------
[[noreturn]] void Application::run()
{
    util::wrappers::Task::applicationIsReadyStartAllTasks();
    while (true)
    {
        vTaskDelay(portMAX_DELAY);
    }
}

//--------------------------------------------------------------------------------------------------
Application &Application::getApplicationInstance()
{
    // Not constructing Application in this singleton, to avoid bugs where something tries to
    // access this function, while application constructs which will cause infinite recursion
    return *instance;
}