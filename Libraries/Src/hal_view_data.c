#include "hal_view_data.h"
#include "hal_i2c_1602.h"
#include "hal_algorithm.h"
#include "usbd_cdc_if.h"
#include "hal_my_adc.h"
#include "hal_my_rtc.h"
#include "hal_my_lib.h"
#include "hal_buzzer.h"

/************************************************************************
											Глобальные переменные модуля
*************************************************************************/
VD_Struct VD_Obj;
//-----------------------------------------------------------------------

/************************************************************************
											Прототипы всех функций модуля
*************************************************************************/
void VD_Init(void);																												// Инициализация модуля
void VD_Handler_View(void);																								// Обработчик отображения данных и событий на дисплее
void VD_Handler_View_Tm(void);																						// Обработчик интервалов для отображения (положить в таймер 1мс)

void VD_Detected_Usb_Connected(void);																			// Обнаружение подключения USB кабеля к контроллеру

void VD_Refresh_Data_On_Slade_Rfid_Devs(void);														// Обновить данные на слайде - напряжение батареи RTC и акумов резервного питания
void VD_Refresh_Data_On_Slade_Vbat_Pbat(void);														// Обновить данные на слайде - уровень заряда батарей и состояние RF датчиков
void VD_Refresh_Data_On_NTime_VTime(void);																// Обновить данные на слайде - текущее время и заданное время для вкл. клапанов от закисания
//-----------------------------------------------------------------------

//***********************************************************************
//								 		Описание всех функций модуля
//***********************************************************************
/**
	* @brief Инициализация модуля
  */
void VD_Init(void)
{
	VD_Obj.cnt_ref 		 = NULL;
	VD_Obj.cnt_slade 	 = NULL;
	VD_Obj.fl_view_ok  = false;
	VD_Obj.fl_view_ref = false;
	VD_Obj.view_slade  = VD_SLADE_HELLO;
	VD_Obj.save_slade  = VD_SLADE_EMPTY;
}

/**
	* @brief Обработчик отображения данных и событий на дисплее
  */
void VD_Handler_View(void)
{
	VD_Detected_Usb_Connected();
	
	if(VD_Obj.view_slade == VD_SLADE_RFID_DEVS)
	{
		if(!VD_Obj.fl_view_ok)
		{
			VD_Obj.fl_view_ref = true;			
			VD_Obj.fl_view_ok = true;
			VD_Obj.cnt_ref = 0;			
		}
		VD_Refresh_Data_On_Slade_Rfid_Devs();	
	}
	else
	if(VD_Obj.view_slade == VD_SLADE_VBAT_PPAT)
	{
		if(!VD_Obj.fl_view_ok)
		{
			VD_Obj.fl_view_ref = true;			
			VD_Obj.fl_view_ok = true;	
			VD_Obj.cnt_ref = 0;			
		}
		VD_Refresh_Data_On_Slade_Vbat_Pbat();		
	}
	else
	if(VD_Obj.view_slade == VD_SLADE_TIME)
	{
		if(!VD_Obj.fl_view_ok)
		{
			VD_Obj.fl_view_ref = true;
			VD_Obj.fl_view_ok = true;
			VD_Obj.cnt_ref = 0;			
		}
		VD_Refresh_Data_On_NTime_VTime();		
	}
	else		
	if(VD_Obj.view_slade == VD_SLADE_MUTE)
	{
		if(!VD_Obj.fl_view_ok)
		{
			LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_1, LCD1602_COL_1, (u8*)"      MUTE      ", 16);
			
			if(BUZ_obj.fl_mute)
				LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_2, LCD1602_COL_1, (u8*)"       ON       ", 16);	
			else
				LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_2, LCD1602_COL_1, (u8*)"       OFF      ", 16);	
			
			VD_Obj.fl_view_ok = true;			
		}
	}
	else		
	if(VD_Obj.view_slade == VD_SLADE_USB_CONNECTED)
	{
		if(!VD_Obj.fl_view_ok)
		{
			LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_1, LCD1602_COL_1, (u8*)"       USB      ", 16);
			LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_2, LCD1602_COL_1, (u8*)"    CONNECTED   ", 16);	
			VD_Obj.fl_view_ok = true;			
		}
	}
	else
	if(VD_Obj.view_slade == VD_SLADE_USB_DISCONNECTED)
	{
		if(!VD_Obj.fl_view_ok)
		{
			LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_1, LCD1602_COL_1, (u8*)"       USB      ", 16);
			LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_2, LCD1602_COL_1, (u8*)"   DISCONNECTED ", 16);	
			VD_Obj.fl_view_ok = true;			
		}		
	}
//	else
//	if(VD_Obj.view_slade == VD_SLADE_USB_DATA)
//	{
//		if(!VD_Obj.fl_view_ok)
//		{
//			LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_1, LCD1602_COL_1, (u8*)"    USB DATA:   ", 16);
//			LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_2, LCD1602_COL_1, (u8*)"                ", 16);	
//			VD_Obj.fl_view_ok = true;			
//		}			
//		else
//		{
//			
//		}
//	}
	else
	if(VD_Obj.view_slade == VD_SLADE_HELLO)
	{
		if(!VD_Obj.fl_view_ok)
		{
			LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_1, LCD1602_COL_1, (u8*)" Water leakage  ", 16);
			LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_2, LCD1602_COL_1, (u8*)"controller v.1.0", 16);	
			VD_Obj.fl_view_ok = true;			
		}
	}
}

/**
	* @brief Обработчик интервалов для отображения (положить в таймер 1мс)
  */
void VD_Handler_View_Tm(void)
{
	if(VD_Obj.fl_view_ok)
	{
		if(VD_Obj.view_slade == VD_SLADE_RFID_DEVS)
		{
			VD_Obj.cnt_slade++;
			
			if(VD_Obj.cnt_slade == VD_SLADE_MS)
			{
				VD_Obj.cnt_slade = 0;
				VD_Obj.view_slade = VD_SLADE_VBAT_PPAT;
				VD_Obj.fl_view_ok = false;
				VD_Obj.fl_view_ref = true;
			}
		}	
		else
		if(VD_Obj.view_slade == VD_SLADE_VBAT_PPAT)
		{
			VD_Obj.cnt_slade++;
			
			if(VD_Obj.cnt_slade == VD_SLADE_MS)
			{
				VD_Obj.cnt_slade = 0;
				VD_Obj.view_slade = VD_SLADE_TIME;
				VD_Obj.fl_view_ok = false;
				VD_Obj.fl_view_ref = true;
			}
		}
		else
		if(VD_Obj.view_slade == VD_SLADE_TIME)
		{
			VD_Obj.cnt_slade++;
			
			if(VD_Obj.cnt_slade == VD_SLADE_MS)
			{
				VD_Obj.cnt_slade = 0;
				VD_Obj.view_slade = VD_SLADE_RFID_DEVS;
				VD_Obj.fl_view_ok = false;
				VD_Obj.fl_view_ref = true;
			}
		}
		else
		if(VD_Obj.view_slade == VD_SLADE_MUTE)
		{
			VD_Obj.cnt_slade++;
			
			if(VD_Obj.cnt_slade == VD_S_USB_MS)
			{
				VD_Obj.cnt_slade = 0;
				VD_Obj.view_slade = VD_Obj.save_slade;
				VD_Obj.fl_view_ok = false;
				VD_Obj.fl_view_ref = true;
			}			
		}
		else
		if(VD_Obj.view_slade == VD_SLADE_USB_CONNECTED ||
			 VD_Obj.view_slade == VD_SLADE_USB_DISCONNECTED)
		{
			VD_Obj.cnt_slade++;
			
			if(VD_Obj.cnt_slade == VD_S_USB_MS)
			{
				VD_Obj.cnt_slade = 0;
				VD_Obj.view_slade = VD_Obj.save_slade;
				VD_Obj.fl_view_ok = false;
				VD_Obj.fl_view_ref = true;
			}
		}
		else
		if(VD_Obj.view_slade == VD_SLADE_HELLO)
		{
			VD_Obj.cnt_slade++;
			
			if(VD_Obj.cnt_slade == VD_HELLO_MS)
			{
				VD_Obj.cnt_slade = 0;
				VD_Obj.view_slade = VD_SLADE_RFID_DEVS;
				VD_Obj.fl_view_ok = false;
			}
		}					
		if(VD_Obj.view_slade == VD_SLADE_RFID_DEVS || 
			 VD_Obj.view_slade == VD_SLADE_VBAT_PPAT ||
			 VD_Obj.view_slade == VD_SLADE_TIME)
		{
			VD_Obj.cnt_ref++;
			
			if(VD_Obj.cnt_ref == VD_TM_REF_MS)
			{
				VD_Obj.cnt_ref = 0;
				VD_Obj.fl_view_ref = true;
			}
		}	
	}
	else
	{
		
	}
}

/**
	* @brief Обнаружение подключения USB кабеля к контроллеру
  */
void VD_Detected_Usb_Connected(void)
{
	static u8 dev_state = USBD_STATE_SUSPENDED;

	if(hUsbDeviceFS.dev_state > USBD_STATE_ADDRESSED)	
		if(dev_state != hUsbDeviceFS.dev_state)
		{
			VD_Obj.fl_view_ok  = false;
			VD_Obj.fl_view_ref = false;
			VD_Obj.cnt_slade 	 = 0;
			
			if(VD_Obj.view_slade < VD_SLADE_USB_CONNECTED)
				VD_Obj.save_slade	= VD_Obj.view_slade;
			
			if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
				VD_Obj.view_slade = VD_SLADE_USB_CONNECTED;
			else
				VD_Obj.view_slade = VD_SLADE_USB_DISCONNECTED;
			
			dev_state = hUsbDeviceFS.dev_state;
		}
}

/**
	* @brief Обновить данные на слайде
					 (уровень заряда батарей и состояний RF датчиков) 
  */
void VD_Refresh_Data_On_Slade_Vbat_Pbat(void)
{
	static char  str1[] = "1*00%2*00%3*00% ";	
	static char  str2[] = "4*00%5*00%6*00% ";
	static char  sign;
	static u8    pos;
	static char *link_str;
	
	if(VD_Obj.fl_view_ref)
	{
		for(u8 bit = 0; bit < 6; bit++)
		{
			if(!MYLIB_READ_BIT(ALG_obj.rf_s_on.full, bit))
				sign = '*';
			else
			{
				if(MYLIB_READ_BIT(ALG_obj.rf_s_diag.full, bit))
					sign = '!';
				else
				{		
					if(MYLIB_READ_BIT(ALG_obj.rf_s_stat.full, bit))
						sign = '+';
					else
						sign = '-';
				}
			}
			if(bit < 3)
			{
				pos = bit*5;
				link_str = &str1[0];
				str1[1 + pos] = sign;	
			}
			else
			{
				pos = (bit-3)*5;
				link_str = &str2[0];
				str2[1 + pos] = sign;
			}
			if(ALG_obj.rf_s_bat_charge[bit] == 100) // !!! не вмещается по 3 разряда цифр на дисплей, только по 2
				MyLib_Dtoa_And_Wr_Nulls(99, link_str, NULL, 2 + pos, 0, 2);
			else
				MyLib_Dtoa_And_Wr_Nulls(ALG_obj.rf_s_bat_charge[bit], link_str, NULL, 2 + pos, 0, 2);
		}
		LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_1, LCD1602_COL_1, (u8*)&str1, 16);
		LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_2, LCD1602_COL_1, (u8*)&str2, 16);
		VD_Obj.fl_view_ref = false;
	}
}

/**
	* @brief Обновить данные на слайде
					 (напряжение батареи RTC и акумов резервного питания) 
  */
void VD_Refresh_Data_On_Slade_Rfid_Devs(void)
{
	static char str1[] = "VBAT:00.00V 000%";	
	static char str2[] = "PBAT:00.00V 000%";
		
	if(VD_Obj.fl_view_ref)
	{
		MyLib_Dtoa_And_Wr_Nulls(ADC_obj.vbat, str1, NULL, 5, 2, 2);
		MyLib_Dtoa_And_Wr_Nulls(ADC_obj.pbat, str2, NULL, 5, 2, 2);
		MyLib_Dtoa_And_Wr_Nulls(ADC_obj.vbat_per, str1, NULL, 12, 0, 3);
		MyLib_Dtoa_And_Wr_Nulls(ADC_obj.pbat_per, str2, NULL, 12, 0, 3);	
		LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_1, LCD1602_COL_1, (u8*)&str1, 16);
		LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_2, LCD1602_COL_1, (u8*)&str2, 16);
		VD_Obj.fl_view_ref = false;
	}
}

/**
	* @brief Обновить данные на слайде
					 (текущее время и заданное время для вкл. клапанов от закисания) 
  */
void VD_Refresh_Data_On_NTime_VTime(void)
{
	static char str1[] = "NwTime:00:00:00 ";	
	static char str2[] = "VlTime:00:00:00 ";	
	
	if(VD_Obj.fl_view_ref)
	{
		MyLib_Dtoa_And_Wr_Nulls(RTC_obj.tm_now.Hours, 		str1, NULL, 7,  0, 2);
		MyLib_Dtoa_And_Wr_Nulls(RTC_obj.tm_now.Minutes, 	str1, NULL, 10, 0, 2);
		MyLib_Dtoa_And_Wr_Nulls(RTC_obj.tm_now.Seconds, 	str1, NULL, 13, 0, 2);
		MyLib_Dtoa_And_Wr_Nulls(ALG_obj.valve_tm_turn.Hours, 	 str2, NULL, 7,  0, 2);
		MyLib_Dtoa_And_Wr_Nulls(ALG_obj.valve_tm_turn.Minutes, str2, NULL, 10, 0, 2);
		MyLib_Dtoa_And_Wr_Nulls(ALG_obj.valve_tm_turn.Seconds, str2, NULL, 13, 0, 2);
		LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_1, LCD1602_COL_1, (u8*)&str1, 16);
		LCD1602_Jump_Cursor_And_Write_Data(LCD1602_ROW_2, LCD1602_COL_1, (u8*)&str2, 16);
		VD_Obj.fl_view_ref = false;
	}
}
//-----------------------------------------------------------------------
