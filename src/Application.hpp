#pragma once

#pragma once

#include "adc.h"
#include "dma.h"

#include "analog_to_digital/AnalogToDigital.hpp"
#include "button/ButtonHandler.hpp"
#include "leds/AddressableLeds.hpp"
#include "leds/LedFading.hpp"
#include "leds/StateMachine.hpp"
#include "leds/StatusLeds.hpp"

#include "wireless_gateway/PacketProcessor.hpp"
#include "wireless_gateway/UartRx.hpp"
#include "wireless_gateway/UartTx.hpp"
#include "wireless_gateway/Wifi.hpp"

class Application
{
public:
    static constexpr auto AdcPeripherie = &hadc1;
    static constexpr auto DmaLedTimerChannel1 = &hdma_tim1_ch1;
    static constexpr auto EspUartPeripherie = &huart2;

    Application();
    [[noreturn]] void run();

    static Application &getApplicationInstance();

private:
    static inline Application *instance{nullptr};

    // analog digital stuff
    units::si::Voltage ledVoltage{0.0_V};
    units::si::Current ledCurrent{0.0_A};

    AnalogToDigital analogToDigital{AdcPeripherie, ledVoltage, ledCurrent};

    // LED stuff
    AddressableLeds addressableLeds{DmaLedTimerChannel1};
    StatusLeds statusLeds{addressableLeds};
    LedFading ledFading{addressableLeds};
    StateMachine stateMachine{addressableLeds, ledFading};

    // touch button
    ButtonHandler buttonHandler{stateMachine};

    // wireless stuff
    static constexpr auto TxMessageBufferSize = 64;
    util::wrappers::StreamBuffer txMessageBuffer{TxMessageBufferSize, 0};

    static constexpr auto RxStreamBufferSize = 1024;
    util::wrappers::StreamBuffer rxStream{RxStreamBufferSize, 1};
    Wifi wifi{txMessageBuffer};
    UartTx uartTx{EspUartPeripherie, txMessageBuffer};
    UartRx uartRx{EspUartPeripherie, rxStream};
    PacketProcessor packetProcessor{rxStream,     wifi,       addressableLeds, ledFading,
                                    stateMachine, ledVoltage, ledCurrent};
};