#include "hal_control_devs.h"

/************************************************************************
											Глобальные переменные модуля
*************************************************************************/
CDD_sObj CDD_Obj[CDD_OBJ_MAX];																						//  Массив с параметрами для каждого объекта управления
//-----------------------------------------------------------------------

/************************************************************************
											Прототипы всех функций модуля
*************************************************************************/
void	CDD_Init(void);																											// Инициализация модуля
void 	CDD_Handler(void);																									// Обработчик операций (положить main)
void 	CDD_Handler_Tm(void);																								// Обработчик операций в таймере (положить в таймер 1мс)

void  CDD_Set_State(u8 n, CDD_eState state);															// Задать состояние объекта управления
//-----------------------------------------------------------------------

//***********************************************************************
//								 		Описание всех функций модуля
//***********************************************************************
/**
	* @brief Инициализация модуля 
  */
void CDD_Init(void)
{
	for(u8 n = 0; n < CDD_OBJ_MAX; n++)
	{
		CDD_Obj[n].ReadPin   = HAL_GPIO_ReadPin;
		CDD_Obj[n].WritePin  = HAL_GPIO_WritePin;
		CDD_Obj[n].TogglePin = HAL_GPIO_TogglePin;
		CDD_Obj[n].level_a = true;
		
		CDD_Obj[n].cnt_t = 0;
		CDD_Obj[n].cnt_t_m = 500;	
		
		switch(n) 																												 																					
		{
//			case CDD_NAME_LED_CH_1: CDD_Obj[n].gpio = CDD_PORT_LED_CH1;																							
//															CDD_Obj[n].pin  = CDD_PIN_LED_CH1;
//															break;																					
//			case CDD_NAME_LED_CH_1:	CDD_Obj[n].gpio = CDD_PORT_LED_CH2;																							
//															CDD_Obj[n].pin  = CDD_PIN_LED_CH2;
//															break;																						
//			case CDD_NAME_LED_CH_1:	CDD_Obj[n].gpio = CDD_PORT_LED_CH3;																							
//															CDD_Obj[n].pin  = CDD_PIN_LED_CH3;
//															break;
//			case CDD_NAME_LED_CH_1: CDD_Obj[n].gpio = CDD_PORT_LED_CH4;																							
//															CDD_Obj[n].pin  = CDD_PIN_LED_CH4;
//															break;																					
//			case CDD_NAME_LED_CH_1:	CDD_Obj[n].gpio = CDD_PORT_LED_CH5;																							
//															CDD_Obj[n].pin  = CDD_PIN_LED_CH5;
//															break;																						
//			case CDD_NAME_LED_CH_1:	CDD_Obj[n].gpio = CDD_PORT_LED_CH6;																							
//															CDD_Obj[n].pin  = CDD_PIN_LED_CH6;
//															break;
//			case CDD_NAME_LED_VALVE:CDD_Obj[n].gpio = CDD_PORT_LED_VALVE;																							
//															CDD_Obj[n].pin  = CDD_PIN_LED_VALVE;							
//															break;
			case CDD_NAME_OUT_VALVE_CH_1:	CDD_Obj[n].gpio = CDD_PORT_OUT_VALVE_CH1;																							
																		CDD_Obj[n].pin  = CDD_PIN_OUT_VALVE_CH1;			
																		break;
			case CDD_NAME_OUT_VALVE_CH_2:	CDD_Obj[n].gpio = CDD_PORT_OUT_VALVE_CH2;																							
																		CDD_Obj[n].pin  = CDD_PIN_OUT_VALVE_CH2;			
																		break;
			default:	break;																			
		}
		CDD_Set_State(n, CDD_RSTAT_OFF);
	}
}

/**
	* @brief Обработчик операций (положить main)
  */
void CDD_Handler(void)																									
{
	
}

/**
	* @brief Обработчик операций в таймере (положить в таймер 1мс)
  */
void CDD_Handler_Tm(void)																									
{
	for(u8 n = 0; n < CDD_OBJ_MAX; n++)
	{
		if(CDD_Obj[n].state == CDD_RSTAT_BLINK)
		{
			CDD_Obj[n].cnt_t++;
			
			if(CDD_Obj[n].cnt_t == CDD_Obj[n].cnt_t_m)
			{
				CDD_Obj[n].cnt_t = 0;
				CDD_Set_State(n, CDD_Obj[n].state);
			}
		}
		else
		if(CDD_Obj[n].state == CDD_RSTAT_IDLE)
		{
			CDD_Obj[n].cnt_t++;
			
			if(CDD_Obj[n].cnt_t == CDD_VALVE_IDLE_MAX)
			{
				CDD_Obj[n].cnt_t = 0;
				CDD_Obj[n].TogglePin(CDD_Obj[n].gpio, CDD_Obj[n].pin);
		
				if(CDD_Obj[n].ReadPin(CDD_Obj[n].gpio, CDD_Obj[n].pin))
					CDD_Obj[n].state_pin = (CDD_Obj[n].level_a) ? true : false;
				else
					CDD_Obj[n].state_pin = (CDD_Obj[n].level_a) ? false : true;
				
				if(CDD_Obj[n].state_pin)
					CDD_Obj[n].state = CDD_RSTAT_ON;		
				else
					CDD_Obj[n].state = CDD_RSTAT_OFF;	
			}
		}
	}
}

/**
	* @brief 		 				  Задать состояние объекта управления
	* @param	[IN]     n: Номер объекта управления
	* @param	[IN] state: Состояние
  */
void CDD_Set_State(u8 n, CDD_eState state)
{
	if(state == CDD_RSTAT_ON)
	{
		if(CDD_Obj[n].level_a)
		{
			if(!CDD_Obj[n].state_pin)
			{
				CDD_Obj[n].WritePin(CDD_Obj[n].gpio, CDD_Obj[n].pin, GPIO_PIN_SET);
				CDD_Obj[n].state_pin = true;
			}
		}
		else
		{
			if(CDD_Obj[n].state_pin)
			{
				CDD_Obj[n].WritePin(CDD_Obj[n].gpio, CDD_Obj[n].pin, GPIO_PIN_RESET);
				CDD_Obj[n].state_pin = false;
			}
		}
	}
	else
	if(state == CDD_RSTAT_OFF)
	{
		if(CDD_Obj[n].level_a)
		{
			if(CDD_Obj[n].state_pin)
			{
				CDD_Obj[n].WritePin(CDD_Obj[n].gpio, CDD_Obj[n].pin, GPIO_PIN_RESET);
				CDD_Obj[n].state_pin = false;
			}
		}
		else
		{
			if(!CDD_Obj[n].state_pin)
			{
				CDD_Obj[n].WritePin(CDD_Obj[n].gpio, CDD_Obj[n].pin, GPIO_PIN_SET);
				CDD_Obj[n].state_pin = true;
			}
		}
	}
	else
	if(state == CDD_RSTAT_BLINK ||
		 state == CDD_RSTAT_IDLE)
	{
		CDD_Obj[n].TogglePin(CDD_Obj[n].gpio, CDD_Obj[n].pin);
		
		if(CDD_Obj[n].ReadPin(CDD_Obj[n].gpio, CDD_Obj[n].pin))
			CDD_Obj[n].state_pin = (CDD_Obj[n].level_a) ? true : false;
		else
			CDD_Obj[n].state_pin = (CDD_Obj[n].level_a) ? false : true;
	}
	CDD_Obj[n].state = state;
}
//------------------------------------------------------------------------
