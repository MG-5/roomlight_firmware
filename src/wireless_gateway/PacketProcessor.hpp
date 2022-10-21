#pragma once

#include "wrappers/EventGroup.hpp"
#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include "units/si/current.hpp"
#include "units/si/voltage.hpp"

#include "Wifi.hpp"
#include "leds/AddressableLeds.hpp"
#include "leds/LedFading.hpp"

class PacketProcessor : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    PacketProcessor(util::wrappers::StreamBuffer &packetBuffer, Wifi &wifi,
                    AddressableLeds &addressableLeds, LedFading &ledFading,
                    units::si::Voltage &ledVoltage, units::si::Current &ledCurrent)
        : TaskWithMemberFunctionBase("packetProcessor", 128, osPriorityAboveNormal), //
          packetBuffer(packetBuffer),                                                //
          wifi(wifi),                                                                //
          addressableLeds(addressableLeds),                                          //
          ledFading(ledFading),                                                      //
          ledVoltage(ledVoltage),                                                    //
          ledCurrent(ledCurrent)                                                     //
          {};

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    util::wrappers::StreamBuffer &packetBuffer;
    Wifi &wifi;

    AddressableLeds &addressableLeds;
    LedFading &ledFading;

    units::si::Voltage &ledVoltage;
    units::si::Current &ledCurrent;

    static constexpr auto PacketFrameSize = 512 + 64;
    uint8_t packetFrame[PacketFrameSize];
};