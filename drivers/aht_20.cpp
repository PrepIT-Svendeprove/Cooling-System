//
// Created by lucas on 12-04-2026.
//

#include "aht_20.hpp"

#include "etl/array.h"
#include "cmsis_os2.h"

namespace drivers {
    aht20::aht20(I2C_HandleTypeDef *i2c) :
        _i2c{i2c},
        _last_temperature{0},
        _last_humidity{0}
    {
    }

    void aht20::init() const
    {
        osDelay(50);
        etl::array<std::uint8_t, 3> init_cmd{0xBE, 0x08, 0x00};
        etl::array<std::uint8_t, 4> init_data{0};
        HAL_I2C_Mem_Read(_i2c, aht_addr, 0x71, 1, init_data.data(), 1, 100);
        if (!((init_data[0] >> 3) & 0x1)) {
            HAL_I2C_Master_Transmit(_i2c, aht_addr, init_cmd.data(), init_cmd.size(), 1000);
        }

        osDelay(50);
    }

    void aht20::read_sensor()
    {
        etl::array<std::uint8_t, 3> trigger_measurement_cmd{0xAC, 0x33, 0x00};
        HAL_I2C_Master_Transmit(_i2c, aht_addr, trigger_measurement_cmd.data(), trigger_measurement_cmd.size(), 1000);
        osDelay(80);
        etl::array<std::uint8_t, 6> data_buffer{0};
        do {
            HAL_I2C_Mem_Read(_i2c, aht_addr, 0x71, 1, data_buffer.data(), 1, 100);
            osDelay(1);
        } while ((data_buffer[0] >> 7) & 0x1);

        data_buffer.fill(0);

        HAL_I2C_Master_Receive(_i2c, aht_addr, data_buffer.data(), data_buffer.size(), 1000);
        const std::uint32_t temp_data = (data_buffer[3] & 0x0F) << 16 | data_buffer[4] << 8 | data_buffer[5];
        const std::uint32_t humidity_data = data_buffer[1] << 12 | data_buffer[2] << 4 | (data_buffer[3] & 0xF);
        constexpr double i = (1 << 20);
        _last_temperature = static_cast<std::int32_t>( (temp_data / i) * 200 - 50);
        _last_humidity = static_cast<std::int32_t>((humidity_data / i * 100));
    }

    std::int32_t aht20::get_last_temperature() const
    {
        return _last_temperature;
    }

    std::int32_t aht20::get_last_humidity() const
    {
        return _last_humidity;
    }

} // drivers
