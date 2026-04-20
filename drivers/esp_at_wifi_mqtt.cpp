#include "esp_at_wifi_mqtt.hpp"

#include "cmsis_os2.h"
#include "usart.h"

namespace {
    void write_at(const etl::string_view &data) {
        HAL_UART_Transmit(&huart6, reinterpret_cast<const uint8_t *>(data.data()), data.size(), 100);
    }
}

namespace drivers {
    bool esp_at_wifi_mqtt::connect_to_wifi(const etl::string_view &ssid, const etl::string_view &password) {
        // 14 for AT+CWJAP="","" and 48 for each value ssid and password according to max value in SIMPLE_UART_CONFIG_PROTOCOL.md
        etl::string<14 + 48 + 48 + 2> connectionCommandString{"AT+CWJAP=\""};
        connectionCommandString.append(ssid);
        connectionCommandString.append("\",\"");
        connectionCommandString.append(password);
        connectionCommandString.append("\"\r\n");
        write_at(connectionCommandString);
        std::uint16_t receivedBytes = 0;
        std::uint8_t retries = 3;
        etl::array<std::uint8_t, 128> dataBuffer{};
        do {
            HAL_UARTEx_ReceiveToIdle(&huart6, dataBuffer.data(), dataBuffer.size(), &receivedBytes, 1000);
            if (receivedBytes > 0) {
                etl::string_view response{reinterpret_cast<char *>(dataBuffer.data()), receivedBytes};
                if (response.contains("CONNECTED")) {
                    return true;
                }
            }
            retries--;
        } while (receivedBytes > 0 || retries > 0);
        return false;
    }

    bool esp_at_wifi_mqtt::connect_to_mqtt(const etl::string_view &broker, const etl::string_view &port) {
        etl::string<17 + 48 + 4 + 2> connectionCommandString{"AT+MQTTCONN=0,\""};
        connectionCommandString.append(broker);
        connectionCommandString.append("\",");
        connectionCommandString.append(port);
        connectionCommandString.append(",1\r\n");
        write_at(connectionCommandString);
        return true; // TODO: If time, actually check for success
    }

    bool esp_at_wifi_mqtt::configure_mqtt_user(etl::string_view username, etl::string_view password) {
        etl::string<24 + 48 + 48 + 2> connectionCommandString{"AT+MQTTUSERCFG=0,1,\"cooling_system\",\""};
        connectionCommandString.append(username);
        connectionCommandString.append("\",\"");
        connectionCommandString.append(password);
        connectionCommandString.append("\",0,0,\"\"\r\n");
        write_at(connectionCommandString);
        return true; // TODO: If time, actually check for success
    }

    void esp_at_wifi_mqtt::publish_to_mqtt(etl::string_view topic, etl::string_view payload) {
        etl::string<24 + 48 + 48 + 2> connectionCommandString{"AT+MQTTPUB=0,\""};
        connectionCommandString.append(topic);
        connectionCommandString.append("\",\"");
        connectionCommandString.append(payload);
        connectionCommandString.append("\",0,0\r\n");
        write_at(connectionCommandString);
    }
    

    void esp_at_wifi_mqtt::clean_mqtt() {
        etl::string_view connectionCommandString{"AT+MQTTCLEAN=0\r\n"};
        write_at(connectionCommandString);
    }

    bool esp_at_wifi_mqtt::is_mqtt_connected() {
        etl::string_view request{"AT+MQTTCONN?\r\n"};
        write_at(request);
        etl::array<std::uint8_t, 128> dataBuffer{};
        std::uint16_t receivedBytes;
        const uint32_t start = osKernelGetTickCount();
        const uint32_t timeout_ms = 5000;  // 5 seconds max

        do {
            HAL_UARTEx_ReceiveToIdle(&huart6, dataBuffer.data(), dataBuffer.size(), &receivedBytes, 1000);
            if (receivedBytes > 0) {
                // Create a view over the *actual received bytes* each time
                etl::string_view response{reinterpret_cast<char *>(dataBuffer.data()), receivedBytes};
                if (response.starts_with("+MQTTCONN:") && receivedBytes > 12) {
                    etl::string_view state = response.substr(12, 1);
                    if (std::stoi(state.data()) > 3) {
                        return true;
                    }
                }
                // Optional: if you see OK or ERROR without +CWSTATE:2, we can stop early
                if (response.contains("OK") || response.contains("ERROR")) {
                    break;
                }
            }
        } while ((osKernelGetTickCount() - start) < timeout_ms);
        return false;
    }

    bool esp_at_wifi_mqtt::is_wifi_connected() {
        etl::string_view request{"AT+CWSTATE?\r\n"};
        write_at(request);
        etl::array<std::uint8_t, 128> dataBuffer{};
        std::uint16_t receivedBytes;
        const uint32_t start = osKernelGetTickCount();
        const uint32_t timeout_ms = 5000;  // 5 seconds max

        do {
            HAL_UARTEx_ReceiveToIdle(&huart6, dataBuffer.data(), dataBuffer.size(), &receivedBytes, 1000);
            if (receivedBytes > 0) {
                // Create a view over the *actual received bytes* each time
                etl::string_view response{reinterpret_cast<char*>(dataBuffer.data()), receivedBytes};
                if (response.contains("+CWSTATE:2")) {
                    return true;
                }
                // Optional: if you see OK or ERROR without +CWSTATE:2, we can stop early
                if (response.contains("OK") || response.contains("ERROR")) {
                    break;
                }
            }
        } while ((osKernelGetTickCount() - start) < timeout_ms);
        return false;
    }
} // drivers
