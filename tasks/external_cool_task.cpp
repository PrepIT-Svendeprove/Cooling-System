#include "external_cool_task.hpp"

#include <cmath>
#include <cstdint>
#include <etl/array.h>

#include "adc.h"
#include "aht_20.hpp"
#include "i2c.h"

namespace tasks {
    external_cool_task::external_cool_task(const osMessageQueueId_t ambientTemperatures) : _ambientTemperatures(
        ambientTemperatures) {
    }

    void external_cool_task::run() {

        drivers::aht20 temp_sensor{&hi2c1};

        temp_sensor.init();

        while (true) {
            //const auto calculated_voltage_x100 = get_voltage_x100();
            //std::uint32_t calculated_heatsink_temperature = (calculated_voltage_x100 - 218) + 25;

            temp_sensor.read_sensor();

            auto ambient_temp = temp_sensor.get_last_temperature();
            //auto temp_diff = static_cast<std::int32_t>(calculated_heatsink_temperature) - static_cast<std::int32_t>(ambient_temp);
            // LM235s are broken, find replacement for heatsink temperature measurement.

            // Temporary just keep them enabled, until new temperature sensor is available, consider using ambient to check "output" temp of air from heatsink.
            HAL_GPIO_WritePin(EXT_COOL_FANS_GPIO_Port, EXT_COOL_FANS_Pin, GPIO_PIN_SET);

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
