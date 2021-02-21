#include "FreeRTOS.h"
#include "main.h"
#include "task.h"

#include "StatusLeds.hpp"
#include "digitalLED.hpp"
#include "helpers/freertos.hpp"

#include <array>

namespace
{
constexpr auto TaskFrequency = 25.0_Hz;

std::array leds = {StatusLed{StatusLedMode::Off, {LED_RED_GPIO_Port, LED_RED_Pin}},
                   StatusLed{StatusLedMode::Off, {LED_GREEN1_GPIO_Port, LED_GREEN1_Pin}},
                   StatusLed{StatusLedMode::Off, {LED_GREEN2_GPIO_Port, LED_GREEN2_Pin}},
                   StatusLed{StatusLedMode::Off, {LED_GREEN3_GPIO_Port, LED_GREEN3_Pin}},
                   StatusLed{StatusLedMode::Off, {SW_RED_GPIO_Port, SW_RED_Pin}, true},
                   StatusLed{StatusLedMode::Off, {SW_GREEN_GPIO_Port, SW_GREEN_Pin}, true},
                   StatusLed{StatusLedMode::Off, {SW_BLUE_GPIO_Port, SW_BLUE_Pin}, true}};
} // namespace

StatusLed *ledRed = &leds[0];
StatusLed *ledGreen1 = &leds[1];
StatusLed *ledGreen2 = &leds[2];
StatusLed *ledGreen3 = &leds[3];
StatusLed *ledSwitchRed = &leds[4];
StatusLed *ledSwitchGreen = &leds[5];
StatusLed *ledSwitchBlue = &leds[6];

// -------------------------------------------------------------------------------------------------
void setLed(StatusLed &led, bool state)
{
    led.gpio.write(state ^ led.inverted);
}

// -------------------------------------------------------------------------------------------------
extern "C" void statusLedTask(void *)
{
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        for (auto &led : leds)
        {
            switch (led.mode)
            {
            case StatusLedMode::Off:
                setLed(led, false);
                break;

            case StatusLedMode::On:
                setLed(led, true);
                break;

            case StatusLedMode::Blink0_5Hz:
                setLed(led, lastWakeTime % pdMS_TO_TICKS(2000) / pdMS_TO_TICKS(1000));
                break;

            case StatusLedMode::Blink1Hz:
                setLed(led, lastWakeTime % pdMS_TO_TICKS(1000) / pdMS_TO_TICKS(500));
                break;

            case StatusLedMode::Blink2Hz:
                setLed(led, lastWakeTime % pdMS_TO_TICKS(500) / pdMS_TO_TICKS(250));
                break;

            case StatusLedMode::Blink5Hz:
                setLed(led, lastWakeTime % pdMS_TO_TICKS(200) / pdMS_TO_TICKS(100));
                break;

            case StatusLedMode::Blink10Hz:
                setLed(led, lastWakeTime % pdMS_TO_TICKS(100) / pdMS_TO_TICKS(50));
                break;

            case StatusLedMode::Flash:
            {
                unsigned int ticks = lastWakeTime % pdMS_TO_TICKS(1000) / pdMS_TO_TICKS(100);

                setLed(led, (ticks == 0 || ticks == 2));
            }
            break;
            }
        }

        switch (currentLightState)
        {
        case LightState::Off:
            ledSwitchRed->mode = StatusLedMode::On;
            ledSwitchGreen->mode = StatusLedMode::Off;
            ledSwitchBlue->mode = StatusLedMode::Off;
            break;

        case LightState::FullWhite:
        case LightState::MediumWhite:
        case LightState::LowWhite:
            ledSwitchRed->mode = StatusLedMode::Off;
            ledSwitchGreen->mode = StatusLedMode::On;
            ledSwitchBlue->mode = StatusLedMode::Off;
            break;

        case LightState::Custom:
            ledSwitchRed->mode = StatusLedMode::Off;
            ledSwitchGreen->mode = StatusLedMode::Off;
            ledSwitchBlue->mode = StatusLedMode::On;
            break;

        case LightState::System:
            // do nothing
            break;
        }

        vTaskDelayUntil(&lastWakeTime, toOsTicks(TaskFrequency));
    }
}