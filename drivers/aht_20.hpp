#pragma once
#include <cstdint>

#include "i2c.h"
namespace drivers {

class aht20 {
    public:
    aht20(I2C_HandleTypeDef* i2c);
    ~aht20() = default;

    void init() const;
    void read_sensor();
    // TODO: Consider adding float return or more digits.
    [[nodiscard]] std::int32_t get_last_temperature() const;
    [[nodiscard]] std::int32_t get_last_humidity() const;

private:
    static constexpr std::uint8_t aht_addr = 0x38 << 1;
    I2C_HandleTypeDef* _i2c;
    std::int32_t _last_temperature;
    std::int32_t _last_humidity;
};

} // drivers
