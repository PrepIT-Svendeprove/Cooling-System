//
// Created by lucas on 11-04-2026.
//

#include "communication_task.hpp"


#include "commands.hpp"
#include "esp_at_wifi_mqtt.hpp"
#include "spi.h"
#include "stm32f4xx_hal_uart.h"
#include "etl/string.h"
#include "etl/vector.h"
#include "etl/circular_buffer.h"
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
        osDelay(1000);
        drivers::esp_at_wifi_mqtt mqtt{};
        bool connectedWifi = mqtt.is_wifi_connected();
        if (!connectedWifi) {
            connectedWifi = mqtt.connect_to_wifi("FTTH_JX3020", "vadoghabNev3");
        }
        osDelay(100);
        if (connectedWifi) {
            mqtt.clean_mqtt();
            osDelay(100);
            mqtt.configure_mqtt_user("device", "device");
            osDelay(10);
            mqtt.connect_to_mqtt("192.168.0.50", "1883");
            osDelay(10);
        }
        std::uint32_t lastTick{};
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
            if (auto currentTick = osKernelGetTickCount(); currentTick > lastTick + 2000) {
                lastTick = currentTick;
                etl::string<80> payload{R"({\"climateDeviceCode\":\")"};
                payload.append("DEV01");
                payload.append(R"(\"\,\"temperature\":)");
                payload.append(std::to_string(_last_internal_temp).data());
                payload.append(R"(\,\"humidity\":)");
                payload.append(std::to_string(_last_ambient_temp).data()); // TODO: Replace with internal Humidity
                payload.append(R"(})");
                if (mqtt.is_mqtt_connected()) {
                    mqtt.publish_to_mqtt("climate/telemetry", payload);
                }
                //HAL_UART_Transmit_DMA(&huart1, reinterpret_cast<const uint8_t *>(payload.data()), payload.size());
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
                        } else if (parameter == "WIFI_PASS") {
                        }
                        osDelay(1);
                    }
                }
            }

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
                HAL_UART_Transmit_DMA(&huart1, reinterpret_cast<std::uint8_t *>(temperatureStr.data()),
                                      temperatureStr.size());
                osDelay(10);
                break;
            }
            case types::command::GET_INT_HUMIDITY:
                break;
        }
    }
} // tasks
