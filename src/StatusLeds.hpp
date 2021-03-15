#pragma once

#include "util/gpio.hpp"
#include <cstdint>

// Lighting mode of a status LED
enum class StatusLedMode
{
    Off = 0,
    On,
    Blink0_5Hz,
    Blink1Hz,
    Blink2Hz,
    Blink5Hz,
    Blink10Hz,
    Flash
};

// Configuration of a single status LEDs
struct StatusLed
{
    StatusLedMode mode = StatusLedMode::Off;
    util::Gpio gpio;
    bool inverted = false;
};

extern StatusLed *ledRed;
extern StatusLed *ledGreen1;
extern StatusLed *ledGreen2;
extern StatusLed *ledGreen3;