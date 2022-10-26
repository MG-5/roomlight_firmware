#pragma once
#include "LedBase.hpp"
#include "util/gpio.hpp"

namespace util::binary_led
{
//--------------------------------------------------------------------------------------------------
class SingleLed : public LedBase
{
public:
    explicit SingleLed(Gpio gpio, bool isInverted = false) : ledGpio{gpio}
    {
        LedBase::isInverted = isInverted;
    }

private:
    void update() override
    {
        ledGpio.write(isOn ^ isInverted);
    }

    Gpio ledGpio;
};

enum class DualLedColor
{
    Red,
    Green,
    Yellow
};

//--------------------------------------------------------------------------------------------------

/// only for red and green LEDs in bundle
class DualLed : public MultiColorLedBase<DualLedColor>
{
public:
    DualLed(Gpio ledRedGpio, Gpio ledGreenGpio, bool isInverted = false)
        : ledRedGpio{ledRedGpio}, ledGreenGpio{ledGreenGpio}
    {
        LedBase::isInverted = isInverted;
    }

private:
    void update() override
    {
        if (isOn ^ isInverted)
        {
            ledRedGpio.write(
                (currentColor == DualLedColor::Red || currentColor == DualLedColor::Yellow));
            ledGreenGpio.write(
                (currentColor == DualLedColor::Green || currentColor == DualLedColor::Yellow));
        }
        else
        {
            ledRedGpio.write(Gpio::Low);
            ledGreenGpio.write(Gpio::Low);
        }
    }

    Gpio ledRedGpio;
    Gpio ledGreenGpio;
};
} // namespace util::binary_led
