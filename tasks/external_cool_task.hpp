#pragma once
#include <cstdint>

#include "cmsis_os2.h"

namespace tasks {
    class external_cool_task {
    public:
        external_cool_task(osMessageQueueId_t ambientTemperatures);

        void run();
        static void run_wrapper(void* instance) {
            static_cast<external_cool_task*>(instance)->run();
        }
    private:
        static std::uint32_t get_voltage_x100();

        osMessageQueueId_t _ambientTemperatures;
    };
}
