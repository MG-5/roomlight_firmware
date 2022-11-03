#pragma once

#include "gcem.hpp"
#include "leds/StateMachine.hpp"
#include "util/Button.hpp"
#include "wrappers/Queue.hpp"
#include "wrappers/Task.hpp"

#include "main.h"

class ButtonHandler : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    static constexpr auto ButtonSamplingInterval = 15.0_ms;
    static constexpr auto MinimumPulseLength = 30.0_ms;

    explicit ButtonHandler(StateMachine &stateMachine)
        : TaskWithMemberFunctionBase("buttonTask", 128, osPriorityNormal1), //
          stateMachine(stateMachine)
    {
        constexpr auto Divisor = (MinimumPulseLength / ButtonSamplingInterval).getMagnitude();
        static_assert(gcem::ceil(Divisor) == Divisor,
                      "The sampling time is not an even divisor of minimum pulse length!");
    };

protected:
    void taskMain(void *) override;

private:
    StateMachine &stateMachine;

    void buttonCallback(util::Button::Action action);
    void updateLedState(util::Button::Action &action);
    void setTargetLeds();

    util::Gpio touchButtonGpio{Touch_GPIO_Port, Touch_Pin};
    util::Button touchButton{touchButtonGpio,
                             [this](util::Button::Action action) { buttonCallback(action); }, true};
};