#pragma once

#include "LedBase.hpp"
#include "util/PwmOutput.hpp"
#include <limits>

namespace util::pwm_led
{

//--------------------------------------------------------------------------------------------------
template <typename TimerResolution>
class SingleLed : public LedBase
{

public:
    explicit SingleLed(PwmOutput<TimerResolution> pwmOutput, bool isInverted = false)
        : pwmOutput(pwmOutput)
    {
        LedBase::isInverted = isInverted;
    }

    void turnOnInherited() override
    {
        if (isInverted)
            pwmOutput.setMaximumPwm();
        else
            pwmOutput.setPwmValue(0);
    }

    void turnOffInherited() override
    {
        if (isInverted)
            pwmOutput.setPwmValue(0);
        else
            pwmOutput.setMaximumPwm();
    }

private:
    PwmOutput<TimerResolution> pwmOutput;
};

//--------------------------------------------------------------------------------------------------
enum class DualLedColor
{
    Red,
    Green,
    Yellow,
    Orange
};

//--------------------------------------------------------------------------------------------------
template <typename TimerResolution>
class DualLed : public MultiColorLedBase<DualLedColor>
{
public:
    DualLed(PwmOutput<TimerResolution> ledRedPwmOutput,
            PwmOutput<TimerResolution> ledGreenPwmOutput)
        : ledRedPwmOutput(ledRedPwmOutput), ledGreenPwmOutput(ledGreenPwmOutput){};

private:
    void update() override
    {
        if (isOn)
        {
            switch (currentColor)
            {
            case DualLedColor::Red:
                setMaximumBrightness(ledRedPwmOutput);
                setBrightnessOff(ledGreenPwmOutput);
                break;

            case DualLedColor::Yellow:
                setMaximumBrightness(ledRedPwmOutput);
                setBrightness(std::numeric_limits<TimerResolution>::max() / 3, ledGreenPwmOutput);
                break;

            case DualLedColor::Orange:
                setMaximumBrightness(ledRedPwmOutput);
                setBrightness(std::numeric_limits<TimerResolution>::max() / 8, ledGreenPwmOutput);
                break;

            case DualLedColor::Green:
                setBrightnessOff(ledRedPwmOutput);
                setMaximumBrightness(ledGreenPwmOutput);
                break;

            default:
                setBrightnessOff(ledRedPwmOutput);
                setBrightnessOff(ledGreenPwmOutput);
                break;
            }
        }
        else
        {
            setBrightnessOff(ledRedPwmOutput);
            setBrightnessOff(ledGreenPwmOutput);
        }
    }

    void setBrightnessOff(PwmOutput<TimerResolution> &pwmOutput)
    {
        if (isInverted)
            pwmOutput.setPwmValue(0);
        else
            pwmOutput.setMaximumPwm();
    }

    void setMaximumBrightness(PwmOutput<TimerResolution> &pwmOutput)
    {
        if (isInverted)
            pwmOutput.setMaximumPwm();
        else
            pwmOutput.setPwmValue(0);
    }

    void setBrightness(TimerResolution targetBrightness, PwmOutput<TimerResolution> &pwmOutput)
    {
        if (!isInverted)
            pwmOutput.setPwmValue(targetBrightness);

        else
            pwmOutput.setPwmValue(std::numeric_limits<TimerResolution>::max() - targetBrightness);
    }

    PwmOutput<TimerResolution> ledRedPwmOutput;
    PwmOutput<TimerResolution> ledGreenPwmOutput;
};

//--------------------------------------------------------------------------------------------------
enum class TripleLedColor
{
    Red,
    Green,
    Blue,
    Yellow,
    Orange,
    Purple,
    Turquoise
};

//--------------------------------------------------------------------------------------------------
template <typename TimerResolution>
class TripleLed : public MultiColorLedBase<TripleLedColor>
{
public:
    TripleLed(PwmOutput<TimerResolution> ledRedPwmOutput,
              PwmOutput<TimerResolution> ledGreenPwmOutput,
              PwmOutput<TimerResolution> ledBluePwmOutput, bool isInverted = false)
        : ledRedPwmOutput{ledRedPwmOutput}, ledGreenPwmOutput{ledGreenPwmOutput},
          ledBluePwmOutput{ledBluePwmOutput}
    {
        LedBase::isInverted = isInverted;
    }

private:
    void update() override
    {
        if (isOn)
        {
            switch (currentColor)
            {
            case TripleLedColor::Red:
                setMaximumBrightness(ledRedPwmOutput);
                setBrightnessOff(ledGreenPwmOutput);
                setBrightnessOff(ledBluePwmOutput);
                break;

            case TripleLedColor::Yellow:
                setMaximumBrightness(ledRedPwmOutput);
                setBrightness(std::numeric_limits<TimerResolution>::max() / 3, ledGreenPwmOutput);
                setBrightnessOff(ledBluePwmOutput);
                break;

            case TripleLedColor::Orange:
                setMaximumBrightness(ledRedPwmOutput);
                setBrightness(std::numeric_limits<TimerResolution>::max() / 8, ledGreenPwmOutput);
                setBrightnessOff(ledBluePwmOutput);
                break;

            case TripleLedColor::Green:
                setBrightnessOff(ledRedPwmOutput);
                setMaximumBrightness(ledGreenPwmOutput);
                setBrightnessOff(ledBluePwmOutput);
                break;

            case TripleLedColor::Blue:
                setBrightnessOff(ledRedPwmOutput);
                setBrightnessOff(ledGreenPwmOutput);
                setMaximumBrightness(ledBluePwmOutput);
                break;

            case TripleLedColor::Turquoise:
                setBrightnessOff(ledRedPwmOutput);
                setMaximumBrightness(ledGreenPwmOutput);
                setBrightness(std::numeric_limits<TimerResolution>::max() / 2, ledBluePwmOutput);
                break;

            case TripleLedColor::Purple:
                setMaximumBrightness(ledRedPwmOutput);
                setBrightnessOff(ledGreenPwmOutput);
                setBrightness(std::numeric_limits<TimerResolution>::max() / 2, ledBluePwmOutput);
                break;

            default:
                setBrightnessOff(ledRedPwmOutput);
                setBrightnessOff(ledGreenPwmOutput);
                setBrightnessOff(ledBluePwmOutput);
                break;
            }
        }
        else
        {
            setBrightnessOff(ledRedPwmOutput);
            setBrightnessOff(ledGreenPwmOutput);
            setBrightnessOff(ledBluePwmOutput);
        }
    }

    void setBrightnessOff(PwmOutput<TimerResolution> &pwmOutput)
    {
        if (isInverted)
            pwmOutput.setPwmValue(0);
        else
            pwmOutput.setMaximumPwm();
    }

    void setMaximumBrightness(PwmOutput<TimerResolution> &pwmOutput)
    {
        if (isInverted)
            pwmOutput.setMaximumPwm();
        else
            pwmOutput.setPwmValue(0);
    }

    void setBrightness(TimerResolution targetBrightness, PwmOutput<TimerResolution> &pwmOutput)
    {
        if (!isInverted)
            pwmOutput.setPwmValue(targetBrightness);

        else
            pwmOutput.setPwmValue(std::numeric_limits<TimerResolution>::max() - targetBrightness);
    }

    PwmOutput<TimerResolution> ledRedPwmOutput;
    PwmOutput<TimerResolution> ledGreenPwmOutput;
    PwmOutput<TimerResolution> ledBluePwmOutput;
};

} // namespace util::pwm_led