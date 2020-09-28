#pragma once

#include "stm32f1xx_hal.h"
#include <cstdint>

class Gpio
{
public:
    Gpio(GPIO_TypeDef *port, uint16_t pin) : port{port}, pin{pin}
    {
    }

    bool read()
    {
        return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET;
    }

    void write(bool state)
    {
        HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

private:
    GPIO_TypeDef *const port;
    const uint16_t pin;
};