#ifndef HAL_CONTROL_DEVICES_H
#define HAL_CONTROL_DEVICES_H

#include "stm32f1xx_hal.h"
#include "hal_types.h"

//***************************************************************************
//									Модуль управления обектами
//***************************************************************************
/*  Возможности:
		1. Настройка активного уровня
		2. Вкл\выкл
		3. Вкл\выкл с заданной частотой
*/
//---------------------------------------------------------------------------

#define CDD_OBJ_MAX							2U													// Макс. кол-во объектов управления

#define CDD_VALVE_IDLE_MAX			10000U											// Bремя ожидания вкл.\выкл. клапана (мс.) !!! не более 30 сек.

//#define CDD_PORT_LED_CH1 				GPIOB											// состояние 1-го канала
//#define CDD_PIN_LED_CH1 				GPIO_PIN_3												

//#define CDD_PORT_LED_CH2 				GPIOB															
//#define CDD_PIN_LED_CH2 				GPIO_PIN_4												

//#define CDD_PORT_LED_CH3 				GPIOB																
//#define CDD_PIN_LED_CH3 				GPIO_PIN_5											

//#define CDD_PORT_LED_CH4 				GPIOB												
//#define CDD_PIN_LED_CH4 				GPIO_PIN_6										

//#define CDD_PORT_LED_CH5 				GPIOB															
//#define CDD_PIN_LED_CH5 				GPIO_PIN_7												

//#define CDD_PORT_LED_CH6 				GPIOB															
//#define CDD_PIN_LED_CH6 				GPIO_PIN_8										

//#define CDD_PORT_LED_VALVE 			GPIOB												// состояние клапана															
//#define CDD_PIN_LED_VALVE				GPIO_PIN_9													

#define CDD_PORT_OUT_VALVE_CH1 	GPIOB												// управление клапаном канал 1														
#define CDD_PIN_OUT_VALVE_CH1		GPIO_PIN_10	

#define CDD_PORT_OUT_VALVE_CH2 	GPIOB																							
#define CDD_PIN_OUT_VALVE_CH2		GPIO_PIN_11	

volatile typedef enum
{
//	CDD_NAME_LED_CH_1	= 0x00U,																// светодиод - канал 1
//	CDD_NAME_LED_CH_2,																							
//	CDD_NAME_LED_CH_3,	
//	CDD_NAME_LED_CH_4,
//	CDD_NAME_LED_CH_5,
//	CDD_NAME_LED_CH_6,
//	CDD_NAME_LED_VALVE,
	CDD_NAME_OUT_VALVE_CH_1	= 0x00U,													// управление клапаном канал 1 (закрыть)
	CDD_NAME_OUT_VALVE_CH_2,																	// управление клапаном канал 2 (открыть)
} CDD_eName;																								// Тип данных - назначение

volatile typedef enum
{
	CDD_RSTAT_INIT = 0x00U,
	CDD_RSTAT_ON,																							// вкл
	CDD_RSTAT_OFF,																						// выкл
	CDD_RSTAT_BLINK,																					// вкл\выкл с заданной частотой
	CDD_RSTAT_IDLE,																						// подождать заданное время и сменить сост-е контаткта на противоположное

} CDD_eState;																								// Тип данных - состояние

volatile typedef struct 									 
{	
	CDD_eName				name;																			// назначение клавиш				
	
	bool            level_a;																	// флаг - активный уровень (true - высокий; false - низкий)
	CDD_eState     	state;																		// флаг - текущее состояние
	bool     				state_pin;																// флаг - текущее состояние контакта
	
	u16 				 		cnt_t;																		// счетчик мигания - через сколько сменить состояние объекта управления (мс)
	u16     	 			cnt_t_m;																	// макс. значение счетчика cnt_t (мс)
	
	u16 					  pin;																			// номер контакта	управления
	GPIO_TypeDef* 	gpio;																			// ссылка на порт контакта управления
	GPIO_PinState 	(*ReadPin)  (GPIO_TypeDef* gpio, u16 pin);// ссылка на метод - чтение состояния контакта управления
	void 						(*WritePin) (GPIO_TypeDef* gpio, u16 pin, 
															 GPIO_PinState PinState);			// ссылка на метод - задать состояние контакта управления
	void 						(*TogglePin)(GPIO_TypeDef* gpio, u16 pin);// ссылка на метод - сменить состояние контакта управления на противоположное

} CDD_sObj; 									  														// Cтруктура - параметры объекта управления

/************************************************************************
								Прототипы глобальных переменных модуля
*************************************************************************/
extern CDD_sObj CDD_Obj[CDD_OBJ_MAX];																			// Массив с параметрами для каждого объекта управления
//-----------------------------------------------------------------------

/************************************************************************
									Прототипы глобальных функций модуля
*************************************************************************/
void 	CDD_Init(void);																											// Инициализация модуля
void 	CDD_Handler(void);																									// Обработчик операций (положить main)
void 	CDD_Handler_Tm(void);																								// Обработчик операций в таймере (положить в таймер 1мс)

void  CDD_Set_State(u8 n, CDD_eState state);															// Задать состояние объекта управления
//-----------------------------------------------------------------------

#endif
