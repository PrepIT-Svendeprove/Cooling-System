
#include "serialport.hpp"


namespace drivers {
    serialport::serialport(UART_HandleTypeDef* huart) : _huart(huart) {
    }
}