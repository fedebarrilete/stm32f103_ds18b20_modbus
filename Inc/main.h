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
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#define LED0_Pin GPIO_PIN_13
#define LED0_GPIO_Port GPIOC
#define Clear_Pin GPIO_PIN_4
#define Clear_GPIO_Port GPIOA
#define W5500_CS_Pin GPIO_PIN_0
#define W5500_CS_GPIO_Port GPIOB
#define DHT22_Pin GPIO_PIN_1
#define DHT22_GPIO_Port GPIOB
#define DHT22_EXTI_IRQn EXTI1_IRQn
#define Boot1_Pin GPIO_PIN_2
#define Boot1_GPIO_Port GPIOB
#define LCD_LATCH_Pin GPIO_PIN_12
#define LCD_LATCH_GPIO_Port GPIOB
#define HC595_LATCH_Pin GPIO_PIN_8
#define HC595_LATCH_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_11
#define BUZZER_GPIO_Port GPIOA
#define DOOR_SW_Pin GPIO_PIN_12
#define DOOR_SW_GPIO_Port GPIOA
#define ENC_SW_Pin GPIO_PIN_15
#define ENC_SW_GPIO_Port GPIOA
#define ENC_SW_EXTI_IRQn EXTI15_10_IRQn
#define DBG_Pin GPIO_PIN_3
#define DBG_GPIO_Port GPIOB
#define W25Q16_CS_Pin GPIO_PIN_8
#define W25Q16_CS_GPIO_Port GPIOB
#define W5500_RST_Pin GPIO_PIN_9
#define W5500_RST_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

//#define USE_UPRINTF

	
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
