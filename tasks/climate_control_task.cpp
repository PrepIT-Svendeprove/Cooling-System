#include "climate_control_task.hpp"

#include "aht_20.hpp"
#include "commands.hpp"

namespace tasks {
    climate_control_task::climate_control_task(osMessageQueueId_t internal_temperatures,
                                               osMessageQueueId_t internal_humidity,
                                               osMessageQueueId_t target_temperatures,
                                               osMessageQueueId_t target_humidity,
                                               osMessageQueueId_t control_commands) :
    _internal_temperatures{internal_temperatures},
        _internal_humidity{internal_humidity},
        _target_temperatures{target_temperatures},
        _target_humidity{target_humidity},
        _control_commands{control_commands},
        _current_target_temperature{10},
        _current_target_humidity{50},
        _cooling_enabled{false} {
    }

    void climate_control_task::run() {
        drivers::aht20 temp_sensor{&hi2c2};
        temp_sensor.init();
        while (true) {
            temp_sensor.read_sensor();

            auto temp = temp_sensor.get_last_temperature();
            osMessageQueuePut(_internal_temperatures, &temp, 0, 0);

            auto humidity = temp_sensor.get_last_humidity();
            osMessageQueuePut(_internal_humidity, &humidity, 0, 0);

            std::uint32_t targets = osMessageQueueGetCount(_target_temperatures);
            if (targets > 0) {
                std::int32_t temp_target{0};
                osMessageQueueGet(_target_temperatures, &temp_target, nullptr, 0);
                if (temp_target > min_temp_target && temp_target < max_temp_target) {
                    _current_target_temperature = temp_target;
                }
            }
            targets = osMessageQueueGetCount(_target_temperatures);
            if (targets > 0) {
                std::int32_t target{0};
                osMessageQueueGet(_target_humidity, &target, nullptr, 0);
                if (target > min_humidity_target && target < max_humidity_target) {
                    _current_target_humidity = target;
                }
            }

            // For now only temperature is controllable, testing will conclude if humidity is realistic to change.

            if ( _cooling_enabled && temp > _current_target_temperature) {
                HAL_GPIO_WritePin(PELTIER_GPIO_Port, PELTIER_Pin, GPIO_PIN_SET);
                HAL_GPIO_WritePin(INT_CIR_FAN_GPIO_Port, INT_CIR_FAN_Pin, GPIO_PIN_SET);
            } else {
                HAL_GPIO_WritePin(INT_CIR_FAN_GPIO_Port, INT_CIR_FAN_Pin, GPIO_PIN_RESET);
                osDelay(1000);
                HAL_GPIO_WritePin(PELTIER_GPIO_Port, PELTIER_Pin, GPIO_PIN_RESET);
            }
            if (const auto messageCount = osMessageQueueGetCount(_control_commands); messageCount > 0) {
                for (std::size_t i = 0; i < messageCount; i++) {
                    types::command cmd;
                    osMessageQueueGet(_control_commands, &cmd, nullptr, 0);
                    if (cmd == types::command::ENABLE_COOLING) {
                        _cooling_enabled = true;
                    } else if (cmd == types::command::DISABLE_COOLING) {
                        _cooling_enabled = false;
                    }
                }
            }

            osDelay(1000);
        }
    }
}
