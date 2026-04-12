//
// Created by lucas on 11-04-2026.
//

#include "communication_task.hpp"
#include "etl/string.h"
#include "usart.h"

namespace tasks {
    communication_task::communication_task(osMessageQueueId_t ambientTemperatures) :
    _ambientTemperatures(ambientTemperatures)
    {
    }

    void communication_task::run() {

        while (true) {
            if (osMessageQueueGetCount(_ambientTemperatures) > 0) {
                std::int32_t tempValue;
                osMessageQueueGet(_ambientTemperatures, &tempValue, nullptr, 0);
                etl::string<18> str{"ADC value: "};
                str.append(std::to_string(tempValue).data());
                str.append("\r\n");
                etl::array<std::uint8_t, 18> data{0};
                std::copy(str.begin(), str.end(), data.begin());

                if (HAL_UART_Transmit_DMA(&huart1, data.data(), data.size()) != HAL_OK) {
                    osDelay(100);
                    HAL_UART_Transmit_DMA(&huart1, data.data(), data.size());
                }
            }
            osDelay(1000);
        }
    }
} // tasks