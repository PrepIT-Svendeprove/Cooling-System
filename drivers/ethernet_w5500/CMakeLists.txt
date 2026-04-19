add_library(ethernet_w5500)

file(GLOB mqtt_sources MQTT/MQTTPacket/src/*.c MQTT/*.c)

target_sources(ethernet_w5500 PRIVATE
        socket.c
        wizchip_conf.c
        W5500/w5500.c
        DHCP/dhcp.c
        DHCP6/dhcpv6.c
        DNS/dns.c
        AAC/AddressAutoConfig.c
        ${mqtt_sources}
)

target_include_directories(ethernet_w5500 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        W5500
        DHCP
        DHCP6
        DNS
        AAC
        MQTT
        MQTT/MQTTPacket/src
)

add_compile_definitions(_WIZCHIP_=5500)