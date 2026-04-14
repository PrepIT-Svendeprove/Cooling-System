//
// Created by lucas on 11-04-2026.
//

#include "communication_task.hpp"
#include "etl/string.h"
#include "usart.h"


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
            std::int32_t _last_ambient_temp{};
            std::int32_t _last_internal_temp{};
            if (osMessageQueueGetCount(_ambientTemperatures) > 0) {
                std::int32_t tempValue;
                osMessageQueueGet(_ambientTemperatures, &tempValue, nullptr, 0);
                if (tempValue > 0) {
                    _last_ambient_temp = tempValue;
                }
            }
            if (osMessageQueueGetCount(_internalTemperatures) > 0) {
                std::int32_t tempValue;
                osMessageQueueGet(_internalTemperatures, &tempValue, nullptr, 0);
                if (tempValue > 0) {
                    _last_internal_temp = tempValue;
                }
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

            osDelay(10);
        }
    }
} // tasks
