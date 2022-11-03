#pragma once

#include "leds/AddressableLeds.hpp"
#include "leds/LedFading.hpp"

class StateMachine
{
public:
    StateMachine(AddressableLeds &addressableLeds, LedFading &ledFading)
        : addressableLeds(addressableLeds), //
          ledFading(ledFading)              //
          {};

    void updateLedStateOnButtonClick();
    void updateLedStateOnButtonLongPress();
    void updateTargetLeds();

private:
    AddressableLeds &addressableLeds;
    LedFading &ledFading;
};