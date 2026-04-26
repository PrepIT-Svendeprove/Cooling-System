#include "adc.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "tasks/external_cool_task.hpp"
#include <cstdint>

#include "climate_control_task.hpp"
#include "commands.hpp"
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "tasks/communication_task.hpp"

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

namespace {
    osMessageQueueAttr_t ambient_temp_attr = {.name = "ambient_temp"};
    osMessageQueueId_t ambient_temperatures_queue_id = osMessageQueueNew(3, sizeof(std::uint32_t), &ambient_temp_attr);
    osMessageQueueAttr_t internal_temp_attr = {.name = "internal_temp"};
    osMessageQueueId_t internal_temperatures_queue_id = osMessageQueueNew(3, sizeof(std::int32_t), &internal_temp_attr);
    osMessageQueueAttr_t internal_humidity_attr = {.name = "internal_humidity"};
    osMessageQueueId_t internal_humidity_queue_id = osMessageQueueNew(3, sizeof(std::int32_t), &internal_humidity_attr);
    osMessageQueueAttr_t target_temp_attr = {.name = "target_temp"};
    osMessageQueueId_t target_temperatures_queue_id = osMessageQueueNew(3, sizeof(std::int32_t), &target_temp_attr);
    osMessageQueueAttr_t target_humidity_attr = {.name = "target_humidity"};
    osMessageQueueId_t target_humidity_queue_id = osMessageQueueNew(3, sizeof(std::int32_t), &target_humidity_attr);
    osMessageQueueAttr_t control_commands_attr = {.name = "control_commands"};
    osMessageQueueId_t control_commands_queue_id = osMessageQueueNew(3, sizeof(types::command), &control_commands_attr);

    tasks::external_cool_task ext_cool_task{ambient_temperatures_queue_id};
    tasks::communication_task comms_task{
        ambient_temperatures_queue_id,
        internal_temperatures_queue_id,
        target_temperatures_queue_id,
        control_commands_queue_id
    };
    tasks::climate_control_task cc_task{
        internal_temperatures_queue_id,
        internal_humidity_queue_id,
        target_temperatures_queue_id,
        target_humidity_queue_id,
        control_commands_queue_id
    };


    const osThreadAttr_t ext_cool_attr = {
        .name = "Ext_Cooling",
        .stack_size = 2048,
        .priority = osPriorityAboveNormal
    };

    const osThreadAttr_t communication_attr = {
        .name = "Communication",
        .stack_size = 8192,
        .priority = osPriorityHigh
    };

    const osThreadAttr_t climate_control_attr = {
        .name = "Climate_Control",
        .stack_size = 2048,
        .priority = osPriorityHigh1
    };

    auto ext_cool_task_handle = osThreadNew(tasks::external_cool_task::run_wrapper, &ext_cool_task, &ext_cool_attr);
    auto communication_task_handle = osThreadNew(tasks::communication_task::run_wrapper, &comms_task,
                                                 &communication_attr);
    auto climate_control_task_handle = osThreadNew(tasks::climate_control_task::run_wrapper, &cc_task,
                                                   &climate_control_attr);
    auto testFunctionTaskHandle = osThreadNew(TestFunction, NULL, new osThreadAttr_t{
                                                  .name = "TestFunction", .stack_size = 128,
                                                  .priority = osPriorityNormal
                                              });
}

int main() {
    // INIT Functions
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_I2C2_Init();
    MX_SPI1_Init();
    MX_USART1_UART_Init();
    //MX_USART2_UART_Init();
    MX_USART6_UART_Init();

    osKernelInitialize();
    MX_FREERTOS_Init();
    // Start FreeRTOS Scheduler
    osKernelStart();

    // We shouldn't hit this point as the scheduler should have taken over.
    return 1;
}
