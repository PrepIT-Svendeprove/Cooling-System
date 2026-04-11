#include "external_cool_task.hpp"

#include <cstdint>
#include <etl/array.h>

#include "adc.h"
#include "i2c.h"

namespace tasks {
    external_cool_task::external_cool_task(const osMessageQueueId_t ambientTemperatures) :
    _ambientTemperatures(ambientTemperatures)
    {
    }

    void external_cool_task::run() {

        while (true) {

            const auto calculated_voltage_x100 = get_voltage_x100();
            std::uint32_t calculated_heatsink_temperature = (calculated_voltage_x100 - 218)+25;
            osMessageQueuePut(_ambientTemperatures,&calculated_heatsink_temperature,0,0);
            osDelay(1000);
        }
    }

    std::uint32_t external_cool_task::get_voltage_x100() {
        std::uint32_t value = 0;
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
        value = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        return static_cast<std::uint32_t>(static_cast<float>(value) * (static_cast<float>(330)/static_cast<float>(4095)));
    }
}
