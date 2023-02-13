#include "hal_my_rtc.h"
#include "hal_my_lib.h"

/***************************************************************************
										Объявляем переменные внутри модуля
****************************************************************************/
RTC_sObj RTC_obj;																														// Объект управления RTC
//--------------------------------------------------------------------------

/***************************************************************************
											Прототипы всех функций модуля
****************************************************************************/
void RTC_Init(RTC_HandleTypeDef* _hrtc);																		// Iнициализация модуля RTC
void RTC_Handler(void);		 																									// Обработчик модуля RTC (положить в Main)

bool RTC_Set_Smooth_Calibr(RTC_HandleTypeDef *hrtc, u8 val);								// Задать значение регистра калибровки RTC
void RTC_Correct_Time(void);																								// Корректировать текущее время (калибровка)
void RTC_Keep_Track_Of_Time(void);																					// Следить за временем
void RTC_Set_Up_A_NewTime(void);																						// Установить новое время
//--------------------------------------------------------------------------

/***************************************************************************
												 Описание функций модуля
****************************************************************************/
/**
	* @brief 				 Инициализация модуля RTC
	* @param 	_hrtc: Структура для работы с регитрами модуля RTC
  */
void RTC_Init(RTC_HandleTypeDef* _hrtc)
{
	RTC_obj.hrtc = _hrtc;
	RTC_obj.bkp_octlr_cal = 0;
}

/**
	* @brief Обработчик модуля RTC (положить в Main)
  */
void RTC_Handler(void)
{
	RTC_Keep_Track_Of_Time();
	RTC_Correct_Time();	
	RTC_Set_Up_A_NewTime();
}

/**
	* @brief 				  Задать значение регистра калибровки RTC (для stm32)
	* @param 	 _hrtc: Структура для работы с регитрами модуля RTC
	* @param 	calVal: Значение для перекалибровки (0-127)
  */
bool RTC_Set_Smooth_Calibr(RTC_HandleTypeDef *hrtc, u8 calVal)
{
  if(hrtc == NULL)
    return HAL_ERROR;
	
  __HAL_LOCK(hrtc);
  hrtc->State = HAL_RTC_STATE_BUSY;
  MODIFY_REG(BKP->RTCCR, BKP_RTCCR_CAL, calVal);
  hrtc->State = HAL_RTC_STATE_READY;
  __HAL_UNLOCK(hrtc);

  return HAL_OK;
}

/**
	* @brief 					Задать значение регистра калибровки RTC (для ch32)
	* @param 	 _hrtc: Структура для работы с регитрами модуля RTC
	* @param 	calVal: Значение для перекалибровки (0-31)
  */
void CH32_BKP_Set_RTC_Calibration_Value(RTC_HandleTypeDef *hrtc, u8 calVal)
{
	/* OCTLR register bit mask */
	#define OCTLR_CAL_MASK    	 ((u16)0xFF80)
	#define OCTLR_MASK       		 ((u16)0xFC7F)
	
  u16 tmpReg = 0;
	
  tmpReg  = /*CH32_BKP->OCTLR*/ BKP->RTCCR;
  tmpReg &= /*OCTLR_CAL_MASK*/  OCTLR_CAL_MASK;
  tmpReg |= calVal;
  /*BKP->OCTLR*/ BKP->RTCCR = tmpReg;
	
//	if(hrtc == NULL)
//    return HAL_ERROR;
//	
//  __HAL_LOCK(hrtc);
//  hrtc->State = HAL_RTC_STATE_BUSY;
//  MODIFY_REG(BKP->RTCCR, BKP_RTCCR_CAL, calVal);
//  hrtc->State = HAL_RTC_STATE_READY;
//  __HAL_UNLOCK(hrtc);

//  return HAL_OK;
}

/**
	* @brief Корректировать текущее время (калибровка)
  */
void RTC_Correct_Time(void)
{
	static u8 bkpOctlrCal = 0;
		
	if(RTC_obj.bkp_octlr_cal != bkpOctlrCal)
	{
		RTC_Set_Smooth_Calibr(RTC_obj.hrtc, RTC_obj.bkp_octlr_cal);
//		CH32_BKP_Set_RTC_Calibration_Value(RTC_obj.hrtc, RTC_obj.bkp_octlr_cal);
		bkpOctlrCal = RTC_obj.bkp_octlr_cal;
	}	
}

/**
	* @brief Следить за временем (обновление данных 1 раз в сек)
  */
void RTC_Keep_Track_Of_Time(void)
{
	static RTC_TimeTypeDef nowTime;
	
	HAL_RTC_GetTime(RTC_obj.hrtc, (RTC_TimeTypeDef*)&nowTime, RTC_FORMAT_BIN);
	
	if(RTC_obj.tm_now.Seconds != nowTime.Seconds)
		HAL_RTC_GetTime(RTC_obj.hrtc, (RTC_TimeTypeDef*)&RTC_obj.tm_now, RTC_FORMAT_BIN);
}

/**
	* @brief Установить новое время
  */
void RTC_Set_Up_A_NewTime(void)
{
	static RTC_TimeTypeDef setupTime;

	if(RTC_obj.tm_setup.Seconds != setupTime.Seconds || 
		 RTC_obj.tm_setup.Minutes != setupTime.Minutes ||
	   RTC_obj.tm_setup.Hours 	!= setupTime.Hours)
	{
		HAL_RTC_SetTime(RTC_obj.hrtc, (RTC_TimeTypeDef*)&RTC_obj.tm_setup, RTC_FORMAT_BIN);
		setupTime.Seconds = RTC_obj.tm_setup.Seconds; 
		setupTime.Minutes = RTC_obj.tm_setup.Minutes;
	  setupTime.Hours 	= RTC_obj.tm_setup.Hours;
	}
}
//--------------------------------------------------------------------------
