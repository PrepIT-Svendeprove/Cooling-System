#pragma once
#include <cstdint>

namespace types {
    enum class command : std::uint8_t {
        RESTART = 0x00,
        RESET_SETTINGS = 0x05,
        SAVE_MQTT_SETTINGS = 0x06,
        SAVE_WIFI_SETTINGS = 0x07,
        ENABLE_COOLING = 0x10,
        DISABLE_COOLING = 0x11,
        GET_INT_TEMP = 0x20,
        GET_INT_HUMIDITY = 0x21,
        CONNECT_MQTT = 0x40,
        CONNECT_WIFI = 0x41,
    };
}
