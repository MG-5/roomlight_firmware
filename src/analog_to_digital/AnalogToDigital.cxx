#include "FreeRTOS.h"
#include "adc.h"
#include "task.h"

#include "AnalogToDigital.hpp"
#include "units/si/current.hpp"
#include "units/si/resistance.hpp"
#include "units/si/scalar.hpp"
#include "units/si/voltage.hpp"
#include <array>

void AnalogToDigital::calibrateAdc()
{
    HAL_ADCEx_Calibration_Start(&hadc1);
}

void AnalogToDigital::startConversion()
{
    HAL_ADC_Start_DMA(&hadc1, reinterpret_cast<uint32_t *>(adcResults.data()), AdcChannelCount);
}

void AnalogToDigital::waitUntilConversionFinished()
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void AnalogToDigital::conversionCompleteCallback()
{
    auto higherPriorityTaskWoken = pdFALSE;
    notifyFromISR(1, util::wrappers::NotifyAction::SetBits, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void AnalogToDigital::taskMain(void *)
{
    auto lastWakeTime = xTaskGetTickCount();

    calibrateAdc();

    while (true)
    {
        startConversion();
        waitUntilConversionFinished();

        ledVoltage = (adcResults[0] * ReferenceVoltage * VoltageDivider) / AdcResolution;

        ledCurrent =
            (adcResults[1] * ReferenceVoltage) / (AdcResolution * AmplifierGain * SenseResistance);

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(20));
    }
}