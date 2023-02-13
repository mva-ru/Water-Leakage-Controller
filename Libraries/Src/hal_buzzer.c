#include "hal_buzzer.h"

BUZ_sObj BUZ_obj;																													// Объект управления зуммером

/************************************************************************
											Глобальные переменные модуля
*************************************************************************/

//-----------------------------------------------------------------------

/************************************************************************
											Прототипы всех функций модуля
*************************************************************************/
void BUZ_Init(void);																											// Инициализация модуля модуля 
void BUZ_Handler(void);		 																								// Обработчик модуля (положить в Main)
void BUZ_Handler_Tm(void);																								// Обработчик интервалов для модуля (положить в таймер 1мс)

void BUZ_Set_State_Pin_Control(bool state);																// Задать состояние контакта управления звуковым оповещением
bool BUZ_Get_State_Pin_Control(void);																			// Получить состояние контакта управления звуковым оповещением
//------------------------------------------------------------------------

/**
	* @brief Инициализация модуля
  */
void BUZ_Init(void)
{
	BUZ_obj.gpio = BUZ_PORT_CONTROL;																							
	BUZ_obj.pin  = BUZ_PIN_CONTROL;	
	
	BUZ_obj.ReadPin   = HAL_GPIO_ReadPin;
	BUZ_obj.WritePin  = HAL_GPIO_WritePin;
	BUZ_obj.TogglePin = HAL_GPIO_TogglePin;

	BUZ_obj.level_a = true;
	BUZ_obj.fl_mute = true;	
	BUZ_obj.fl_on_alarm = false;
	BUZ_obj.cnt_tone = 0;
	BUZ_obj.cnt_tone_m = BUZ_TONE_1_MS;	
	BUZ_obj.fl_tm_alarm = false;					
	BUZ_obj.cnt_tm = 0;
	BUZ_obj.cnt_tm_m = BUZ_TIME_ON_MS;	
}

/**
	* @brief Обработчик модуля (положить в Main)
  */
void BUZ_Handler(void)
{
	
}

/**
	* @brief Обработчик интервалов для модуля (положить в таймер 1мс)
  */
void BUZ_Handler_Tm(void)
{
	if(BUZ_obj.fl_on_alarm)
	{
		if(!BUZ_obj.fl_mute)
		{
			BUZ_obj.cnt_tone++;
			
			if(BUZ_obj.cnt_tone == BUZ_obj.cnt_tone_m)
			{
				BUZ_obj.cnt_tone = 0;
				HAL_GPIO_TogglePin(BUZ_PORT_CONTROL, BUZ_PIN_CONTROL);
			}
		}
		if(BUZ_obj.fl_tm_alarm)
		{
			BUZ_obj.cnt_tm++;
			
			if(BUZ_obj.cnt_tm >= BUZ_obj.cnt_tm_m)
			{
				BUZ_obj.cnt_tm = 0;
				BUZ_obj.fl_on_alarm = false;
				BUZ_obj.fl_tm_alarm = false;
			}	
		}
	}
	else
		if(BUZ_Get_State_Pin_Control())
			BUZ_Set_State_Pin_Control(false);	
}	

/**
	* @brief 		 				  Задать состояние контакта управления звуковым оповещением
	* @param	[IN] state: Флаг состояния (true - вкл.)
  */
void BUZ_Set_State_Pin_Control(bool state)
{
	if(state)
	{
		if(BUZ_obj.level_a)
			BUZ_obj.WritePin(BUZ_obj.gpio, BUZ_obj.pin, GPIO_PIN_SET);
		else
			BUZ_obj.WritePin(BUZ_obj.gpio, BUZ_obj.pin, GPIO_PIN_RESET);
	}
	else
	{
		if(BUZ_obj.level_a)
			BUZ_obj.WritePin(BUZ_obj.gpio, BUZ_obj.pin, GPIO_PIN_RESET);
		else
			BUZ_obj.WritePin(BUZ_obj.gpio, BUZ_obj.pin, GPIO_PIN_SET);	
	}
}

/**
	* @brief 	 Получить состояние контакта управления звуковым оповещением
	* @retval	 Возвращает флаг состояния (true - вкл.)
  */
bool BUZ_Get_State_Pin_Control(void)
{
	if(HAL_GPIO_ReadPin(BUZ_PORT_CONTROL, BUZ_PIN_CONTROL))
	{
		if(BUZ_obj.level_a)
			return true;
		else
			return false;
	}
	else
	{
		if(BUZ_obj.level_a)
			return true;
		else
			return false;
	}
}
//------------------------------------------------------------------------
