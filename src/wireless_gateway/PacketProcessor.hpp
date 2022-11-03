#pragma once

#include "wrappers/EventGroup.hpp"
#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include "units/si/current.hpp"
#include "units/si/voltage.hpp"

#include "Wifi.hpp"
#include "leds/AddressableLeds.hpp"
#include "leds/LedFading.hpp"
#include "leds/StateMachine.hpp"

class PacketProcessor : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    PacketProcessor(util::wrappers::StreamBuffer &rxStream, Wifi &wifi,
                    AddressableLeds &addressableLeds, LedFading &ledFading,
                    StateMachine &stateMachine, units::si::Voltage &ledVoltage,
                    units::si::Current &ledCurrent)
        : TaskWithMemberFunctionBase("packetProcessor", 128, osPriorityAboveNormal), //
          rxStream(rxStream),                                                        //
          wifi(wifi),                                                                //
          addressableLeds(addressableLeds),                                          //
          ledFading(ledFading),                                                      //
          stateMachine(stateMachine),                                                //
          ledVoltage(ledVoltage),                                                    //
          ledCurrent(ledCurrent)                                                     //
          {};

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    util::wrappers::StreamBuffer &rxStream;
    Wifi &wifi;

    AddressableLeds &addressableLeds;
    LedFading &ledFading;
    StateMachine &stateMachine;

    units::si::Voltage &ledVoltage;
    units::si::Current &ledCurrent;

    static constexpr auto PacketBufferSize = 1024;
    uint8_t packetBuffer[PacketBufferSize];

    uint16_t bufferStartPosition = 0;
    uint16_t bufferLastPosition = 0;

    uint8_t *payload = nullptr;
    PacketHeader header{};

    bool extractPacketFromBuffer();
};