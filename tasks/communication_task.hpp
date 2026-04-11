#pragma once
#include "cmsis_os2.h"

namespace tasks {
    class communication_task {
    public:
        communication_task(osMessageQueueId_t ambientTemperatures);

        void run();

        static void run_wrapper(void *instance) {
            static_cast<communication_task *>(instance)->run();
        }

    private:
        osMessageQueueId_t _ambientTemperatures;
    };
} // tasks
