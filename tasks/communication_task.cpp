//
// Created by lucas on 11-04-2026.
//

#include "communication_task.hpp"
#include "etl/string.h"
#include "usart.h"

extern "C" {
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

}
}

namespace tasks {
    communication_task::communication_task(osMessageQueueId_t ambientTemperatures,
                                           osMessageQueueId_t internalTemperatures,
                                           osMessageQueueId_t targetTemperatures) : _ambientTemperatures(
            ambientTemperatures),
        _internalTemperatures(internalTemperatures),
        _targetTemperatures(targetTemperatures) {
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
            etl::array<std::uint8_t, 64> uartRxData{};
            if (HAL_UART_Receive_DMA(&huart1, uartRxData.data(), 2) == HAL_OK) {
                if (uartRxData[0] == 0xAA && uartRxData[1] == 0xAA) {
                    if (HAL_UART_Receive_DMA(&huart1, uartRxData.data() + 2, 1) != HAL_OK) {
                        //return false;
                    } else {
                        // reorganise in new method later.

                        std::uint8_t size = uartRxData[2];
                        if (HAL_UART_Receive_DMA(&huart1, uartRxData.data() + 3, size) != HAL_OK) {
                        } else {
                            etl::string<64> received_str{};
                            etl::copy_n(uartRxData.data() + 3, size, received_str.begin());

                            auto first_equals = received_str.find_first_of("=");
                            if (received_str.substr(0, first_equals) == "TARGET_TEMP") {
                                std::size_t paramEnd = size;
                                if (auto split = received_str.find(";")) {
                                    paramEnd = split;
                                }
                                std::int32_t tempValue = std::stoi(received_str.substr(first_equals + paramEnd).data());
                                osMessageQueuePut(_targetTemperatures, &tempValue, 0, 0);
                                HAL_UART_Transmit(&huart1, reinterpret_cast<const uint8_t *>("TARGET_TEMP SET!\r\n"),
                                                  sizeof("TARGET_TEMP SET!\r\n"), 0);
                            }
                        }
                    }
                }
            }

            osDelay(1000);
        }
    }
} // tasks
