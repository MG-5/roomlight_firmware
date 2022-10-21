#include "PacketProcessor.hpp"
#include "leds/ledConstants.hpp"

#include <cstring>

void PacketProcessor::taskMain(void *)
{
    uint8_t *payload = nullptr;

    while (true)
    {
        payload = nullptr;
        auto length = packetBuffer.receive(packetFrame, PacketFrameSize, portMAX_DELAY);

        if (length == 0)
        {
            packetBuffer.reset();
            continue;
        }

        PacketHeader header;
        std::memcpy(&header, packetFrame, sizeof(PacketHeader));

        if ((header.src_dest & DestMask) != LedPcb)
            continue; // packet is not relevant to us

        if (header.payloadSize > 0)
            payload = packetFrame + sizeof(PacketHeader);

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

        switch (header.command)
        {
        case LedSetAll:
            if (header.payloadSize == 4 * TotalPixels)
            {
                std::memcpy(reinterpret_cast<uint8_t *>(ledFading.getTargetArray().data()), payload,
                            header.payloadSize);
                header.status = ResponseOkay;
            }
            else
                header.status = ResponseOperationFailed;
            break;

        case LedSetSegments:
            if (header.payloadSize == sizeof(SetLedSegment))
            {
                auto *segmentToSet = reinterpret_cast<SetLedSegment *>(payload);
                ledFading.getTargetArray()[segmentToSet->position] = segmentToSet->segment;
                header.status = ResponseOkay;
            }
            else
                header.status = ResponseOperationFailed;
            break;

        case LedFadeSoft:
            header.status = ResponseOkay;
            addressableLeds.setLightState(AddressableLeds::LightState::Custom);
            ledFading.notify(1, util::wrappers::NotifyAction::SetBits);
            break;

        case LedFadeHard:
            std::memcpy(addressableLeds.getCurrentLedArray().data(),
                        ledFading.getTargetArray().data(), TotalPixels * sizeof(uint32_t));
            header.status = ResponseOkay;
            addressableLeds.setLightState(AddressableLeds::LightState::Custom);
            addressableLeds.notify(1, util::wrappers::NotifyAction::SetBits);
            break;

        case LedGetCurrent:
            header.status = ResponseOkay;
            header.payloadSize = sizeof(ledCurrent);
            wifi.sendResponsePacket(&header, reinterpret_cast<uint8_t *>(&ledCurrent));
            continue;
            break;

        case LedGetVoltage:
            header.status = ResponseOkay;
            header.payloadSize = sizeof(ledVoltage);
            wifi.sendResponsePacket(&header, reinterpret_cast<uint8_t *>(&ledVoltage));
            continue;
            break;

        case LedGetErrorCode:
            header.status = ResponseNotSupported;
            break;

        case LedEspOtaStarted:
            wifi.wifiEvents.setBits(Wifi::EventWifiUpgradeStarted);
            // do not disturb ESP
            continue;
            break;

        case LedEspOtaFinished:
            wifi.wifiEvents.setBits(Wifi::EventWifiUpgradeFinished);
            continue;
            break;

        case LedSetStatusLed:
            header.status = ResponseNotSupported;
            break;

        default:
            header.status = ResponseInvalidCommand;
            break;
        }
        wifi.sendResponsePacket(&header, nullptr);
    }
}