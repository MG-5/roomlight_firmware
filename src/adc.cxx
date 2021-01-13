#include "adc.h"
#include "FreeRTOS.h"
#include "task.h"

#include "defines.hpp"
#include <array>

extern TaskHandle_t adcHandle;

namespace
{
constexpr auto AdcChannelCount = 2;
constexpr auto ReferenceVoltage = 3.3;
constexpr auto AdcResolution = 12;
constexpr auto MaxAdcValue = (1 << AdcResolution) - 1;

constexpr auto AmplifierGain = 50;
constexpr auto Rsense = 5; // in mOhm

using RawAdcResultArray = std::array<uint16_t, AdcChannelCount>;
RawAdcResultArray adcResults;
} // namespace

void calibrateAdc()
{
    HAL_ADCEx_Calibration_Start(&hadc1);
}

void startConversion()
{
    HAL_ADC_Start_DMA(&hadc1, reinterpret_cast<uint32_t *>(adcResults.data()), AdcChannelCount);
}

void waitUntilConversionFinished()
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *)
{
    auto higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(adcHandle, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

extern "C" void adcTask(void *)
{
    auto lastWakeTime = xTaskGetTickCount();

    calibrateAdc();

    while (1)
    {
        startConversion();
        waitUntilConversionFinished();

        // 10k to 1k voltage divider = > (10k + 1k) / 10k = 11 multiplier
        ledVoltage = (adcResults.at(0) * 3.3f * 11.0f) / MaxAdcValue;

        ledCurrent =
            (adcResults.at(1) * 3.3f * 1000) / (MaxAdcValue * AmplifierGain * Rsense); // in mA

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(20));
    }
}