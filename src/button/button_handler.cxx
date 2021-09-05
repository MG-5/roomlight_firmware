#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "Button.hpp"
#include "digitalLED.hpp"
#include "helpers/freertos.hpp"
#include "main.h"

#include <array>

using units::si::Time;
using util::Gpio;

QueueHandle_t buttonQueue = xQueueCreate(8, sizeof(Button::Action));

namespace
{
constexpr auto ButtonSamplingInterval = 15.0_ms;
constexpr auto LongPressTime = 500.0_ms;

void buttonCallback(Button::Action action)
{
    xQueueSend(buttonQueue, &action, 0);
}

Button touchButton{{Touch_GPIO_Port, Touch_Pin}, buttonCallback, LongPressTime};

} // namespace

extern "C" void buttonUpdateTask(void *)
{
    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        touchButton.update(ButtonSamplingInterval);

        vTaskDelayUntil(&lastWakeTime, toOsTicks(ButtonSamplingInterval));
    }
}

extern "C" void uiUpdateTask(void *)
{
    while (true)
    {
        Button::Action action;

        xQueueReceive(buttonQueue, &action, portMAX_DELAY);
        switch (action)
        {
        case Button::Action::ShortPress:
        {
            LEDSegment seg;
            bool applyData = true;

            switch (currentLightState)
            {
            case LightState::Off:
                currentLightState = LightState::FullWhite;
                currentLightMode = LightMode::BothStrips;
                seg.white = 255;
                break;

            case LightState::FullWhite:
                currentLightState = LightState::MediumWhite;
                seg.white = 128;
                break;

            case LightState::MediumWhite:
                currentLightState = LightState::LowWhite;
                seg.white = 64;
                break;

            case LightState::LowWhite:
            case LightState::Custom:
                currentLightState = LightState::Off;
                // let seg empty
                break;

            case LightState::System:
            default:
                // block button inputs
                applyData = false;
                break;
            }

            if (applyData)
            {
                for (auto &targetSeg : ledTargetData)
                    targetSeg = seg;

                xTaskNotify(ledFadingHandle, 1, eSetBits);
            }
        }
        break;

        case Button::Action::LongPress:
        {
            if (currentLightMode == LightMode::BothStrips)
            {
                currentLightMode = LightMode::OnlyStrip1;
                xTaskNotify(zeroCheckerHandle, 1, eSetBits);
            }
            else
            {
                currentLightMode = LightMode::BothStrips;
                xTaskNotify(digitalLEDHandle, 1, eSetBits);
            }
        }
        break;

        case Button::Action::StopLongPress:
            break;
        }
    }
}
