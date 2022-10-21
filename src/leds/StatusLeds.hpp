#pragma once

#include "AddressableLeds.hpp"
#include "util/BinaryLed.hpp"
#include "util/PwmLed.hpp"
#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

#include "tim.h"

using util::PwmOutput8Bit;
using util::binary_led::SingleLed;
using util::pwm_led::TripleLed;

class StatusLeds : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    explicit StatusLeds(AddressableLeds &addrLeds)
        : TaskWithMemberFunctionBase("statusLedsTask", 128, osPriorityLow4), //
          addrLeds(addrLeds){};

    SingleLed ledRed{{LED_RED_GPIO_Port, LED_RED_Pin}};
    SingleLed ledGreen1{{LED_GREEN1_GPIO_Port, LED_GREEN1_Pin}};
    SingleLed ledGreen2{{LED_GREEN2_GPIO_Port, LED_GREEN2_Pin}};
    SingleLed ledGreen3{{LED_GREEN3_GPIO_Port, LED_GREEN3_Pin}};

    static constexpr PwmOutput8Bit RedChannel{&htim3, TIM_CHANNEL_2};
    static constexpr PwmOutput8Bit GreenChannel{&htim3, TIM_CHANNEL_1};
    static constexpr PwmOutput8Bit BlueChannel{&htim2, TIM_CHANNEL_1};
    TripleLed<uint8_t> switchLed{RedChannel, GreenChannel, BlueChannel};

protected:
    void taskMain(void *) override;

private:
    AddressableLeds &addrLeds;

    void updateStates();
    void updateTimes(TickType_t currentTicks);
};
