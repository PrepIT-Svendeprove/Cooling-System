//
// Created by lucas on 11-04-2026.
//

#include "communication_task.hpp"


#include "commands.hpp"
#include "spi.h"
#include "stm32f4xx_hal_gpio.h"
#include "etl/string.h"
#include "etl/vector.h"
#include "usart.h"
#include "ethernet_w5500/wizchip_conf.h"

extern "C" {
uint8_t spi_readbyte() {
    uint8_t tx = 0x00;
    uint8_t byte;
    HAL_SPI_TransmitReceive(&hspi1, &tx, &byte, 1, 100);
    return byte;
}

void spi_writebyte(uint8_t byte) {
    HAL_SPI_Transmit(&hspi1, &byte, 1, 100);
}

void spi_readburst(std::uint8_t *data, std::uint16_t len) {
    HAL_SPI_Receive(&hspi1, data, len, 500);
}

void spi_writeburst(std::uint8_t *data, std::uint16_t len) {
    HAL_SPI_Transmit(&hspi1, data, len, 100);
}

void cs_select() {
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
}

void cs_deselect() {
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
}
}

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
        reg_wizchip_cs_cbfunc(cs_select, cs_deselect);
        reg_wizchip_spi_cbfunc(spi_readbyte, spi_writebyte);
        reg_wizchip_spiburst_cbfunc(spi_readburst, spi_writeburst);

        for (uint8_t i = 0; i < 0x2F; i++) {
            cs_select();
            // Write address phase (read command for VERSIONR: 0x0039)
            uint8_t readVer[8] = {0x00, i, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t rxVer[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            HAL_SPI_TransmitReceive(&hspi1, readVer, rxVer, 8, 500);
            //HAL_SPI_Receive(&hspi1, data, 6, 100);
            cs_deselect();
            osDelay(10);
            HAL_UART_Transmit_DMA(&huart1, rxVer, 8);
        }


        HAL_GPIO_WritePin(ENC_RST_GPIO_Port, ENC_RST_Pin, GPIO_PIN_RESET);
        osDelay(200);
        HAL_GPIO_WritePin(ENC_RST_GPIO_Port, ENC_RST_Pin, GPIO_PIN_SET);
        osDelay(500);
        uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
        std::int8_t init_result = wizchip_init(memsize[0], memsize[1]);
        if (init_result == -1) {
            while (1);
        }
        osDelay(10);
        uint8_t ver = getVERSIONR();
        etl::string<32> version{"Version: "};
        version.append(std::to_string(ver).data());
        version.append("\r\n");
        HAL_UART_Transmit_DMA(&huart1, reinterpret_cast<const uint8_t *>(version.data()), version.size());
        osDelay(10);
        std::int8_t link = 0;
        std::uint8_t retries = 10;
        while (link != PHY_LINK_ON && retries != 0) {
            if (link = wizphy_getphylink(); link == PHY_LINK_ON) {
                wiz_NetInfo gWIZNETINFO = {
                    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
                    .ip = {192, 168, 0, 55},
                    .sn = {255, 255, 255, 0},
                    .gw = {192, 168, 0, 1},
                    .dns = {8, 8, 8, 8}, // Optional, but good practice
                    .dhcp = NETINFO_STATIC
                };
                ctlnetwork(CN_SET_NETINFO, (void *) &gWIZNETINFO);
                break;
            }
            etl::string<32> retrying{"retrying, tries left:"};
            retrying.append(std::to_string(retries).data());
            retrying.append("\r\n");
            HAL_UART_Transmit_DMA(&huart1, reinterpret_cast<const uint8_t *>(retrying.data()), retrying.size());
            retries--;
            osDelay(500);
        }
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
                        } else if (parameter == "WIFI_PASSWORD") {
                        }
                        osDelay(1);
                    }
                    // auto first_equals = received_str.find('=');
                    // etl::string<64> parameter = received_str.substr(0, first_equals);
                    // if (first_equals != std::string::npos &&  parameter == "TARGET_TEMP") {
                    //     std::size_t paramEnd = messageSize;
                    //     if (auto split = received_str.find(";"); split != std::string::npos) {
                    //         paramEnd = split;
                    //     }
                    //     auto valueStr = received_str.substr(first_equals + 1, paramEnd - (first_equals + 1)).data();
                    //     std::int32_t tempValue = std::stoi(valueStr);
                    //     osMessageQueuePut(_targetTemperatures, &tempValue, 0, 0);
                    //     HAL_UART_Transmit_DMA(&huart1, reinterpret_cast<const uint8_t *>("TARGET_TEMP SET!\r\n"),
                    //                           sizeof("TARGET_TEMP SET!\r\n"));
                    // }
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
