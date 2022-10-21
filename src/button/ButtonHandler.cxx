#include "ButtonHandler.hpp"
#include "helpers/freertos.hpp"
#include "protocol.hpp"

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
    switch (action)
    {
    case Button::Action::ShortPress:
    {
        LedSegment seg;

        switch (addressableLeds.getLightState())
        {
        case AddressableLeds::LightState::Off:
            addressableLeds.setLightState(AddressableLeds::LightState::FullWhite);
            addressableLeds.setLightMode(AddressableLeds::LightMode::BothStrips);
            seg.white = 255;
            break;

        case AddressableLeds::LightState::FullWhite:
            addressableLeds.setLightState(AddressableLeds::LightState::MediumWhite);
            seg.white = 128;
            break;

        case AddressableLeds::LightState::MediumWhite:
            addressableLeds.setLightState(AddressableLeds::LightState::LowWhite);
            seg.white = 64;
            break;

        case AddressableLeds::LightState::LowWhite:
        case AddressableLeds::LightState::Custom:
            addressableLeds.setLightState(AddressableLeds::LightState::Off);
            // let seg empty
            break;

        case AddressableLeds::LightState::System:
        default:
            // block button inputs
            return;
        }

        for (auto &targetSeg : ledFading.getTargetArray())
            targetSeg = seg;

        ledFading.notify(1, util::wrappers::NotifyAction::SetBits);
    }
    break;

    case Button::Action::LongPress:
    {
        if (addressableLeds.getLightMode() == AddressableLeds::LightMode::BothStrips)
        {
            addressableLeds.setLightMode(AddressableLeds::LightMode::OnlyStrip1);
            addressableLeds.notify(1, util::wrappers::NotifyAction::SetBits);
        }
        else
        {
            addressableLeds.setLightMode(AddressableLeds::LightMode::BothStrips);
            addressableLeds.notify(1, util::wrappers::NotifyAction::SetBits);
        }
    }
    break;

    case Button::Action::StopLongPress:
        break;
    }
}