#pragma once
#include <cstdint>

#include "cmsis_os2.h"

namespace tasks {
    class climate_control_task {
    public:
        climate_control_task(osMessageQueueId_t internal_temperatures, osMessageQueueId_t internal_humidity, osMessageQueueId_t target_temperatures, osMessageQueueId_t target_humidity);
        ~climate_control_task() = default;
        climate_control_task(const climate_control_task&) = delete;
        climate_control_task& operator=(const climate_control_task&) = delete;
        climate_control_task(climate_control_task&&) = delete;
        climate_control_task& operator=(climate_control_task&&) = delete;

        [[noreturn]] void run();

        static void run_wrapper(void *instance) {
            static_cast<climate_control_task *>(instance)->run();
        }

    private:
        static constexpr std::int32_t min_temp_target{-2};
        static constexpr std::int32_t max_temp_target{15};
        static constexpr std::int32_t min_humidity_target{30};
        static constexpr std::int32_t max_humidity_target{60};

        osMessageQueueId_t _internal_temperatures;
        osMessageQueueId_t _internal_humidity;
        osMessageQueueId_t _target_temperatures;
        osMessageQueueId_t _target_humidity;
        std::int32_t _current_target_temperature;
        std::int32_t _current_target_humidity;
    };
}
