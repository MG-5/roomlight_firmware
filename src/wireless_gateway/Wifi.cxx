#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include <climits>
#include <cstring>

#include "Wifi.hpp"

#include "units/si/current.hpp"
#include "units/si/voltage.hpp"

Wifi::Mode Wifi::getMode()
{
    return mode;
}

// -------------------------------------------------------------------------------------------------
void Wifi::setMode(Mode newMode, bool forceRestart)
{
    if (mode == newMode && !forceRestart)
        return;

    espEnable.write(util::Gpio::Low);
    espReset.write(util::Gpio::Low);

    // txMessageBuffer.reset();

    if (newMode == Mode::Programming)
        espGpio0.write(util::Gpio::Low);

    else if (newMode == Mode::Normal)
        espGpio0.write(util::Gpio::High);

    vTaskDelay(pdMS_TO_TICKS(10));

    espEnable.write(util::Gpio::High);
    espReset.write(util::Gpio::High);

    mode = newMode;
}

// -------------------------------------------------------------------------------------------------
void Wifi::sendPacket(const PacketHeader *header, const uint8_t *payload)
{
    std::memcpy(finalFrame, header, sizeof(PacketHeader));
    std::memcpy(finalFrame + sizeof(PacketHeader), payload, header->payloadSize);

    txMessageBuffer.send(finalFrame, sizeof(PacketHeader) + header->payloadSize, 0);
}

// -------------------------------------------------------------------------------------------------
void Wifi::swapSrcDest(uint8_t &val)
{
    uint8_t tmp = val;
    val <<= SrcPos;
    val |= (tmp >> SrcPos) & DestMask;
}

// -------------------------------------------------------------------------------------------------
void Wifi::sendResponsePacket(PacketHeader *const header, const uint8_t *payload)
{
    header->command |= ResponseMask;

    if (!payload)
        header->payloadSize = 0;

    swapSrcDest(header->src_dest);
    sendPacket(header, payload);
}

// -------------------------------------------------------------------------------------------------
bool Wifi::checkConnection()
{
    PacketHeader header;
    header.src_dest = (LedPcb << SrcPos) | EspPcb;
    header.command = EspConnectionTest;
    header.payloadSize = 0;

    wifiEvents.clearBits(EventConnection);

    sendPacket(&header, nullptr);

    EventBits_t uxBits = wifiEvents.waitBits(EventConnection, pdTRUE, pdTRUE, pdMS_TO_TICKS(10));

    return (uxBits & EventConnection);
}

// -------------------------------------------------------------------------------------------------
void Wifi::initWifi()
{
    setMode(Mode::Normal, true);
}

// -------------------------------------------------------------------------------------------------
void Wifi::taskMain(void *)
{
    initWifi();
    bool result = false;

    while (true)
    {
        // ledRed->mode = result ? StatusLedMode::Off : StatusLedMode::Blink1Hz;

        uint32_t notifiedValue;
        notifyWait(0, ULONG_MAX, &notifiedValue, pdMS_TO_TICKS(2000));
        if ((notifiedValue & 1) != 0)
            result = true;

        else
            result = Wifi::checkConnection();
    }
}