#ifndef HAL_BUZZER_H
#define HAL_BUZZER_H

#include "stm32f1xx_hal.h"
#include "hal_types.h"

#define BUZ_PORT_CONTROL  GPIOB
#define BUZ_PIN_CONTROL   GPIO_PIN_9	

#define BUZ_TONE_1_MS  		100																// тональность сигнала для оповещения протечки	
#define BUZ_TONE_2_MS  		500																// тональность сигнала для оповещения потери связи с одним из датчиков
#define BUZ_TIME_ON_MS  	10000

volatile typedef struct 									 
{	
	bool            level_a;																	// флаг - активный уровень (true - высокий; false - низкий)

	bool            fl_mute;																	// флаг - вкл. блокировку сирены
	bool     				fl_on_alarm;															// флаг - вкл. звуковое оповещение	
	u16 				 		cnt_tone;																	// счетчик - тональность звукового оповещения (мс)
	u16     	 			cnt_tone_m;																// макс. значение счетчика cnt_alarm (мс)
	bool     				fl_tm_alarm;															// флаг - вкл. таймер времени работы звукового оповещения
	u16 				 		cnt_tm;																		// счетчик - времени работы звукового оповещения (мс)
	u16     	 			cnt_tm_m;																	// макс. значение счетчика cnt_tm (мс)
	
	u16 					  pin;																			// номер контакта	управления
	GPIO_TypeDef* 	gpio;																			// ссылка на порт контакта управления
	
	GPIO_PinState		(*ReadPin) (GPIO_TypeDef* gpio, u16 pin); // ссылка на метод - прочитать состояние контакта 
	void 						(*WritePin)(GPIO_TypeDef* gpio, u16 pin, 
															GPIO_PinState PinState);			// ссылка на метод - задать состояние контакта
	void 						(*TogglePin)(GPIO_TypeDef* gpio, u16 pin);// ссылка на метод - сменить состояние контакта на противоположное

} BUZ_sObj; 									  														// Cтруктура - параметры объекта управления

/************************************************************************
								Прототипы глобальных переменных модуля
*************************************************************************/
extern BUZ_sObj BUZ_obj;																		// Объект управления пищалкой
//-----------------------------------------------------------------------

/************************************************************************
							 Прототипы глобальных функций модуля
*************************************************************************/
void BUZ_Init(void);																											// Iнициализация модуля модуля 
void BUZ_Handler(void);		 																								// Обработчик модуля (положить в Main)
void BUZ_Handler_Tm(void);																								// Обработчик интервалов для модуля (положить в таймер 1мс)

void BUZ_Set_State_Pin_Control(bool state);																// Задать состояние контакта управления звуковым оповещением
//------------------------------------------------------------------------

#endif
