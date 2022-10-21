#include "StatusLeds.hpp"

using util::pwm_led::TripleLedColor;

void StatusLeds::taskMain(void *)
{
    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        updateStates();
        updateTimes(lastWakeTime);

        constexpr auto TaskFrequency = 25.0_Hz;
        vTaskDelayUntil(&lastWakeTime, toOsTicks(TaskFrequency));
    }
}

void StatusLeds::updateStates()
{
    switch (addrLeds.getLightState())
    {
    case AddressableLeds::LightState::Off:
        switchLed.setColor(TripleLedColor::Red);
        break;

    case AddressableLeds::LightState::FullWhite:
    case AddressableLeds::LightState::MediumWhite:
    case AddressableLeds::LightState::LowWhite:
        switchLed.setColor(TripleLedColor::Green);
        break;

    case AddressableLeds::LightState::Custom:
        switchLed.setColor(TripleLedColor::Blue);
        break;

    case AddressableLeds::LightState::System:
        // do nothing
        break;
    }
}

void StatusLeds::updateTimes(TickType_t currentTicks)
{
    ledRed.updateState(currentTicks);
    ledGreen1.updateState(currentTicks);
    ledGreen2.updateState(currentTicks);
    ledGreen3.updateState(currentTicks);
    switchLed.updateState(currentTicks);
}