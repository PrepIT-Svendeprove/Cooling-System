#pragma once
#include <etl/array.h>

#include "stm32f4xx_hal_uart.h"

namespace drivers {
    class serialport {
        public:
        serialport(UART_HandleTypeDef* huart);
        ~serialport();

        template<std::size_t N>
        int read(etl::array<uint8_t, N>& buffer) {

        }

        int write(const std::uint8_t* buffer, const std::size_t size, const std::uint32_t timeout = 100) {
            return HAL_UART_Transmit(_huart, buffer, size, timeout);
        }
    private:
        UART_HandleTypeDef* _huart;
    };
}

