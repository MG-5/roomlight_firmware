#pragma once

#include "util/gpio.hpp"
#include <gmock/gmock.h>

class GpioMock : public util::Gpio
{
public:
    GpioMock() : util::Gpio(nullptr, 0)
    {
    }

    MOCK_METHOD(State, read, (), (override));
    MOCK_METHOD(void, write, (State state), (override));
};