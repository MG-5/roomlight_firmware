#include "FreeRTOS.h"
#include "main.h"
#include "task.h"
#include <array>

#include "StatusLeds.hpp"

constexpr auto LEDS_TASK_DELAY = 20;

std::array leds = {
    // red
    StatusLed{StatusLedMode::Off, {LED_RED_GPIO_Port, LED_RED_Pin}},

    // green1
    StatusLed{StatusLedMode::Off, {LED_GREEN1_GPIO_Port, LED_GREEN1_Pin}},

    // green2
    StatusLed{StatusLedMode::Off, {LED_GREEN2_GPIO_Port, LED_GREEN2_Pin}},

    // green3
    StatusLed{StatusLedMode::Off, {LED_GREEN3_GPIO_Port, LED_GREEN3_Pin}}};

StatusLed *ledRed = &leds[0];
StatusLed *ledGreen1 = &leds[1];
StatusLed *ledGreen2 = &leds[2];
StatusLed *ledGreen3 = &leds[3];

void setLed(StatusLed &led, bool state)
{
    led.gpio.write(state ^ led.inverted);
}

extern "C" void statusLedTask(void *)
{
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        for (auto &led : leds)
        {
            switch (led.mode)
            {
            default:
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
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(LEDS_TASK_DELAY));
    }
}