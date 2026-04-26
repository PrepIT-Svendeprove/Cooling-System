//
// Created by lucas on 11-04-2026.
//

#include "communication_task.hpp"


#include "commands.hpp"
#include "spi.h"
#include "etl/string.h"
#include "etl/vector.h"
#include "usart.h"

namespace tasks {
    communication_task::communication_task(osMessageQueueId_t ambientTemperatures,
                                           osMessageQueueId_t internalTemperatures,
                                           osMessageQueueId_t targetTemperatures,
                                           osMessageQueueId_t controlCommands) : _ambientTemperatures{
            ambientTemperatures
        },
        _internalTemperatures{internalTemperatures},
        _targetTemperatures{targetTemperatures},
        _controlCommands{controlCommands},
        _last_internal_temp{0},
        _last_ambient_temp{0} {
    }

    void communication_task::run() {
        while (true) {
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
                    etl::vector<etl::string<20>, 10> parameters{};
                    if (received_str.contains(';')) {
                        auto pos = received_str.find(';');
                        while (pos != std::string::npos) {
                            auto parameter = received_str.substr(0, pos);
                            parameters.push_back(parameter);
                            received_str.erase(0, pos + 1);
                            pos = received_str.find(';');
                        }
                    } else {
                        parameters.push_back(received_str);
                    }
                    for (const auto &parameterString: parameters) {
                        const auto equals = parameterString.find('=');
                        if (equals == std::string::npos) {
                            continue;
                        }
                        auto parameter = parameterString.substr(0, equals);
                        auto value = parameterString.substr(equals + 1);
                        if (parameter == "TARGET_TEMP") {
                            std::int32_t tempValue = std::stoi(value.data());
                            osMessageQueuePut(_targetTemperatures, &tempValue, 0, 0);
                            HAL_UART_Transmit_DMA(&huart1, reinterpret_cast<const uint8_t *>("TARGET_TEMP SET\r\n"),
                                                  sizeof("TARGET_TEMP SET\r\n"));
                        } else if (parameter == "TARGET_HUMIDITY") {
                            etl::string<35> responseStr{"HUMIDITY CURRENTLY NOT SUPPORTED\r\n"};
                            HAL_UART_Transmit_DMA(&huart1, reinterpret_cast<const uint8_t *>(responseStr.data()),
                                                  responseStr.size());
                        } else if (parameter == "MQTT_BROKER") {
                        } else if (parameter == "MQTT_PORT") {
                        } else if (parameter == "MQTT_USER") {
                        } else if (parameter == "MQTT_PASS") {
                        } else if (parameter == "CMD") {
                            auto cmd_id = std::stoi(value.data());
                            handle_command(cmd_id);
                        } else if (parameter == "ENABLE_WIFI_NETWORKING") {
                        } else if (parameter == "WIFI_SSID") {
                            _settings.wifiSsid = value;
                        } else if (parameter == "WIFI_PASS") {
                            _settings.wifiPassword = value;
                        } else if (parameter == "ENDURE_DEVICE_CODE") {
                            _settings.deviceCode = value;
                        }
                        osDelay(1);
                    }
                }
            }
            uartRxData.fill(0);
            receivedBytes = 0;
            //HAL_UARTEx_ReceiveToIdle(&huart6, uartRxData.data(), uartRxData.size(), &receivedBytes, 100);
            // if (receivedBytes > 0) {
            //     etl::string_view mqttString{reinterpret_cast<char *>(uartRxData.data()), receivedBytes};
            //     if (mqttString.starts_with("+MQTTSUBRECV:0,") && receivedBytes > 15) {
            //         mqttString.remove_prefix(15);
            //         etl::string_view topic = mqttString.substr(0, mqttString.find_first_of(',') + 1);
            //         osDelay(1);
            //         if (topic.contains("climate/control/") && topic.contains(_settings.deviceCode)) {
            //             etl::string_view payload = mqttString.substr(mqttString.find_first_of('{'));
            //             ArduinoJson::JsonDocument doc;
            //             ArduinoJson::deserializeJson(doc, payload);
            //             auto temperature = doc["temperature"].as<std::int32_t>();
            //             // Minimum number we allow it to be set to.
            //             if (temperature > 2 && temperature < _last_ambient_temp) {
            //                 osMessageQueuePut(_targetTemperatures, &temperature, 0, 0);
            //             }
            //         }
            //     }
            // }
            osDelay(10);
        }
    }

    void communication_task::handle_command(std::uint8_t command) {
        switch (static_cast<types::command>(command)) {
            case types::command::RESTART:
                NVIC_SystemReset();
            case types::command::RESET_SETTINGS:
                break;
            case types::command::ENABLE_COOLING:
            case types::command::DISABLE_COOLING: {
                osMessageQueuePut(_controlCommands, &command, 0, 0);
                break;
            }
            case types::command::GET_INT_TEMP: {
                etl::string<20> temperatureStr{"INT_TEMP:"};
                temperatureStr.append(std::to_string(_last_internal_temp).data());
                temperatureStr.append("\r\n");
                HAL_UART_Transmit(&huart1, reinterpret_cast<std::uint8_t *>(temperatureStr.data()),
                                  temperatureStr.size(), 100);
                osDelay(10);
                break;
            }
            case types::command::GET_INT_HUMIDITY:
                break;
        }
    }
} // tasks
