#pragma once
#include "protocol.hpp"

namespace Wifi
{
/** WiFi module operation mode */
enum class Mode
{
    Disabled,    //!< Disabled/powered off.
    Programming, //!< Module is in flashing/programming mode running the bootloader.
    Normal       //!< Normal operation mode running our custom firmware.
};

/** Function return codes */
enum class Result
{
    Success = 0,
    Failure,
    Timeout,
    TooShort,
    InvalidResponse
};

Wifi::Mode getMode();

/**
 * @brief Set the mode of the Wifi module.
 *
 * Requires restarting the module if mode differs from the mode the
 * module is currently in unless forceRestart is set to true.
 */
void setMode(Wifi::Mode mode, bool forceRestart = false);

void receiveData(const uint8_t *data, size_t length);
bool checkConnection();
} // namespace Wifi