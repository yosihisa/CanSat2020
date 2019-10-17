/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "stm32l0xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_L0_Pin GPIO_PIN_13
#define LED_L0_GPIO_Port GPIOC
#define FLIGHT_PIN_Pin GPIO_PIN_14
#define FLIGHT_PIN_GPIO_Port GPIOC
#define SUB_EN_Pin GPIO_PIN_15
#define SUB_EN_GPIO_Port GPIOC
#define SUB_TX_Pin GPIO_PIN_0
#define SUB_TX_GPIO_Port GPIOA
#define SUB_RX_Pin GPIO_PIN_1
#define SUB_RX_GPIO_Port GPIOA
#define GNSS_TX_Pin GPIO_PIN_2
#define GNSS_TX_GPIO_Port GPIOA
#define GNSS_RX_Pin GPIO_PIN_3
#define GNSS_RX_GPIO_Port GPIOA
#define LED_TX_Pin GPIO_PIN_10
#define LED_TX_GPIO_Port GPIOB
#define Nichrome_Pin GPIO_PIN_11
#define Nichrome_GPIO_Port GPIOB
#define PC_TX_Pin GPIO_PIN_3
#define PC_TX_GPIO_Port GPIOB
#define PC_RX_Pin GPIO_PIN_4
#define PC_RX_GPIO_Port GPIOB
#define COM_TX_Pin GPIO_PIN_6
#define COM_TX_GPIO_Port GPIOB
#define COM_RX_Pin GPIO_PIN_7
#define COM_RX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
