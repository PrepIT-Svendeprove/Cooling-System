add_library(ethernet)

file(GLOB sources src/*.c)

target_sources(ethernet PRIVATE
        ${sources}
)

target_include_directories(ethernet PUBLIC
        inc
)

target_link_libraries(ethernet
    stm32cubemx
)