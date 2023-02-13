#include "hal_algorithm.h"
#include "hal_control_devs.h"
#include "hal_nrf24l01_drv.h"
#include "hal_i2c_1602.h"
#include "hal_buttons.h"
#include "hal_buzzer.h"
#include "hal_my_rtc.h"
#include "hal_my_lib.h"
#include "hal_view_data.h"

/************************************************************************
											Глобальные переменные модуля
*************************************************************************/
ALG_Struct ALG_obj;																												// Объект для работы с параметрами алгоритма
//-----------------------------------------------------------------------

/************************************************************************
										 Прототипы всех функций модуля
*************************************************************************/
void ALG_Init(void);																											// Инициализация модуля модуля 
void ALG_Handler(void);		 																								// Обработчик модуля (положить в Main)
void ALG_Handler_Tm(void);																								// Обработчик интервалов для модуля (положить в таймер 1мс)

void ALG_Handler_Buttons(void);																						// Обработчик нажатия на кнопоки
void ALG_Handler_Sensors(void);																						// Обработчик данных от радиодатчиков
void ALG_Diagnostic_Sensors(void);																				// Диагностика радиодатчиков
void ALG_Keep_Track_Of_Time_Off_Valve(void);															// Следить за временем выкл. клапана по заданному расписанию

//	void NRF24_Callback_Process_BufRx(u8* bf_radio, u8 n_ch); 							// Обработать принятые данные из FIFO RX радиотрансивера
//------------------------------------------------------------------------

/**
	* @brief Инициализация модуля
  */
void ALG_Init(void)
{
	ALG_obj.rf_s_on.full 		 = ALG_RF_S_ON;
	ALG_obj.rf_s_stat.full 	 = NULL;
	ALG_obj.rf_s_rx_ok.full  = NULL;
	ALG_obj.rf_s_diag.full   = NULL;
	ALG_obj.rf_s_diag_s.full = ALG_RF_S_ON;
	
	for(u8 i = 0; i < ALG_MAX_DEV; i++)
		ALG_obj.rf_s_cnt_tm_diag[i] = 0;
	
	ALG_obj.n_off = 1;
	ALG_obj.n_min_off = 1;
	ALG_obj.valve_tm_turn.Hours 	= 0;
	ALG_obj.valve_tm_turn.Minutes = 0;
	ALG_obj.valve_tm_turn.Seconds = 0;
	ALG_obj.fl_state_vl = ALG_VALVE_CLOZE;
}

/**
	* @brief Обработчик модуля (положить в Main)
  */
void ALG_Handler(void)
{
	ALG_Handler_Buttons();
	ALG_Handler_Sensors();
	ALG_Keep_Track_Of_Time_Off_Valve();
}

/**
	* @brief Обработчик интервалов для модуля (положить в таймер 1мс)
  */
void ALG_Handler_Tm(void)
{
	for(u8 i = 0; i < ALG_MAX_DEV; i++)
		if(ALG_obj.rf_s_on.full&(1<<i))
			if(ALG_obj.rf_s_diag_s.full&(1<<i))
			{
				ALG_obj.rf_s_cnt_tm_diag[i]++;
				
				if(ALG_obj.rf_s_cnt_tm_diag[i] == ALG_TM_DIAG)
				{
					ALG_obj.rf_s_cnt_diag[i]++;
					ALG_obj.rf_s_cnt_tm_diag[i] = 0;
					MYLIB_SET_BIT(ALG_obj.rf_s_diag.full, i);
					MYLIB_CLEAR_BIT(ALG_obj.rf_s_diag_s.full, i);
				}
			}
}

/**
	* @brief Обработчик нажатия на кнопоки
  */
void ALG_Handler_Buttons(void)
{
	for(u8 i = 0; i < BTN_OBJ_MAX; i++)
	{
		if(BTN_Obj[i].name == BTN_NAME_ON)
		{
			if(BTN_Obj[i].stat_sav_work2)
			{
				BTN_Obj[i].stat_sav_work1 = false;
				BTN_Obj[i].stat_sav_work2 = false;
				
				if(!LCD1602_Get_State_Led())
				{
					LCD1602_Led_On();	
					break;					
				}	
				if(CDD_Obj[CDD_NAME_OUT_VALVE_CH_1].state == CDD_RSTAT_OFF &&
					 CDD_Obj[CDD_NAME_OUT_VALVE_CH_2].state == CDD_RSTAT_OFF)
				{
					CDD_Set_State(CDD_NAME_OUT_VALVE_CH_1, CDD_RSTAT_IDLE);		
					ALG_obj.fl_state_vl = ALG_VALVE_CLOZE;
				}
				if(!BUZ_obj.fl_on_alarm)
				{
					BUZ_obj.fl_on_alarm = true; // вкл. пищалку
					BUZ_obj.fl_tm_alarm = true; // вкл. откл. пищалки по таймауту
				}				
			}
		}
		else
		if(BTN_Obj[i].name == BTN_NAME_OFF)
		{
			if(BTN_Obj[i].stat_sav_work2)
			{
				BTN_Obj[i].stat_sav_work1 = false;
				BTN_Obj[i].stat_sav_work2 = false;
				
				if(!LCD1602_Get_State_Led())
				{
					LCD1602_Led_On();	
					break;					
				}				
				if(CDD_Obj[CDD_NAME_OUT_VALVE_CH_1].state == CDD_RSTAT_OFF &&
					 CDD_Obj[CDD_NAME_OUT_VALVE_CH_2].state == CDD_RSTAT_OFF)
				{
					CDD_Set_State(CDD_NAME_OUT_VALVE_CH_2, CDD_RSTAT_IDLE);
					ALG_obj.fl_state_vl = ALG_VALVE_OPEN;
				}
				if(!BUZ_obj.fl_on_alarm)
				{
					BUZ_obj.fl_on_alarm = true; // вкл. пищалку
					BUZ_obj.fl_tm_alarm = true; // вкл. откл. пищалки по таймауту		
				}					
			}
		}
		else
		if(BTN_Obj[i].name == BTN_NAME_MUTE)
		{
			if(BTN_Obj[i].stat_sav_work2)
			{
				BTN_Obj[i].stat_sav_work1 = false;
				BTN_Obj[i].stat_sav_work2 = false;
				
				if(!LCD1602_Get_State_Led())
				{
					LCD1602_Led_On();	
					break;					
				}				
				BUZ_obj.fl_mute = BUZ_obj.fl_mute ? false : true;
				
				if(BUZ_obj.fl_mute &&
					 BUZ_obj.fl_on_alarm) 
					BUZ_Set_State_Pin_Control(false);
				
				if(VD_Obj.view_slade < VD_SLADE_USB_CONNECTED)
				{
					VD_Obj.save_slade	= VD_Obj.view_slade;
					VD_Obj.view_slade = VD_SLADE_MUTE;
				}
			}
		}
	}
}	

/**
	* @brief Обработчик данных от радиодатчиков
  */
void ALG_Handler_Sensors(void)
{
	static u16  sensStat = 0;
		
	if(ALG_obj.rf_s_stat.full != sensStat || 
		 ALG_obj.rf_s_rx_ok.full)
	{
		if(CDD_Obj[CDD_NAME_OUT_VALVE_CH_1].state != CDD_RSTAT_OFF ||
			 CDD_Obj[CDD_NAME_OUT_VALVE_CH_2].state != CDD_RSTAT_OFF)
			return;
		
		for(u8 n_ch = 0; n_ch < ALG_MAX_DEV; n_ch++)
			if(ALG_obj.rf_s_on.full&(1<<n_ch))
				if(ALG_obj.rf_s_rx_ok.full&(1<<n_ch))
				{
					ALG_obj.rf_s_cnt_tm_diag[n_ch] = 0;
					MYLIB_SET_BIT(ALG_obj.rf_s_diag_s.full, n_ch);
					MYLIB_CLEAR_BIT(ALG_obj.rf_s_diag.full, n_ch);
					MYLIB_CLEAR_BIT(ALG_obj.rf_s_rx_ok.full, n_ch);
				}		
		if(!ALG_obj.rf_s_stat.full)
		{
			if(ALG_obj.fl_state_vl == ALG_VALVE_OPEN)
			{
				CDD_Set_State(CDD_NAME_OUT_VALVE_CH_1, CDD_RSTAT_IDLE);
				ALG_obj.fl_state_vl = ALG_VALVE_CLOZE;
			}
		}
		else
		{
			if(ALG_obj.fl_state_vl == ALG_VALVE_CLOZE)
			{
				CDD_Set_State(CDD_NAME_OUT_VALVE_CH_2, CDD_RSTAT_IDLE);
				ALG_obj.fl_state_vl = ALG_VALVE_OPEN;
			}
		}
		BUZ_obj.fl_on_alarm = ALG_obj.rf_s_stat.full ? true : false;
		sensStat = ALG_obj.rf_s_stat.full;
	}
	ALG_Diagnostic_Sensors();
}

/**
	* @brief Диагностика радиодатчиков
  */
void ALG_Diagnostic_Sensors(void)
{
	static u16 sensDiag = 0;
	
	if(ALG_obj.rf_s_diag.full != sensDiag) // Диагностика радиодатчиков
	{
//		for(u8 n_ch = 0; n_ch < ALG_MAX_DEV; n_ch++) // Мигание светодиодами
//			if(ALG_obj.rf_s_on.full&(1<<n_ch))
//				if(ALG_obj.rf_s_diag.full&(1<<n_ch))
//					CDD_Set_State(n_ch, CDD_RSTAT_BLINK);	 // для старой версии (управление контактами mcu)
		
		if(ALG_obj.rf_s_diag.full)
		{
			BUZ_obj.fl_on_alarm = true;
			BUZ_obj.cnt_tone_m = BUZ_TONE_2_MS;
		}
		else
		{
			BUZ_obj.fl_on_alarm = false;
			BUZ_obj.cnt_tone_m = BUZ_TONE_1_MS;	
		}
		sensDiag = ALG_obj.rf_s_diag.full;
	}
}

/**
	* @brief Следить за временем выкл. клапана по заданному расписанию
  */
void ALG_Keep_Track_Of_Time_Off_Valve(void)
{
	if(ALG_obj.n_off)
	{
		static RTC_TimeTypeDef tmOffValve;
		static bool flStart = false;
		static u8 cntOff;	
		
		if(!flStart)
			if(RTC_obj.tm_now.Seconds == ALG_obj.valve_tm_turn.Seconds) 
				if(RTC_obj.tm_now.Minutes == ALG_obj.valve_tm_turn.Minutes)
					if(RTC_obj.tm_now.Hours == ALG_obj.valve_tm_turn.Hours)
					{
						CDD_Set_State(CDD_NAME_OUT_VALVE_CH_1, CDD_RSTAT_IDLE); 
						ALG_obj.fl_state_vl = ALG_VALVE_CLOZE;
						tmOffValve = ALG_obj.valve_tm_turn;
						tmOffValve.Minutes += ALG_obj.n_min_off;
						flStart = true;
						cntOff = 0;
					}
		if(flStart)
			if(RTC_obj.tm_now.Seconds == tmOffValve.Seconds) 
				if(RTC_obj.tm_now.Minutes == tmOffValve.Minutes)
					if(RTC_obj.tm_now.Hours == tmOffValve.Hours)
					{
						cntOff++;
						
						if(cntOff <= ALG_obj.n_off) 							
						{
							if(cntOff%2)
							{
								CDD_Set_State(CDD_NAME_OUT_VALVE_CH_2, CDD_RSTAT_IDLE);
								ALG_obj.fl_state_vl = ALG_VALVE_OPEN;
							}
							else
							{
								CDD_Set_State(CDD_NAME_OUT_VALVE_CH_1, CDD_RSTAT_IDLE);
								ALG_obj.fl_state_vl = ALG_VALVE_CLOZE;
							}
							if(tmOffValve.Minutes == 59)
							{
								if(tmOffValve.Hours == 23)
									tmOffValve.Hours = NULL;
								else
									tmOffValve.Hours++;
								
								tmOffValve.Minutes = NULL;
							}
							else
								tmOffValve.Minutes += ALG_obj.n_min_off;
						}
						else
						{
							CDD_Set_State(CDD_NAME_OUT_VALVE_CH_2, CDD_RSTAT_IDLE);
							ALG_obj.fl_state_vl = ALG_VALVE_OPEN;
							flStart = false;
						}
					}		
	}
}

/**
	* @brief 						Обработать принятые данные из FIFO RX радиотрансивера
	* @param  bf_radio: Масссив с принятыми данными
	* @param  		n_ch: Номер канала по которому приняли данные (0-5)
  */
void NRF24_Callback_Process_BufRx(u8* bf_radio, u8 n_ch)
{
	// Формат посылки 32 байта: код->состояние датчика->напряжение батареи
  // "LivnyPribrgH10 State:x Vbat:xxxx"
	const u8	 sCode = 14;												// Размер данных кодовой строки
	const u8	 iStat = 21;												// Индекс начала данных для обработки
	const u8	 iVbat = 28;												// Индекс начала данных строки параметра напряжения батареи
	S_Float 	 bat_v;
	const char strCode[sCode] = "LivnyPribrgH10";	// Кодовая строка
	
	for(u8 i = 0; i < sCode; i++)									// Проверить кодовую строку
		if(strCode[i] != bf_radio[i])
			return;
	
	if(bf_radio[iStat] == '1')
		MYLIB_SET_BIT(ALG_obj.rf_s_stat.full, n_ch);
	else
	if(bf_radio[iStat] == '0')
		MYLIB_CLEAR_BIT(ALG_obj.rf_s_stat.full, n_ch);
	
	MYLIB_SET_BIT(ALG_obj.rf_s_rx_ok.full, n_ch);
	bat_v.byte._3 = bf_radio[iVbat];
	bat_v.byte._2 = bf_radio[iVbat+1];
	bat_v.byte._1 = bf_radio[iVbat+2];
	bat_v.byte._0 = bf_radio[iVbat+3];
	ALG_obj.rf_s_bat_voltage[n_ch] = bat_v.full;
	ALG_obj.rf_s_bat_charge[n_ch] = MyLib_Convert_Val_In_Charge_Persent(ALG_obj.rf_s_bat_voltage[n_ch], ALG_VBAT_L, ALG_VBAT_H);
}
//------------------------------------------------------------------------
