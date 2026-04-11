#include "adc.h"
#include "cmsis_os2.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"

extern "C" {
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
}

void TestFunction(void *argument) {
    while (1) {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        osDelay(1000);
    }
}

int main() {
    // INIT Functions
    MX_GPIO_Init();
    MX_DMA_Init();
    SystemClock_Config();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_I2C2_Init();
    MX_SPI1_Init();
    MX_USART1_UART_Init();

    auto testFunctionTaskHandle = osThreadNew(TestFunction, NULL, new osThreadAttr_t{.name = "TestFunction", .stack_size = 128, .priority = osPriorityHigh});

    osKernelInitialize();
    MX_FREERTOS_Init();
    // Start FreeRTOS Scheduler
    osKernelStart();

    // We shouldn't hit this point as the scheduler should have taken over.
    return 1;
}
