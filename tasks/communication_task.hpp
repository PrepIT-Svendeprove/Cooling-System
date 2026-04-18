#pragma once
#include <etl/string_view.h>

#include "cmsis_os2.h"

namespace tasks {
    class communication_task {
    public:
        communication_task(osMessageQueueId_t ambientTemperatures, osMessageQueueId_t internalTemperatures, osMessageQueueId_t targetTemperatures, osMessageQueueId_t controlCommands);

        void run();

        static void run_wrapper(void *instance) {
            static_cast<communication_task *>(instance)->run();
        }

    private:
        void handle_command(std::uint8_t command);
        osMessageQueueId_t _ambientTemperatures;
        osMessageQueueId_t _internalTemperatures;
        osMessageQueueId_t _targetTemperatures;
        osMessageQueueId_t _controlCommands;
        std::int32_t _last_ambient_temp;
        std::int32_t _last_internal_temp;
        bool _mqttValueUpdated;
    };
} // tasks
