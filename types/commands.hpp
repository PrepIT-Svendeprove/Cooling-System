#pragma once
#include <cstdint>

namespace types {
    enum class command : std::uint8_t {
        RESTART = 0,
        ENABLE_COOLING,
        DISABLE_COOLING,
    };
}
