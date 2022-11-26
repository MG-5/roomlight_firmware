#include "StateMachine.hpp"

void StateMachine::updateLedStateOnButtonClick()
{
    switch (addressableLeds.getLightState())
    {
    case AddressableLeds::LightState::Off:
        addressableLeds.setWarmWhite(true);
        addressableLeds.setLightState(AddressableLeds::LightState::FullWhite);
        addressableLeds.setLightMode(AddressableLeds::LightMode::BothStrips);
        break;

    case AddressableLeds::LightState::FullWhite:
        addressableLeds.setLightState(AddressableLeds::LightState::MediumWhite);
        break;

    case AddressableLeds::LightState::MediumWhite:
        addressableLeds.setLightState(AddressableLeds::LightState::LowWhite);
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
}

// -------------------------------------------------------------------------------------------------
void StateMachine::updateLedStateOnButtonLongPress()
{
    if (addressableLeds.getLightState() == AddressableLeds::LightState::Off)
    {
        addressableLeds.setLightMode(AddressableLeds::LightMode::BothStrips);
        addressableLeds.setWarmWhite(false);
        addressableLeds.setLightState(AddressableLeds::LightState::FullWhite);
    }
    else if (addressableLeds.getLightMode() == AddressableLeds::LightMode::BothStrips)
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

// -------------------------------------------------------------------------------------------------
void StateMachine::updateTargetLeds()
{
    if (addressableLeds.getLightState() == AddressableLeds::LightState::Custom ||
        addressableLeds.getLightState() == AddressableLeds::LightState::System)
        return;

    LedSegment seg; // empty segment

    if (addressableLeds.getLightState() != AddressableLeds::LightState::Off)
    {
        seg.white = 255;

        if (!addressableLeds.isWarmWhite())
        {
            seg.blue = 170;
            seg.green = 60;
        }
    }

    if (addressableLeds.getLightState() == AddressableLeds::LightState::MediumWhite)
    {
        seg.white /= 2;
        seg.blue /= 2;
        seg.green /= 2;
    }
    else if (addressableLeds.getLightState() == AddressableLeds::LightState::LowWhite)
    {
        seg.white /= 4;
        seg.blue /= 4;
        seg.green /= 4;
    }

    for (auto &ledSegment : ledFading.getTargetArray())
        ledSegment = seg;

    ledFading.notify(1, util::wrappers::NotifyAction::SetBits);
}