#pragma once
#include <cstdint>
#include <etl/string.h>
#include <etl/string_view.h>
#include <etl/vector.h>

constexpr std::uint8_t MaxTopics = 1;

namespace drivers {
    class esp_at_wifi_mqtt {
    public:
        esp_at_wifi_mqtt() = default;
        ~esp_at_wifi_mqtt() = default;

        static bool connect_to_wifi(const etl::string_view &ssid, const etl::string_view &password);
        static bool connect_to_mqtt(const etl::string_view &broker, const etl::string_view &port);
        static bool configure_mqtt_user(etl::string_view username, etl::string_view password);
        static void publish_to_mqtt(etl::string_view topic, etl::string_view payload);

        void subscribe_to_mqtt(etl::string<32> topic);
        etl::string<128> poll_topic(etl::string<32> topic);
    private:
        etl::vector<etl::string<32>, MaxTopics> _topics;
    };
} // drivers
