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
#include "hal_nrf24l01_api.h"
#include "hal_nrf24l01_drv.h"
#include "hal_mbus_ascii_slave.h"
#include "hal_mbus_ascii.h"
#include "hal_config.h"
#include "hal_flash.h"
#include "usbd_cdc_if.h"
#include "hal_control_devs.h"
#include "hal_algorithm.h"
#include "hal_buttons.h"
#include "hal_buzzer.h"
#include "hal_i2c_1602.h"
#include "hal_view_data.h"
#include "hal_my_rtc.h"
#include "hal_my_adc.h"
#include "spi.h"
#include "i2c.h"
#include "adc.h"
#include "tim.h"
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
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task01 */
osThreadId_t Task01Handle;
const osThreadAttr_t Task01_attributes = {
  .name = "Task01",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task02 */
osThreadId_t Task02Handle;
const osThreadAttr_t Task02_attributes = {
  .name = "Task02",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Init_All_Moduls(void); 											
void My_System_Reset(void);													 
void Jump_Boot_Loader(void);									
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask01(void *argument);
void StartTask02(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	LCD1602_Init(&hi2c1);
	VD_Init();
	NRF24_Init(&hspi1, NRF24_TYPE_RX_TX_WHILE);
	ADC_Init(&hadc1);
	RTC_Init(&hrtc);
	ASCII_Init();
	CDD_Init();
	BTN_Init();	
	BUZ_Init();
	ALG_Init();
	CFG_Init();
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
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

  /* creation of Task01 */
  Task01Handle = osThreadNew(StartTask01, NULL, &Task01_attributes);

//  /* creation of Task02 */
  Task02Handle = osThreadNew(StartTask02, NULL, &Task02_attributes);

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
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
		static bool fl_init_hcdc_ok = false;
		
		if(!fl_init_hcdc_ok)
			if(hUsbDeviceFS.pClassData != NULL)
			{
				ASCII_Init_Port(     ASCII_IPORT_1,            		 ASCII_TYPE_PORT_USB_COM, 
											  (u8*)hUsbDeviceFS.pClassData, 		 ASCII_MODE_PRTKL_SLAVE,
												(u8*)UserRxBufferFS,	        (u8*)UserTxBufferFS);
				
				fl_init_hcdc_ok = true;
			}
		ASCII_Handler();
		CDD_Handler();
		RTC_Handler();
		ADC_Handler();
		ALG_Handler();
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask01 */
/**
* @brief Function implementing the Task01 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask01 */
void StartTask01(void *argument)
{
  /* USER CODE BEGIN StartTask01 */
		
  /* Infinite loop */
  for(;;)
  {
    NRF24_Stream_Handler();
  }
  /* USER CODE END StartTask01 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the Task02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {
		VD_Handler_View();
		CFG_Handler();
  }
  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/**
  * @brief 
  */
void Init_All_Moduls(void)
{
//	__set_PRIMASK(1);									  										
//	SCB->VTOR = (u32)0x08004800;  														
//	__set_PRIMASK(0);								  									
	
	CFG_setup.uid_cpu[0] = HAL_GetUIDw0();
	CFG_setup.uid_cpu[1] = HAL_GetUIDw1();
	CFG_setup.uid_cpu[2] = HAL_GetUIDw2();
  CFG_Read_All_Cfg_And_Check_Stat();											
}

/**
  * @brief
  */
void Jump_Boot_Loader(void)
{
	if(ASCII_Regs_Jump_BootLdr == 0xA55A && 				
		 ASCII_Port[ASCII_IPORT_1].statTx != ASCII_STAT_TX_SEND)
	{	
		HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);		
		HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
		HAL_NVIC_DisableIRQ(EXTI1_IRQn);
		HAL_NVIC_DisableIRQ(TIM2_IRQn);				  											  									
		HAL_NVIC_DisableIRQ(SPI1_IRQn);
		HAL_NVIC_DisableIRQ(RTC_IRQn);
		HAL_NVIC_DisableIRQ(RCC_IRQn);
		HAL_DeInit();																						
		Flash_Jump_Adress(0x08000000);													
	}
}

/**
  * @brief
  */	
void My_System_Reset(void)
{
	if(ASCII_Regs_Jump_BootLdr == 0xF00F && 				
		 ASCII_Port[ASCII_IPORT_1].statTx != ASCII_STAT_TX_SEND)
		HAL_NVIC_SystemReset();
}
/* USER CODE END Application */
