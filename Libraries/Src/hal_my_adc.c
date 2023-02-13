#include "hal_my_adc.h"
#include "hal_my_lib.h"

ADC_Struct ADC_obj;																												// Структура для работы с модулем

/************************************************************************
									 Прототипы всех функций модуля
*************************************************************************/
void  ADC_Init(ADC_HandleTypeDef* hadc);																	// Инициализация модуля АЦП
void  ADC_Handler(void);																									// Обработчик данных полученных с АЦП (положить в main)
void  ADC_Handler_Tm(void); 																							// Управление перезапросом новых измерений каналов АЦП (положить в таймер 1мс)

void  ADC_Stop_DMA(void);																									// Стоп DMA и сбрасываем необходимые флаги (положить в обработчик прерываний АЦП)
void  ADC_Reset_Params(void);																							// Обнуление флагов и буферов модуля АЦП

u32 	ADC_Filtr_Simple_Midle(u16 val);																		// Фильтр простого среднего

void  ADC_Calc_Vbat(void);																								// Расчет напряжения батареи RTC
void  ADC_Calc_Pbat(void);																								// Расчет напряжения батареи резервного питания
//------------------------------------------------------------------------

//************************************************************************
//									Описание всех функций модуля
//************************************************************************
/**
	* @brief  			Инициализация модуля АЦП
	* @param *hadc: Структура для работы с АЦП
  */
void ADC_Init(ADC_HandleTypeDef* hadc)
{
	ADC_obj.hadc = hadc;
	
	ADC_obj.fl_start = 0;																								
	ADC_obj.fl_save  = 0;																										
	ADC_obj.fl_handl = 0;																										
	ADC_obj.fl_end 	 = 1;																									

	ADC_obj.cnt = 0;																												
	ADC_obj.cnt_max = 1000;																									
	ADC_obj.ncnt = 0;																												
	ADC_obj.ncnt_max = 500;	

	ADC_obj.vbat = 0;	
	ADC_obj.pbat = 0;	
	ADC_obj.vbat_per = 0;	
	ADC_obj.pbat_per = 0;
	ADC_obj.vbat_cal = 0.75;	
	ADC_obj.pbat_cal = -0.10;	
	ADC_obj.vbat_state = ADC_CH_ERR_NOT;	
	ADC_obj.pbat_state = ADC_CH_ERR_NOT;	

	for(u8 ch = 0; ch < ADC_K_CHANLES; ch++)
	{
		ADC_obj.buf_data_e[ch]  = 0;															
		ADC_obj.buf_data_ms[ch] = 0;														
		ADC_obj.buf_data_c[ch]  = 0;				
	}	
	HAL_ADCEx_Calibration_Start(ADC_obj.hadc);
}

/**
	* @brief Управление перезапросом новых измерений каналов АЦП 
					(положить в обработчик таймера с прерыванием в 1 мс)
  */
void ADC_Handler_Tm(void)					
{
	if(ADC_obj.fl_end)																							
	{
		ADC_obj.cnt++;
		if(ADC_obj.cnt == ADC_obj.cnt_max)																		
		{
			ADC_obj.fl_start = 1;																									
			ADC_obj.cnt = 0;																										
			ADC_obj.fl_end = 0;																						
		}
	}
}

/**
	* @brief Обработчик данных, полученных с АЦП (положить в main)
  */
void ADC_Handler(void)
{
	if(ADC_obj.fl_start)																											
	{	
		HAL_ADC_Start_DMA(ADC_obj.hadc, (u32*)ADC_obj.buf_data_e, ADC_K_CHANLES);	
		ADC_obj.fl_start = 0;																											
	}
	if(ADC_obj.fl_save)																								
	{	
		for(u8 ch = 0; ch < ADC_K_CHANLES; ch++)		
			ADC_obj.buf_data_ms[ch] += ADC_obj.buf_data_e[ch];
		
		ADC_obj.ncnt++;
		
		if(ADC_obj.ncnt == ADC_obj.ncnt_max)																	
		{	
			for(u8 ch = 0; ch < ADC_K_CHANLES; ch++)	
				ADC_obj.buf_data_c[ch] = ADC_obj.buf_data_ms[ch]/ADC_obj.ncnt_max;		
											
			ADC_Calc_Vbat();	
			ADC_Calc_Pbat();			
			ADC_Reset_Params();																										
			ADC_obj.fl_end = 1;																						
		}
		else
			ADC_obj.fl_start = 1;																								
		
		ADC_obj.fl_save = 0;																								
	}
}

/**
  * @brief  Regular conversion complete callback in non blocking mode 
  * @param  hadc pointer to a ADC_HandleTypeDef structure that contains
  *         the configuration information for the specified ADC.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(ADC_obj.hadc == hadc)
		ADC_Stop_DMA(); 																											
}

/**
	* @brief  					Выставляем и сбрасываем необходимые флаги 
											(положить в обработчик прерываний DMA)
	* @param  *ADC_dma:	Структура для работы с АЦП по DMA
  */
void ADC_Stop_DMA(void)
{
	if(ADC_obj.hadc->DMA_Handle->State == HAL_DMA_STATE_READY)
	{
		if(HAL_ADC_Stop_DMA(ADC_obj.hadc) == HAL_OK)														
			ADC_obj.fl_save = 1;																									
		else
		{
			ADC_Reset_Params();																										
			ADC_obj.fl_end = 1;																										
		}																																					
	}
	if(ADC_obj.hadc->State == HAL_DMA_STATE_TIMEOUT /*HAL_DMA_STATE_ERROR*/)	
	{
		ADC_Reset_Params();																											
		ADC_obj.fl_end = 1;																											
	}																																				
}

/**
	* @brief Обнуление флагов и буферов модуля АЦП
  */
void ADC_Reset_Params(void)
{
	for(u8 ch = 0; ch < ADC_K_CHANLES; ch++)
		ADC_obj.buf_data_ms[ch] = 0;
	
	ADC_obj.ncnt = 0;	
	ADC_obj.fl_save = 0;																								
	ADC_obj.fl_start = 0;	
}

/**
	* @brief Фильтр простого среднего
  */
u32 ADC_Filtr_Simple_Midle(u16 val)
{	
	static double summ = 0;	
	const  u8 size = 5;
	static u8 cnt  = 0;	
	
	summ += val;
	cnt++;
	
	if(cnt == size)
	{
		val = summ/size;
		summ = 0;	
		cnt = 0;
		return val;
	}
	else
		return 0xFFFFFFFF;
}

/**
	* @brief Расчет напряжения батареи RTC
  */
void ADC_Calc_Vbat(void)
{
	static float vbat;
	static u32 midle;
	
	midle = /*ADC_Filtr_Simple_Midle(*/ADC_obj.buf_data_c[ADC_IDX_VBAT]/*)*/;
	
	if(midle == 0xFFFFFFFF)
	{
		ADC_obj.vbat_state = ADC_CH_ERR_SHORT_CIRCUIT;
		return;
	}
	else
	if(midle == NULL)
	{
		ADC_obj.vbat_state = ADC_CH_ERR_BREAK;
		return;
	}
	else
		ADC_obj.vbat_state = ADC_CH_ERR_NOT;
	
	vbat = (3.3f/4096.0f)*ADC_obj.buf_data_c[ADC_IDX_VBAT];		
  vbat += ADC_obj.vbat_cal;
	vbat = MyLib_Round(vbat, 1);
	
	if(vbat >= 0 && vbat < 5)		
	{		
		ADC_obj.vbat = vbat;	
		ADC_obj.vbat_per = MyLib_Convert_Val_In_Charge_Persent(ADC_obj.vbat, ADC_VBAT_L, ADC_VBAT_H);
	}
	else
		ADC_obj.vbat_state = ADC_CH_ERR_OUT_OF_RANGE;
}

/**
	* @brief Расчет напряжения батареи резервного питания
  */
void ADC_Calc_Pbat(void)
{
	static float pbat = 0;
	static u32 midle = 0;
	
	midle = /*ADC_Filtr_Simple_Midle(*/ADC_obj.buf_data_c[ADC_IDX_PBAT]/*)*/;
	
	if(midle == 0xFFFFFFFF)
	{
		ADC_obj.pbat_state = ADC_CH_ERR_SHORT_CIRCUIT;
		return;
	}
	else
	if(midle == NULL)
	{
		ADC_obj.pbat_state = ADC_CH_ERR_BREAK;
		return;
	}
	else
		ADC_obj.pbat_state = ADC_CH_ERR_NOT;
	
	pbat = (3.3f/4096.0f)*ADC_obj.buf_data_c[ADC_IDX_PBAT];				
	pbat *= 4.12f;																					// коэф-т делителя напряжения
	pbat += ADC_obj.pbat_cal;
	pbat = MyLib_Round(pbat, 1);
	
	if(pbat >= 0 && pbat < 14)		
	{		
		ADC_obj.pbat = pbat;
		ADC_obj.pbat_per = MyLib_Convert_Val_In_Charge_Persent(ADC_obj.pbat, ADC_PBAT_L, ADC_PBAT_H);
	}
	else
		ADC_obj.pbat_state = ADC_CH_ERR_OUT_OF_RANGE;	
}
//------------------------------------------------------------------------
