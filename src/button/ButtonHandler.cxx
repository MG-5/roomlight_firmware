#include "ButtonHandler.hpp"
#include "helpers/freertos.hpp"

using util::Button;

void ButtonHandler::taskMain(void *)
{
    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        touchButton.update(ButtonSamplingInterval);

        vTaskDelayUntil(&lastWakeTime, toOsTicks(ButtonSamplingInterval));
    }
}

void ButtonHandler::buttonCallback(util::Button::Action action)
{
    if (action == util::Button::Action::ShortPress)
        stateMachine.updateLedStateOnButtonClick();

    else if (action == util::Button::Action::LongPress)
        stateMachine.updateLedStateOnButtonLongPress();

    stateMachine.updateTargetLeds();
}