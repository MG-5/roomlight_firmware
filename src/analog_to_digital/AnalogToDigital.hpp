#pragma once

#pragma once

#include "units/si/current.hpp"
#include "units/si/frequency.hpp"
#include "units/si/resistance.hpp"
#include "units/si/scalar.hpp"
#include "units/si/voltage.hpp"

#include "core/SafeAssert.h"
#include "wrappers/Task.hpp"

#include <array>

class AnalogToDigital : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    static constexpr auto AdcChannelCount = 2;
    static constexpr auto ReferenceVoltage = 3.3_V;
    static constexpr auto AdcResolution = 4095_;

    // 10k to 1k voltage divider = > (10k + 1k) / 10k = 11 multiplier
    static constexpr auto VoltageDivider = 11.0_;
    static constexpr auto AmplifierGain = 50_;
    static constexpr auto SenseResistance = 5_mOhm;

    AnalogToDigital(ADC_HandleTypeDef *peripherie, units::si::Voltage &ledVoltage,
                    units::si::Current &ledCurrent)
        : TaskWithMemberFunctionBase("adcTask", 128, osPriorityLow2), //
          peripherie(peripherie),                                     //
          ledVoltage(ledVoltage),                                     //
          ledCurrent(ledCurrent)
    {
        SafeAssert(peripherie != nullptr);
    }

    void conversionCompleteCallback();

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    ADC_HandleTypeDef *peripherie = nullptr;

    using RawAdcResultArray = std::array<uint16_t, AdcChannelCount>;
    RawAdcResultArray adcResults{};

    units::si::Voltage &ledVoltage;
    units::si::Current &ledCurrent;

    void calibrateAdc();
    void startConversion();
    void waitUntilConversionFinished();

    template <class T>
    void updateFastLowpass(T &oldValue, const T newSample, const uint8_t sampleCount)
    {
        oldValue += (newSample - oldValue) / static_cast<float>(sampleCount);
    }
};