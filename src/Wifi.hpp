#pragma once
#include <cstddef>
#include <cstdint>

namespace Wifi
{
void receiveData(const uint8_t *data, std::size_t length);
}