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
#define VBAT_Pin GPIO_PIN_0
#define VBAT_GPIO_Port GPIOA
#define PBAT_Pin GPIO_PIN_1
#define PBAT_GPIO_Port GPIOA
#define OUT_NRF_CE_Pin GPIO_PIN_4
#define OUT_NRF_CE_GPIO_Port GPIOA
#define OUT_NRF_CSN_Pin GPIO_PIN_0
#define OUT_NRF_CSN_GPIO_Port GPIOB
#define IN_NRF_IRQ_Pin GPIO_PIN_1
#define IN_NRF_IRQ_GPIO_Port GPIOB
#define IN_NRF_IRQ_EXTI_IRQn EXTI1_IRQn
#define V_OPEN_Pin GPIO_PIN_10
#define V_OPEN_GPIO_Port GPIOB
#define V_CLOSE_Pin GPIO_PIN_11
#define V_CLOSE_GPIO_Port GPIOB
#define SPI2_LE_Pin GPIO_PIN_12
#define SPI2_LE_GPIO_Port GPIOB
#define SPI2_OE_Pin GPIO_PIN_14
#define SPI2_OE_GPIO_Port GPIOB
#define B_RESET_Pin GPIO_PIN_3
#define B_RESET_GPIO_Port GPIOB
#define B_CLOSE_Pin GPIO_PIN_4
#define B_CLOSE_GPIO_Port GPIOB
#define B_OPEN_Pin GPIO_PIN_5
#define B_OPEN_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_9
#define BUZZER_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
