#pragma once
#include "protocol.hpp"

namespace Wifi
{
enum class Mode
{
    Disabled,
    Programming,
    Normal
};

enum class Result
{
    Success = 0,
    Failure,
    Timeout,
    TooShort,
    InvalidResponse
};

Wifi::Mode getMode();

/// Set the mode of the Wifi module.
/// Requires restarting the module if mode differs from the mode the
/// module is currently in unless forceRestart is set to true.
void setMode(Wifi::Mode mode, bool forceRestart = false);

void receiveData(const uint8_t *data, uint32_t length);
bool checkConnection();
} // namespace Wifi