#pragma once

#include "gpio.hpp"
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
    StatusLedMode mode;
    Gpio gpio;
    bool inverted = false;
};