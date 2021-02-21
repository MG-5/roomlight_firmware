#include "adc.h"
#include "FreeRTOS.h"
#include "task.h"

#include "units/si/current.hpp"
#include "units/si/resistance.hpp"
#include "units/si/scalar.hpp"
#include "units/si/voltage.hpp"
#include <array>

units::si::Voltage ledVoltage{0.0_V};
units::si::Current ledCurrent{0.0_A};

extern TaskHandle_t adcHandle;

namespace
{
constexpr auto AdcChannelCount = 2;
constexpr auto ReferenceVoltage = 3.3_V;
constexpr auto AdcResolution = 4095_;

// 10k to 1k voltage divider = > (10k + 1k) / 10k = 11 multiplier
constexpr auto VoltageDivider = 11.0_;
constexpr auto AmplifierGain = 50_;
constexpr auto SenseResistance = 5_mOhm;

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