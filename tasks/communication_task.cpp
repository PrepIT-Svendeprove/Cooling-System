//
// Created by lucas on 11-04-2026.
//

#include "communication_task.hpp"


#include "spi.h"
#include "etl/string.h"
#include "usart.h"
#include "Ethernet.h"

void ENC28J60_DelayMs(uint32_t delay) {
    osDelay(delay);
}

uint32_t ENC28J60_GetMs(void) {
    return HAL_GetTick();
}

uint8_t ENC28J60_TransceiveByte(uint8_t data) {
    uint8_t received;
    if (HAL_SPI_TransmitReceive(&hspi1, &data, &received, 1, 1000) == HAL_OK) {
        return received;
    }
    return 0;
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

        static uint8_t mymac[6] = { 0x54, 0x57, 0x51, 0x10, 0x00, 0x25 };
        //static uint8_t myip[4] = { 192, 168, 0, 15};
        static uint8_t buf[500];
        uint16_t plen, dat_p;

        /* For DHCP */
        static uint8_t myip[4] = { 0, 0, 0, 0 };
        static uint8_t mynetmask[4] = { 0, 0, 0, 0 };

        // Default gateway. The ip address of your DSL router. It can be set to the same as
        // websrvip the case where there is no default GW to access the
        // web server (=web server is on the same lan as this host)
        static uint8_t gwip[4] = { 192, 168, 0, 1 };

        static uint8_t dnsip[4] = { 0, 0, 0, 0 };
        static uint8_t dhcpsvrip[4] = { 0, 0, 0, 0 };

        HAL_GPIO_WritePin(ENC_RST_GPIO_Port, ENC_RST_Pin, GPIO_PIN_RESET);
        osDelay(10);
        HAL_GPIO_WritePin(ENC_RST_GPIO_Port, ENC_RST_Pin, GPIO_PIN_SET);
        osDelay(10);
        Ethernet_Init(mymac);

        if (Ethernet_Revision() <= 0) {
            // Failed to access ENC28J60
            while (1); // Just loop here
        }
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
                    if (received_str.contains(';')) {
                        //TODO: Make handler for multiple values
                    }
                    auto first_equals = received_str.find('e');
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
