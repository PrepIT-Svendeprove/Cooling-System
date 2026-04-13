#pragma once
#include "cmsis_os2.h"

namespace tasks {
    class communication_task {
    public:
        communication_task(osMessageQueueId_t ambientTemperatures, osMessageQueueId_t internalTemperatures, osMessageQueueId_t targetTemperatures);

        void run();

        static void run_wrapper(void *instance) {
            static_cast<communication_task *>(instance)->run();
        }

    private:
        osMessageQueueId_t _ambientTemperatures;
        osMessageQueueId_t _internalTemperatures;
        osMessageQueueId_t _targetTemperatures;
    };
} // tasks
