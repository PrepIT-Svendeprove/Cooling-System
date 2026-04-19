add_library(ethernet_enc28j60)

file(GLOB sources src/*.c)

target_sources(ethernet_enc28j60 PRIVATE
        ${sources}
)

target_include_directories(ethernet_enc28j60 PUBLIC
        inc
)

target_link_libraries(ethernet_enc28j60
    stm32cubemx
)