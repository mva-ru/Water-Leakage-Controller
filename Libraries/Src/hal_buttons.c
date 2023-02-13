#include "hal_buttons.h"

/************************************************************************
											Глобальные переменные модуля
*************************************************************************/
BTN_sObj BTN_Obj[BTN_OBJ_MAX];																						// Массив с параметрами для каждой клавиши
//-----------------------------------------------------------------------

/************************************************************************
											Прототипы всех функций модуля
*************************************************************************/
void	BTN_Init(void);																											// Инициализация модуля
void 	BTN_Handler_Tm(void);																								// Обработчик операций в таймере (положить в таймер 1мс)
void 	BTN_Identify_State(u8 n);																						// Определить состояние клавиши
//-----------------------------------------------------------------------

//***********************************************************************
//								 		Описание всех функций модуля
//***********************************************************************
/**
	* @brief Инициализация модуля 
  */
void BTN_Init(void)
{
	for(u8 i = 0; i < BTN_OBJ_MAX; i++)
	{
		BTN_Obj[i].level_a  		 		= false;
		BTN_Obj[i].stat_sav_work1	  = false;
		BTN_Obj[i].stat_sav_work2	  = false;
		BTN_Obj[i].stat_sav_hold_3s = false;
		BTN_Obj[i].Read_Stat 		 	  = HAL_GPIO_ReadPin;
		
		if(BTN_Obj[i].level_a)
		{
			BTN_Obj[i].level_now  = false;
			BTN_Obj[i].level_last = false;
		}
		else
		{
			BTN_Obj[i].level_now  = true;
			BTN_Obj[i].level_last = true;
		}
		BTN_Obj[i].cnt_t   	= 0;
		BTN_Obj[i].cnt_d   	= 0;
		BTN_Obj[i].cnt_dr   = 0;
		BTN_Obj[i].cnt_t_m 	= 10;												// время одного периода опроса уровня, мс
		BTN_Obj[i].cnt_d_m	= 5;												// период фильтра дребезга, кол-во
		BTN_Obj[i].cnt_dr_m	= 100;											// время удержания, кол-во		

		switch(i) 																												 																					
		{
			case 0: BTN_Obj[i].gpio = BTN_PORT_ON;																							
							BTN_Obj[i].pin  = BTN_PIN_ON;
							BTN_Obj[i].name = BTN_NAME_ON;
							break;																					
			case 1:	BTN_Obj[i].gpio = BTN_PORT_OFF;																							
							BTN_Obj[i].pin  = BTN_PIN_OFF;
							BTN_Obj[i].name = BTN_NAME_OFF;
							break;																						
			case 2:	BTN_Obj[i].gpio = BTN_PORT_MUTE;																							
							BTN_Obj[i].pin  = BTN_PIN_MUTE;
							BTN_Obj[i].name = BTN_NAME_MUTE;
							break;	
			default:break;																			
		}
	}
}

/**
	* @brief Обработчик операций в таймере (положить в таймер 1мс)
  */
void BTN_Handler_Tm(void)																									
{
	for(u8 i = 0; i < BTN_OBJ_MAX; i++)
	{
		BTN_Obj[i].cnt_t++;
		
		if(BTN_Obj[i].cnt_t == BTN_Obj[i].cnt_t_m)
		{
			BTN_Obj[i].cnt_t = 0;
			BTN_Obj[i].level_now = BTN_Obj[i].Read_Stat(BTN_Obj[i].gpio, BTN_Obj[i].pin);
			
			if(BTN_Obj[i].level_now == BTN_Obj[i].level_last)
				BTN_Obj[i].cnt_d++;
			else
				BTN_Obj[i].cnt_d = 0;
			
			if(BTN_Obj[i].cnt_d == BTN_Obj[i].cnt_d_m)												// Фильтр от дребезга
			{
				BTN_Obj[i].cnt_d = 0;
				BTN_Obj[i].buf_l[BTN_Obj[i].cnt_l] = BTN_Obj[i].level_now;
				BTN_Identify_State(i);																					// Определить состояние клавиши
				
				if(!BTN_Obj[i].block_i)																					// Флаг - блок инкримента элемента массива логических состояний
				{
					if(BTN_Obj[i].cnt_l < BTN_LEV_MAX)
						BTN_Obj[i].cnt_l++;
				}
				else
					BTN_Obj[i].block_i = 0;
			}
			BTN_Obj[i].level_last = BTN_Obj[i].level_now;
		}
	}
}

/**
	* @brief 		 			Определить состояние клавиши
	* @param	[IN] n: Номер клавиши
  */
void BTN_Identify_State(u8 n)
{
	if(!BTN_Obj[n].cnt_l)
	{
		if(BTN_Obj[n].buf_l[0] && !BTN_Obj[n].level_a)
		{
			BTN_Obj[n].block_i = true;
			BTN_Obj[n].stat_r_sav = BTN_RSTAT_DEPRESS_1;
		}
		else				
		if(!BTN_Obj[n].buf_l[0] && BTN_Obj[n].level_a)
		{
			BTN_Obj[n].block_i = true;
			BTN_Obj[n].stat_r_sav = BTN_RSTAT_DEPRESS_0;
		}
		else
		if(!BTN_Obj[n].buf_l[0] && !BTN_Obj[n].level_a)
		{
			BTN_Obj[n].stat_r_sav = BTN_RSTAT_PRESS_1_0;																// кнопка нажата
			BTN_Obj[n].stat_sav_work1 = true;
		}
		else
		if(BTN_Obj[n].buf_l[0] && BTN_Obj[n].level_a)
		{
			BTN_Obj[n].stat_r_sav = BTN_RSTAT_PRESS_0_1;																// кнопка нажата
			BTN_Obj[n].stat_sav_work1 = true;
		}
	}
	if(BTN_Obj[n].cnt_l == 1)
	{
		if(BTN_Obj[n].buf_l[0] && !BTN_Obj[n].buf_l[1] && BTN_Obj[n].level_a)
		{
			BTN_Obj[n].stat_r_sav = BTN_RSTAT_PRESS_1_0_1;															// кнопка нажата-отжата
			BTN_Obj[n].stat_sav_work2 = true;
			BTN_Obj[n].fl_hold_3s_r = false;
			BTN_Obj[n].cnt_l = 0;
		}
		else	
			if(!BTN_Obj[n].buf_l[0] && BTN_Obj[n].buf_l[1] && !BTN_Obj[n].level_a)
			{
				BTN_Obj[n].stat_r_sav = BTN_RSTAT_PRESS_0_1_0;														// кнопка нажата-отжата
				BTN_Obj[n].stat_sav_work2 = true;
				BTN_Obj[n].fl_hold_3s_r = false;
				BTN_Obj[n].cnt_l = 0;
			}
			else	
				if(BTN_Obj[n].buf_l[0] && BTN_Obj[n].buf_l[1] && BTN_Obj[n].level_a)
					BTN_Obj[n].stat_r_sav = BTN_RSTAT_HOLD_1;																// кнопка удерживается					
				else						
					if(!BTN_Obj[n].buf_l[0] && !BTN_Obj[n].buf_l[1] && !BTN_Obj[n].level_a)
						BTN_Obj[n].stat_r_sav = BTN_RSTAT_HOLD_0;															// кнопка удерживается
	
		if(BTN_Obj[n].stat_r_sav == BTN_RSTAT_HOLD_0 || 
			 BTN_Obj[n].stat_r_sav == BTN_RSTAT_HOLD_1)																	// Идет удержание уровня
		{
			if(!BTN_Obj[n].stat_sav_hold_3s && 																					// Не взведен флаг - сохранить статус удержания уровня
				 !BTN_Obj[n].fl_hold_3s_r)																								// Сброшен флаг - удержание 3с было, но отжатия не было																									
			{
				BTN_Obj[n].cnt_dr++;
				
				if(BTN_Obj[n].cnt_dr == BTN_Obj[n].cnt_dr_m)															// Отсчет времени удержания
				{
					BTN_Obj[n].cnt_dr = 0;
					BTN_Obj[n].fl_hold_3s_r = true;																					// взведен флаг - удержание 3с было, но отжатия не было	
					BTN_Obj[n].stat_sav_hold_3s = true;
					
					if(BTN_Obj[n].level_a)																									// в зависимости от активного уровня
						BTN_Obj[n].stat_r_sav = BTN_RSTAT_HOLD_1_3s;													// выставить расширенный статус
					else
						BTN_Obj[n].stat_r_sav = BTN_RSTAT_HOLD_0_3s;
				}
			}
		}
		else
			if(BTN_Obj[n].cnt_dr)
				BTN_Obj[n].cnt_dr = 0;
		
		BTN_Obj[n].block_i = true;
	}
}
//------------------------------------------------------------------------
