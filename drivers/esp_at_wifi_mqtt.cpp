#include "esp_at_wifi_mqtt.hpp"

#include "usart.h"

namespace {
    void write_at(const etl::string_view &data) {
        HAL_UART_Transmit_DMA(&huart6, reinterpret_cast<const uint8_t *>(data.data()), data.size());
    }
}

namespace drivers {
    bool esp_at_wifi_mqtt::connect_to_wifi(const etl::string_view &ssid, const etl::string_view &password) {
        // 14 for AT+CWJAP="","" and 48 for each value ssid and password according to max value in SIMPLE_UART_CONFIG_PROTOCOL.md
        etl::string<14+48+48+2> connectionCommandString{"AT+CWJAP=\""};
        connectionCommandString.append(ssid);
        connectionCommandString.append("\",\"");
        connectionCommandString.append(password);
        connectionCommandString.append("\"\r\n");
        write_at(connectionCommandString);
        return true; // TODO: If time, actually check for success
    }

    bool esp_at_wifi_mqtt::connect_to_mqtt(const etl::string_view &broker, const etl::string_view &port) {

        etl::string<17+48+4+2> connectionCommandString{"AT+MQTTCONN=0,\""};
        connectionCommandString.append(broker);
        connectionCommandString.append("\",");
        connectionCommandString.append(port);
        connectionCommandString.append("\r\n");
        write_at(connectionCommandString);
        return true; // TODO: If time, actually check for success
    }

    bool esp_at_wifi_mqtt::configure_mqtt_user(etl::string_view username, etl::string_view password) {
        etl::string<24+48+48+2> connectionCommandString{"AT+MQTTUSERCFG=0,1,\""};
        connectionCommandString.append(username);
        connectionCommandString.append("\",\"");
        connectionCommandString.append(password);
        connectionCommandString.append("\"\r\n");
        write_at(connectionCommandString);
        return true; // TODO: If time, actually check for success
    }

    void esp_at_wifi_mqtt::publish_to_mqtt(etl::string_view topic, etl::string_view payload) {
        etl::string<24+48+48+2> connectionCommandString{"AT+MQTTPUB=0,\""};
        connectionCommandString.append(topic);
        connectionCommandString.append("\",\"");
        connectionCommandString.append(payload);
        connectionCommandString.append("\"\r\n");
        write_at(connectionCommandString);
    }
} // drivers