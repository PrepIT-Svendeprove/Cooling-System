#include "external_cool_task.hpp"

#include <cmath>
#include <cstdint>
#include <etl/array.h>

#include "adc.h"
#include "i2c.h"

namespace tasks {
    external_cool_task::external_cool_task(const osMessageQueueId_t ambientTemperatures) : _ambientTemperatures(
        ambientTemperatures) {
    }

    void external_cool_task::run() {
        constexpr std::uint8_t aht_addr = 0x38 << 1;

        osDelay(50);
        etl::array<std::uint8_t, 3> init_cmd{0xBE, 0x08, 0x00};
        etl::array<std::uint8_t, 4> init_data{0};
        HAL_I2C_Mem_Read(&hi2c1, aht_addr, 0x71, 1, init_data.data(), 1, 100);
        if (!((init_data[0] >> 3) & 0x1)) {
            HAL_I2C_Master_Transmit(&hi2c1, aht_addr, init_cmd.data(), init_cmd.size(), 1000);
        }

        osDelay(50);

        while (true) {
            const auto calculated_voltage_x100 = get_voltage_x100();
            std::uint32_t calculated_heatsink_temperature = (calculated_voltage_x100 - 218) + 25;

            etl::array<std::uint8_t, 3> trigger_measurement_cmd{0xAC, 0x33, 0x00};
            HAL_I2C_Master_Transmit(&hi2c1, aht_addr, trigger_measurement_cmd.data(), trigger_measurement_cmd.size(), 1000);
            osDelay(80);
            etl::array<std::uint8_t, 6> data_buffer{0};
            do {
                HAL_I2C_Mem_Read(&hi2c1, aht_addr, 0x71, 1, data_buffer.data(), 1, 100);
                osDelay(1);
            } while((data_buffer[0] >> 7) & 0x1);

            data_buffer.fill(0);

            HAL_I2C_Master_Receive(&hi2c1, aht_addr, data_buffer.data(), data_buffer.size(), 1000);
            std::uint32_t temp_data = (data_buffer[3]&0x0F)<<16 | (data_buffer[4])<<8 | data_buffer[5];
            auto ambient_temp = static_cast<std::uint32_t>((temp_data / std::pow(2,20)*200 - 50));

            //auto temp_diff = static_cast<std::int32_t>(calculated_heatsink_temperature) - static_cast<std::int32_t>(ambient_temp);
            // LM235s are broken, find replacement for heatsink temperature measurement.

            osMessageQueuePut(_ambientTemperatures, &ambient_temp, 0, 0);
            osDelay(1000);
        }
    }

    std::uint32_t external_cool_task::get_voltage_x100() {
        std::uint32_t value = 0;
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
        value = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        return static_cast<std::uint32_t>(static_cast<float>(value) * (
                                              static_cast<float>(330) / static_cast<float>(4095)));
    }
}
