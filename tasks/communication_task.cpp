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
            std::uint16_t receivedBytes = 0;
            HAL_UARTEx_ReceiveToIdle(&huart1, uartRxData.data(), uartRxData.size(), &receivedBytes, 100);
            if (receivedBytes > 0) {
                if (uartRxData[0] == 0xAA && uartRxData[1] == 0xAA) {
                    std::uint8_t messageSize = uartRxData[2];
                    etl::string<64> received_str{};
                    received_str.fill(0);
                    received_str.resize(messageSize);
                    etl::copy_n(uartRxData.data() + 3, messageSize, received_str.begin());
                    auto e = '=';
                    auto first_equals = received_str.find(e);
                    if (first_equals != std::string::npos && received_str.substr(0, first_equals) == "TARGET_TEMP") {
                        std::size_t paramEnd = messageSize;
                        if (auto split = received_str.find(";"); split != std::string::npos) {
                            paramEnd = split;
                        }
                        auto valueStr = received_str.substr(first_equals + 1, paramEnd - (first_equals + 1)).data();
                        std::int32_t tempValue = std::stoi(valueStr);
                        osMessageQueuePut(_targetTemperatures, &tempValue, 0, 0);
                        HAL_UART_Transmit_DMA(&huart1, reinterpret_cast<const uint8_t *>("TARGET_TEMP SET!\r\n"),
                                          sizeof("TARGET_TEMP SET!\r\n"));
                    }
                }
            }

            osDelay(10);
        }
    }
} // tasks
