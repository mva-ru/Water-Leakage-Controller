/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "i2c.h"

/* USER CODE BEGIN 0 */
typedef struct 
{
  I2C_HandleTypeDef*  instance;
  uint16_t            sdaPin;
  GPIO_TypeDef*       sdaPort;
  uint16_t            sclPin;
  GPIO_TypeDef*       sclPort;
} I2C_Module;

I2C_Module i2c_strc = 
{
	&hi2c1,
	GPIO_PIN_7,	//sdaPin;
	GPIOB, 			//sdaPort;
	GPIO_PIN_6,	//sclPin;
	GPIOB, 			//sclPort;
};

// РїСЂРµРёРЅРёС†РёР°Р»РёР·РёСЂСѓРµР�? I2C !!! С‡С‚РѕР±С‹ СЃР±СЂРѕСЃС‚РёСЊ РѕС€РёР±РєСѓ BUSY
void I2C_ClearBusyFlagErratum(I2C_Module* i2c)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
	// РїРµСЂРµРЅРµСЃ СЃСЋРґР°, РїРѕС‚РѕР�?Сѓ С‡С‚Рѕ РІ /* USER CODE BEGIN SysInit */ РµС‰Рµ РЅРµ РІРєР»СЋС‡РµРЅС‹ РїСЂРµСЂС‹РІР°РЅРёСЏ, Рё Р·Р°РІРёСЃР°РµС‚ РІ HAL_Delay
	__HAL_RCC_I2C2_CLK_ENABLE();
	HAL_Delay(100);
	__HAL_RCC_I2C2_FORCE_RESET();
	HAL_Delay(100);
	__HAL_RCC_I2C2_RELEASE_RESET();
	HAL_Delay(100);
	
  // 1. Clear PE bit.
  i2c->instance->Instance->CR1 &= ~(0x0001);

  //  2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
  GPIO_InitStructure.Mode         = GPIO_MODE_OUTPUT_OD;
//  GPIO_InitStructure.Alternate    = I2C_PIN_MAP;
  GPIO_InitStructure.Pull         = GPIO_PULLUP;
  GPIO_InitStructure.Speed        = GPIO_SPEED_FREQ_HIGH;

  GPIO_InitStructure.Pin          = i2c->sclPin;
  HAL_GPIO_Init(i2c->sclPort, &GPIO_InitStructure);
  HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET);

  GPIO_InitStructure.Pin          = i2c->sdaPin;
  HAL_GPIO_Init(i2c->sdaPort, &GPIO_InitStructure);
  HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET);

  // 3. Check SCL and SDA High level in GPIOx_IDR.
  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sclPort, i2c->sclPin)) __nop();

  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sdaPort, i2c->sdaPin)) __nop();

  // 4. Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
  HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_RESET);

  //  5. Check SDA Low level in GPIOx_IDR.
  while (GPIO_PIN_RESET != HAL_GPIO_ReadPin(i2c->sdaPort, i2c->sdaPin)) __nop();

  // 6. Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
  HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_RESET);

  //  7. Check SCL Low level in GPIOx_IDR.
  while (GPIO_PIN_RESET != HAL_GPIO_ReadPin(i2c->sclPort, i2c->sclPin)) __nop();

  // 8. Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
  HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET);

  // 9. Check SCL High level in GPIOx_IDR.
  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sclPort, i2c->sclPin)) __nop();

  // 10. Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to GPIOx_ODR).
  HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET);

  // 11. Check SDA High level in GPIOx_IDR.
  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sdaPort, i2c->sdaPin)) __nop();

  // 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
  GPIO_InitStructure.Mode         = GPIO_MODE_AF_OD;
//  GPIO_InitStructure.Alternate    = I2C_PIN_MAP;

  GPIO_InitStructure.Pin          = i2c->sclPin;
  HAL_GPIO_Init(i2c->sclPort, &GPIO_InitStructure);

  GPIO_InitStructure.Pin          = i2c->sdaPin;
  HAL_GPIO_Init(i2c->sdaPort, &GPIO_InitStructure);

  // 13. Set SWRST bit in I2Cx_CR1 register.
  i2c->instance->Instance->CR1 |= 0x8000;

  __nop();

  // 14. Clear SWRST bit in I2Cx_CR1 register.
  i2c->instance->Instance->CR1 &= ~0x8000;

  __nop();

  // 15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register
  i2c->instance->Instance->CR1 |= 0x0001;

  // Call initialization function.
  HAL_I2C_Init(i2c->instance);
}
/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */
//	I2C_ClearBusyFlagErratum(&i2c_strc);
  /* USER CODE END I2C1_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
