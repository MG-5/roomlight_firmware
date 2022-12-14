/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EN_MOS3_Pin GPIO_PIN_13
#define EN_MOS3_GPIO_Port GPIOC
#define EN_MOS2_Pin GPIO_PIN_14
#define EN_MOS2_GPIO_Port GPIOC
#define EN_MOS1_Pin GPIO_PIN_15
#define EN_MOS1_GPIO_Port GPIOC
#define VOLTAGE_ADC_Pin GPIO_PIN_0
#define VOLTAGE_ADC_GPIO_Port GPIOA
#define CURRENT_ADC_Pin GPIO_PIN_1
#define CURRENT_ADC_GPIO_Port GPIOA
#define LED_RED_Pin GPIO_PIN_4
#define LED_RED_GPIO_Port GPIOA
#define LED_GREEN1_Pin GPIO_PIN_5
#define LED_GREEN1_GPIO_Port GPIOA
#define LED_GREEN2_Pin GPIO_PIN_6
#define LED_GREEN2_GPIO_Port GPIOA
#define LED_GREEN3_Pin GPIO_PIN_7
#define LED_GREEN3_GPIO_Port GPIOA
#define ESP_GPIO2_Pin GPIO_PIN_1
#define ESP_GPIO2_GPIO_Port GPIOB
#define ESP_GPIO0_Pin GPIO_PIN_2
#define ESP_GPIO0_GPIO_Port GPIOB
#define ESP_RST_Pin GPIO_PIN_10
#define ESP_RST_GPIO_Port GPIOB
#define ESP_EN_Pin GPIO_PIN_11
#define ESP_EN_GPIO_Port GPIOB
#define SwitchBlueChannel_Pin GPIO_PIN_15
#define SwitchBlueChannel_GPIO_Port GPIOA
#define SwitchGreenChannel_Pin GPIO_PIN_4
#define SwitchGreenChannel_GPIO_Port GPIOB
#define SwitchRedChannel_Pin GPIO_PIN_5
#define SwitchRedChannel_GPIO_Port GPIOB
#define Touch_Pin GPIO_PIN_6
#define Touch_GPIO_Port GPIOB
#define DATA1_Pin GPIO_PIN_7
#define DATA1_GPIO_Port GPIOB
#define DATA2_Pin GPIO_PIN_8
#define DATA2_GPIO_Port GPIOB
#define DATA3_Pin GPIO_PIN_9
#define DATA3_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
