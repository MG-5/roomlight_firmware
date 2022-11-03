#include "PacketProcessor.hpp"
#include "leds/ledConstants.hpp"
#include "protocol.hpp"

#include <cstring>

void PacketProcessor::taskMain(void *)
{
    while (true)
    {
        if (!extractPacketFromBuffer())
            continue;

        /*
        if (header.command & ResponseMask)
        {
            header.command &= ~ResponseMask; // remove bit for further processing
            switch (header.command)
            {
            case EspConnectionTest:
                if (header.status == ResponseOkay)
                    wifi.wifiEvents.setBits(Wifi::EventConnection);
                break;

            case EspGetMacAddress:
            default:
                // do nothing with unsupported repsonses
                break;
            }
            continue;
        }
        */

        switch (header.command)
        {
        case command::FullLedData:
            /*
            if (header.payloadSize == 4 * TotalPixels)
            {
                std::memcpy(reinterpret_cast<uint8_t *>(ledFading.getTargetArray().data()), payload,
                            header.payloadSize);
                header.status = ResponseOkay;
            }
            else
                header.status = ResponseOperationFailed;
            */
            break;

        case command::SetSegment:
            /*
            if (header.payloadSize == sizeof(SetLedSegment))
            {
                auto *segmentToSet = reinterpret_cast<SetLedSegment *>(payload);
                ledFading.getTargetArray()[segmentToSet->position] = segmentToSet->segment;
                header.status = ResponseOkay;
            }
            else
                header.status = ResponseOperationFailed;
            */
            break;

        case command::FadeSoft:
            // header.status = ResponseOkay;
            addressableLeds.setLightState(AddressableLeds::LightState::Custom);
            ledFading.notify(1, util::wrappers::NotifyAction::SetBits);
            break;

        case command::FadeHard:
            std::memcpy(addressableLeds.getCurrentLedArray().data(),
                        ledFading.getTargetArray().data(), TotalPixels * sizeof(uint32_t));
            // header.status = ResponseOkay;
            addressableLeds.setLightState(AddressableLeds::LightState::Custom);
            addressableLeds.notify(1, util::wrappers::NotifyAction::SetBits);
            break;

        case command::WarmWhite:
            addressableLeds.setWarmWhite(true);
            stateMachine.updateTargetLeds();
            break;

        case command::NeutralWhite:
            addressableLeds.setWarmWhite(false);
            stateMachine.updateTargetLeds();
            break;

        case command::AllStrips:
            addressableLeds.setLightMode(AddressableLeds::LightMode::BothStrips);
            stateMachine.updateTargetLeds();
            break;

        case command::OneStrips:
            addressableLeds.setLightMode(AddressableLeds::LightMode::OnlyStrip1);
            stateMachine.updateTargetLeds();
            break;

        case command::LightsLow:
            addressableLeds.setLightState(AddressableLeds::LightState::LowWhite);
            stateMachine.updateTargetLeds();
            break;

        case command::LightsMedium:
            addressableLeds.setLightState(AddressableLeds::LightState::MediumWhite);
            stateMachine.updateTargetLeds();
            break;

        case command::LightsHigh:
            addressableLeds.setLightState(AddressableLeds::LightState::FullWhite);
            stateMachine.updateTargetLeds();
            break;

        case command::LightsOff:
            addressableLeds.setLightState(AddressableLeds::LightState::Off);
            stateMachine.updateTargetLeds();
            break;

        case command::GetCurrent:
            // header.status = ResponseOkay;
            header.payloadSize = sizeof(ledCurrent);
            wifi.sendResponsePacket(&header, reinterpret_cast<uint8_t *>(&ledCurrent));
            continue;
            break;

        case command::GetVoltage:
            // header.status = ResponseOkay;
            header.payloadSize = sizeof(ledVoltage);
            wifi.sendResponsePacket(&header, reinterpret_cast<uint8_t *>(&ledVoltage));
            continue;

        case command::GetErrorCode:
            // header.status = ResponseNotSupported;
            break;

        case command::EspOtaStarted:
            wifi.wifiEvents.setBits(Wifi::EventWifiUpgradeStarted);
            // do not disturb ESP
            continue;

        case command::EspOtaFinished:
            wifi.wifiEvents.setBits(Wifi::EventWifiUpgradeFinished);
            continue;

        default:
            // header.status = ResponseInvalidCommand;
            break;
        }
        wifi.sendResponsePacket(&header, nullptr);
    }
}

bool PacketProcessor::extractPacketFromBuffer()
{
    payload = nullptr;

    if (bufferStartPosition == bufferLastPosition)
    {
        bufferStartPosition = 0;
        bufferLastPosition = 0;
    }

    const auto NumberOfBytes = rxStream.receive(
        packetBuffer + bufferLastPosition, PacketBufferSize - bufferLastPosition, portMAX_DELAY);

    bufferLastPosition += NumberOfBytes;

    if (bufferLastPosition >= PacketBufferSize)
    {
        bufferStartPosition = 0;
        bufferLastPosition = 0;
    }

    while (true)
    {
        if (bufferLastPosition - bufferStartPosition < sizeof(PacketHeader))
        {
            // no header to read in buffer
            // wait for new bytes from UART
            return false;
        }

        std::memcpy(&header, packetBuffer + bufferStartPosition, sizeof(PacketHeader));

        if (header.magic == ProtocolMagic)
            break; // header found

        // no valid packet header found in buffer
        // get rid the first byte in buffer and try it again
        bufferStartPosition++;
    }

    if (header.payloadSize > (bufferLastPosition - bufferStartPosition) - sizeof(PacketHeader))
    {
        // excepted payload size is greater than the content in buffer
        // wait for new bytes from UART
        return false;
    }

    if (header.payloadSize > 0)
        payload = packetBuffer + bufferStartPosition + sizeof(PacketHeader);

    bufferStartPosition += sizeof(PacketHeader) + header.payloadSize;
    return true;
}