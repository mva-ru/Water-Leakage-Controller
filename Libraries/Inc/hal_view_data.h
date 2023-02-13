#ifndef HAL_VIEW_DATA_H
#define HAL_VIEW_DATA_H

#include "stm32f1xx_hal.h"
#include "hal_types.h"

//***********************************************************************
//							Модуль для отображения данных на слайдах
//***********************************************************************
/*  
	1. Отображение данных о состоянии и диагностики RFID датчиков
	2. Отображение данных о напряжениях питания контроллера и резервной батареи
	3. Отображение данных о событиях (подключение\отключение\передача данных USB)
*/
//-----------------------------------------------------------------------

#define VD_HELLO_MS		3000U					// Задержка - приветствие (мс)
#define VD_SLADE_MS		6000U					// Задержка - между слайдами (мс)
#define VD_S_USB_MS		2000U					// Задержка - подключение шнура usb (мс)
#define VD_TM_REF_MS	500U					// Время обновления данных на слайде (мс)

volatile typedef enum
{
	VD_SLADE_EMPTY = 0,								// Пустой экран
	VD_SLADE_HELLO,										// Приветствие
	VD_SLADE_RFID_DEVS,								// Слайд - уровень заряда батарей и состояние RF датчиков 
	VD_SLADE_VBAT_PPAT,								// Слайд - напряжение батареи RTC и акумов резервного питания
	VD_SLADE_TIME,										// Слайд - текущее время и заданное время для вкл. клапанов от закисания
	VD_SLADE_USB_CONNECTED,						// Подключили шнур USB
	VD_SLADE_USB_DISCONNECTED,				// Отключили шнур USB
	VD_SLADE_USB_DATA,								// Идет обмен данными по USB
	VD_SLADE_MUTE,										// Сост-е вкл.\выкл. звукового оповещения
	
} VD_Enum_slade;	

volatile typedef struct 									 
{	
	VD_Enum_slade view_slade;					// какой слайд отображать в текущий момент времени
	VD_Enum_slade save_slade;					// сохранения текущего слайда (view_slade), для последующего отображения после инфо слайда
	bool 					fl_view_ok;					// первичные данные слайда выведены на экран
	bool 					fl_view_ref;				// обновить данные на слайде (взводиться по таймеру 1 раз в сек.)
	u16						cnt_slade;					// счетчик отображения слайда (мс)
	u16						cnt_ref;						// счетчик для обновления данных на слайде (мс)
	
} VD_Struct; 									  		// Cтруктура - параметры объекта управления

/************************************************************************
								Прототипы глобальных переменных модуля
*************************************************************************/
extern VD_Struct VD_Obj;

//-----------------------------------------------------------------------

/************************************************************************
									Прототипы глобальных функций модуля
*************************************************************************/
void VD_Init(void);																												// Инициализация модуля
void VD_Handler_View(void);																								// Обработчик отображения данных и событий на дисплее
void VD_Handler_View_Tm(void);																						// Обработчик интервалов для отображения (положить в таймер 1мс)
//-----------------------------------------------------------------------

#endif
