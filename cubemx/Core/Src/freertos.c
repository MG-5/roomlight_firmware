/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 64 * 4
};
/* Definitions for statusLed */
osThreadId_t statusLedHandle;
const osThreadAttr_t statusLed_attributes = {
  .name = "statusLed",
  .priority = (osPriority_t) osPriorityLow1,
  .stack_size = 64 * 4
};
/* Definitions for adc */
osThreadId_t adcHandle;
const osThreadAttr_t adc_attributes = {
  .name = "adc",
  .priority = (osPriority_t) osPriorityLow2,
  .stack_size = 64 * 4
};
/* Definitions for digitalLED */
osThreadId_t digitalLEDHandle;
const osThreadAttr_t digitalLED_attributes = {
  .name = "digitalLED",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 4
};
/* Definitions for uartRX */
osThreadId_t uartRXHandle;
const osThreadAttr_t uartRX_attributes = {
  .name = "uartRX",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for uartTX */
osThreadId_t uartTXHandle;
const osThreadAttr_t uartTX_attributes = {
  .name = "uartTX",
  .priority = (osPriority_t) osPriorityNormal5,
  .stack_size = 64 * 4
};
/* Definitions for processPackets */
osThreadId_t processPacketsHandle;
const osThreadAttr_t processPackets_attributes = {
  .name = "processPackets",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 128 * 4
};
/* Definitions for wifiDaemon */
osThreadId_t wifiDaemonHandle;
const osThreadAttr_t wifiDaemon_attributes = {
  .name = "wifiDaemon",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 64 * 4
};
/* Definitions for ledFading */
osThreadId_t ledFadingHandle;
const osThreadAttr_t ledFading_attributes = {
  .name = "ledFading",
  .priority = (osPriority_t) osPriorityAboveNormal7,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
extern void statusLedTask(void *argument);
extern void adcTask(void *argument);
extern void digitalLEDTask(void *argument);
extern void uartRXTask(void *argument);
extern void uartTXTask(void *argument);
extern void processPacketsTask(void *argument);
extern void wifiDaemonTask(void *argument);
extern void ledFadingTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

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

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of statusLed */
  statusLedHandle = osThreadNew(statusLedTask, NULL, &statusLed_attributes);

  /* creation of adc */
  adcHandle = osThreadNew(adcTask, NULL, &adc_attributes);

  /* creation of digitalLED */
  digitalLEDHandle = osThreadNew(digitalLEDTask, NULL, &digitalLED_attributes);

  /* creation of uartRX */
  uartRXHandle = osThreadNew(uartRXTask, NULL, &uartRX_attributes);

  /* creation of uartTX */
  uartTXHandle = osThreadNew(uartTXTask, NULL, &uartTX_attributes);

  /* creation of processPackets */
  processPacketsHandle = osThreadNew(processPacketsTask, NULL, &processPackets_attributes);

  /* creation of wifiDaemon */
  wifiDaemonHandle = osThreadNew(wifiDaemonTask, NULL, &wifiDaemon_attributes);

  /* creation of ledFading */
  ledFadingHandle = osThreadNew(ledFadingTask, NULL, &ledFading_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

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

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
