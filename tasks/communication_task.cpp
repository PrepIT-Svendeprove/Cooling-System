//
// Created by lucas on 11-04-2026.
//

#include "communication_task.hpp"
#include "etl/string.h"
#include "usart.h"

namespace tasks {
    communication_task::communication_task(osMessageQueueId_t ambientTemperatures, osMessageQueueId_t _internalTemperatures) :
    _ambientTemperatures(ambientTemperatures),
    _internalTemperatures(_internalTemperatures)
    {
    }

    void communication_task::run() {

        while (true) {
            etl::string<64> str{};
            if (osMessageQueueGetCount(_ambientTemperatures) > 0) {
                std::int32_t tempValue;
                osMessageQueueGet(_ambientTemperatures, &tempValue, nullptr, 0);
                str.append("AMBT:");
                str.append(std::to_string(tempValue).data());
                str.append("\r\n");
                osMessageQueueGet(_internalTemperatures, &tempValue, nullptr, 0);
                str.append("INTT:");
                str.append(std::to_string(tempValue).data());
                str.append("\r\n");
            }
            etl::array<std::uint8_t, 64> uartTxData{};
            std::copy_n(str.begin(), str.size(), uartTxData.begin());
            if (HAL_UART_Transmit_DMA(&huart1, uartTxData.data(), str.size()) != HAL_OK) {
                osDelay(100);
                HAL_UART_Transmit_DMA(&huart1, uartTxData.data(), str.size());
            }
            osDelay(1000);
        }
    }
} // tasks