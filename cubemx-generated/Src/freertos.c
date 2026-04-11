/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ClimateControl */
osThreadId_t ClimateControlHandle;
const osThreadAttr_t ClimateControl_attributes = {
  .name = "ClimateControl",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Communication */
osThreadId_t CommunicationHandle;
const osThreadAttr_t Communication_attributes = {
  .name = "Communication",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityHigh1,
};
/* Definitions for EXT_CoolFans */
osThreadId_t EXT_CoolFansHandle;
const osThreadAttr_t EXT_CoolFans_attributes = {
  .name = "EXT_CoolFans",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for Internal_Temperatures */
osMessageQueueId_t Internal_TemperaturesHandle;
uint8_t TemperatureBuffer[ 16 * sizeof( uint16_t ) ];
osStaticMessageQDef_t TemperatureControlBlock;
const osMessageQueueAttr_t Internal_Temperatures_attributes = {
  .name = "Internal_Temperatures",
  .cb_mem = &TemperatureControlBlock,
  .cb_size = sizeof(TemperatureControlBlock),
  .mq_mem = &TemperatureBuffer,
  .mq_size = sizeof(TemperatureBuffer)
};
/* Definitions for Ambient_Temperatures */
osMessageQueueId_t Ambient_TemperaturesHandle;
uint8_t Ambient_TemperaturesBuffer[ 16 * sizeof( uint16_t ) ];
osStaticMessageQDef_t Ambient_TemperaturesControlBlock;
const osMessageQueueAttr_t Ambient_Temperatures_attributes = {
  .name = "Ambient_Temperatures",
  .cb_mem = &Ambient_TemperaturesControlBlock,
  .cb_size = sizeof(Ambient_TemperaturesControlBlock),
  .mq_mem = &Ambient_TemperaturesBuffer,
  .mq_size = sizeof(Ambient_TemperaturesBuffer)
};
/* Definitions for CTRL_Temperature */
osMessageQueueId_t CTRL_TemperatureHandle;
uint8_t CTRL_TemperatureBuffer[ 2 * sizeof( uint16_t ) ];
osStaticMessageQDef_t CTRL_TemperatureControlBlock;
const osMessageQueueAttr_t CTRL_Temperature_attributes = {
  .name = "CTRL_Temperature",
  .cb_mem = &CTRL_TemperatureControlBlock,
  .cb_size = sizeof(CTRL_TemperatureControlBlock),
  .mq_mem = &CTRL_TemperatureBuffer,
  .mq_size = sizeof(CTRL_TemperatureBuffer)
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartClimateControlTask(void *argument);
void StartCommunicationTask(void *argument);
void StartExtCoolingTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Internal_Temperatures */
  Internal_TemperaturesHandle = osMessageQueueNew (16, sizeof(uint16_t), &Internal_Temperatures_attributes);

  /* creation of Ambient_Temperatures */
  Ambient_TemperaturesHandle = osMessageQueueNew (16, sizeof(uint16_t), &Ambient_Temperatures_attributes);

  /* creation of CTRL_Temperature */
  CTRL_TemperatureHandle = osMessageQueueNew (2, sizeof(uint16_t), &CTRL_Temperature_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of ClimateControl */
  ClimateControlHandle = osThreadNew(StartClimateControlTask, NULL, &ClimateControl_attributes);

  /* creation of Communication */
  CommunicationHandle = osThreadNew(StartCommunicationTask, NULL, &Communication_attributes);

  /* creation of EXT_CoolFans */
  EXT_CoolFansHandle = osThreadNew(StartExtCoolingTask, NULL, &EXT_CoolFans_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartClimateControlTask */
/**
* @brief Function implementing the ClimateControl thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartClimateControlTask */
void StartClimateControlTask(void *argument)
{
  /* USER CODE BEGIN StartClimateControlTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartClimateControlTask */
}

/* USER CODE BEGIN Header_StartCommunicationTask */
/**
* @brief Function implementing the Communication thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCommunicationTask */
void StartCommunicationTask(void *argument)
{
  /* USER CODE BEGIN StartCommunicationTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartCommunicationTask */
}

/* USER CODE BEGIN Header_StartExtCoolingTask */
/**
* @brief Function implementing the EXT_CoolFans thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartExtCoolingTask */
void StartExtCoolingTask(void *argument)
{
  /* USER CODE BEGIN StartExtCoolingTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartExtCoolingTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

