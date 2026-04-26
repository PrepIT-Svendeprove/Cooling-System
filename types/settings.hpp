#pragma once
#include <etl/string.h>

namespace types {
    struct settings {
        etl::string<10> deviceCode;
        etl::string<48> mqttBroker;
        etl::string<4> mqttPort{"1883"};
        etl::string<48> mqttUsername;
        etl::string<48> mqttPassword;
        etl::string<48> wifiSsid;
        etl::string<48> wifiPassword;
    };
}
