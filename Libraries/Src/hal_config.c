#include "hal_mbus_ascii_slave.h"
#include "hal_mbus_ascii.h"
#include "hal_control_devs.h"
#include "hal_config.h"
#include "hal_flash.h"
#include "hal_my_rtc.h"
#include "hal_my_crc.h"
#include "hal_my_lib.h"
#include "hal_my_adc.h"
#include "hal_algorithm.h"

CFG_Struct 							 CFG_setup;			 				 													// Все переменные для работы с модулем 

CFG_Map_Cell_Struct			 CFG_map_cells[CFG_MAP_SIZE] = 										// Ячейки карты настроек контроллера
{															
	{0x002E, ((u8*)&ALG_obj.n_off),  									 T_U8, 0},
	
	{0x0030, ((u8*)&ALG_obj.valve_tm_turn.Hours),   	 T_U8, 0},
	{0x0031, ((u8*)&ALG_obj.valve_tm_turn.Minutes), 	 T_U8, 0},
	
	{0x0040, ((u8*)&ADC_obj.vbat_cal), 							T_FLOAT, 0},
	{0x0042, ((u8*)&ADC_obj.pbat_cal), 							T_FLOAT, 0},
};
									
CFG_Map_Struct CFG_map = {0xFC00, 0xFFFF, (0xFFFF-0xFC00), 0, 0, 0, CFG_map_cells}; // карта параметров контроллера

/************************************************************************
									 Прототипы всех функций и методов модуля
*************************************************************************/	
void CFG_Init(void);																											// Инициализация модуля	
void CFG_Handler(void);																										// Обработчик модуля конфигурации (положить в main)																					
void CFG_Handler_Tm(void);																								// Обработчик событий для модуля конфигурации и записи во флеш (положить в таймер 1мс)

void CFG_Read_All_Cfg_And_Check_Stat(void);																// Прочитать всю конф-ю и выставить статус операции

u8   CFG_Calc_Size_Pack_Parameter(u16 n);																	// Вычислить размер пакета параметра конф-и (в байтах)	
void CFG_Calc_Real_Size(void);																						// Вычислить реальный размер конфигурации (в байтах)
void CFG_Calc_Adr_Prms(void);																							// Рассчитать адреса каждого из параметров краты памяти	

void CFG_Write_While_Eeprom(void);																				// Записать в EEPROM конфигурацию контроллера (не в прерывании)
void CFG_Read_While_Eeprom(void);																					// Чтение конфигурации контроллера из EEPROM (не в прерывании)

void CFG_Up_Flag_Save_Prm_In_Eeprom(u16 id); 															// По ID взвести флаг - сохранить параметр в EEPROM
void CFG_Up_Flag_Save_Group_Prms_In_Eeprom(u16 id, u8 n);									// Взвести группу флагов, начиная с указанного ID - сохранить параметр в EEPROM
//------------------------------------------------------------------------

//************************************************************************
//									Описание всех функций и методов модуля
//************************************************************************

/**
	* @brief Инициализация модуля - конфигурирование устройства 
  */
void CFG_Init(void)																
{	
	CFG_setup.stat = CFG_STAT_NONE;																														
	
	CFG_setup.software_vr.full = 0x00010000;						
	CFG_setup.hardware_vr.full = 0x00020000;						

	CFG_setup.cnts.eeprm_prm 			 = 0;									
	CFG_setup.cnts.eeprm_prm_no 	 = 0;
	CFG_setup.cnts.eeprm_prm_no_wr = 0;
	CFG_setup.cnts.eeprm_p_write	 = 0;
	CFG_setup.cnts.flash_prm_crc   = 0;
	CFG_setup.cnts.flash_prm_no	 	 = 0;
	CFG_setup.cnts.flash_prm_no_wr = 0;
	
	CFG_setup.cnt_iwdg.full  = 0;							
	CFG_setup.cnt_on.full 	 = 0;							
	CFG_setup.cnt_save.full  = 0;											

	CFG_setup.cnt_tm_1p_wr	 = 0;				
	CFG_setup.cnt_tm_1p_wr_m = 1000;		
	CFG_setup.fl_block_1p_wr = false;			

	CFG_setup.code_re_init = 0;					
	CFG_setup.code_pre_wr  = 0;		 	     	
	CFG_setup.code_wr 		 = 0;		 	     		   				

	CFG_setup.re_cfg_code  = 0;					
	CFG_setup.fl_re_cfg_ok = false;			
	CFG_setup.fl_re_lpc		 = false; 
	
	CFG_setup.cnt_tm_s	 = 0;					
	CFG_setup.cnt_tm_s_m = 1000;

	CFG_setup.cnt_err 	 = 0;						
	CFG_setup.cnt_err_m  = 3;
	
	CFG_setup.cnt_nmap 	 = 0;							
	CFG_setup.cnt_nmap_m = 50;	
			
	CFG_Calc_Adr_Prms();																								
	CFG_Calc_Real_Size();																						
	CFG_Read_While_Eeprom();	

	if(CFG_setup.stat != CFG_STAT_EPRM_READ_OK)
	{
		ASCII_Regs_Status_Cpu.bit._1_cfg_err_read = true;
		
		if(CFG_setup.stat != CFG_STAT_EPRM_ERR_R_CRC_SIZE)
			ASCII_Regs_Status_Cpu.bit._2_cfg_err_crc_prm_read = true;
	}
}	

/**
	* @brief Обработчик модуля конфигурации (положить в main)
  */
void CFG_Handler(void)																					
{
	if(CFG_setup.fl_re_cfg_ok)
	{
		CFG_setup.fl_re_cfg_ok = false;																	
		CFG_setup.code_pre_wr  = 0xf017;
		CFG_setup.cnt_tm_s     = 0;
	}
	CFG_Write_While_Eeprom();																																																
} 

/**
	* @brief Обработчик событий для модуля конфигурации и записи во флеш (положить в таймер 1мс)
  */
void CFG_Handler_Tm(void)																					
{
	if(CFG_setup.code_pre_wr == 0xf017)																			
	{
		CFG_setup.cnt_tm_s++;				
		
		if(CFG_setup.cnt_tm_s == CFG_setup.cnt_tm_s_m)
		{
			CFG_setup.cnt_tm_s  	= 0;
			CFG_setup.code_pre_wr = 0;																									
			CFG_setup.code_wr 		= 0x710f;																		
		}
	}
} 

/**
	* @brief Прочитать всю конф-ю и выставить статус операции
  */
void CFG_Read_All_Cfg_And_Check_Stat(void)																					
{ 																							
	CFG_Init();																							
	
	if(CFG_setup.stat != CFG_STAT_EPRM_READ_OK)									
		ASCII_Regs_Status_Cpu.bit._1_cfg_err_read = true;										
} 

/**
	* @brief  		Вычислить размер пакета параметра конф-и (в байтах)
	* @param	 n: Номер элемента ячейки массива с параметрами
  * @retval u8: Возвращает размер пакета параметра конф-и (в байтах)
  */
u8 CFG_Calc_Size_Pack_Parameter(u16 n)																											
{
	return (CFG_SIZE_ID + 
					MyLib_Get_Size_Type(CFG_map_cells[n].type) +										// размер значения параметра 
					CFG_SIZE_CRC);																									// размер CRC пакета параметра
} 

/**
	* @brief  Вычислить реальный размер конфигурации (в байтах)
  */
void CFG_Calc_Real_Size(void)																											
{
	CFG_map.size_cfg_eeprom = 0;
	CFG_map.size_cfg_eeprom += sizeof(CFG_map.size_cfg_eeprom);							// 4 байт размер значения - размер всей конф-и
	CFG_map.size_cfg_eeprom += CFG_SIZE_CRC;																// размер СRC значения - размер всей конф-и
	CFG_map.size_cfg_eeprom += CFG_KEY_SIZE;																// 4 байт "ключ" - начало конф-и
	
	for(u16 i = 0; i < CFG_MAP_SIZE; i++) 																	// Перебираем массив параметров конф-и устр-ва
		CFG_map.size_cfg_eeprom += CFG_Calc_Size_Pack_Parameter(i);						// Вычислить размер пакета параметра конф-и (в байтах)										
	
	CFG_map.size_cfg_eeprom += CFG_KEY_SIZE;																// 4 байт "ключ" - конец конф-и
} 

/**
	* @brief Рассчитать адреса каждого из параметров карты памяти
  */
void CFG_Calc_Adr_Prms(void)																					
{
	u16 adr = CFG_map.adr_start_eeprom + sizeof(CFG_KEY_START) + 
		        sizeof(CFG_map.size_cfg_eeprom) + 1;													// определяем начальный адрес для записи в EEPROM 
	
	for(u16 i = 0; i < CFG_MAP_SIZE; i++)
	{
		CFG_map_cells[i].adr = adr;
		adr += sizeof(CFG_map_cells[i].id) +
					 MyLib_Get_Size_Type(CFG_map_cells[i].type) + CFG_SIZE_CRC;
	}
} 

/**
  * @brief Чтение конфигурации контроллера из EEPROM памяти (не в прерывании - может тормозить всю программу)
  */
void CFG_Read_While_Eeprom(void)																											
{
			u16 offset = 0;																													// смещение, чтобы адрес всегда был чётный (иначе Hardfull err)

			CFG_map.cnt_size_end = 0;																								// сбрасываем значение - сколько осталось до конца конф-и
			CFG_setup.cnts.eeprm_prm = 0;																						// счетчик прочитанных параметров
			CFG_setup.cnts.eeprm_prm_no = 0;																				// счетчик не прочитанных параметров
			CFG_setup.stat = CFG_STAT_EPRM_READ_OK;																	// статус - конф-я прочитана успешно
			CFG_map.adr_now = CFG_map.adr_start_eeprom;															// определяем начальный адрес чтения
			
			u16 szoff = CFG_KEY_SIZE+sizeof(CFG_map.size_cfg_eeprom)+CFG_SIZE_CRC;	// для удобства
			u8  buf[CFG_BUF_SIZE];
		
			Flash_Read_Buf(CFG_map.adr_now, (u8*)&buf, szoff);	 										// Чтение "ключа" - начало конф-и в EEPROM и общий размер конф-и с "ключами"			

				u8  crc_val_cfg_size = 0;																							// CRC значения - реальный размер конф-и в EEPROM
				u32 key_start = 0, key_end = 0, cfg_all_size = 0;											// значения конф-ых параметров из памяти, общий размер кон-ф(в байтах) и общая CRC всей конф-и (с ключами)
				crc_val_cfg_size = Crc_8_Config_2((u8*)&buf[0], sizeof(CFG_map.size_cfg_eeprom), 0);	// Считаем CRC значения - реальный размер конф-и в EEPROM
				
				if(crc_val_cfg_size == buf[4] && crc_val_cfg_size > 0)								// Если CRC значения - реальный размер конф-и в EEPROM, совпадает с расчетным 
				{
					for(u8 m = 0, n = 0; m < szoff; m++, n+=8)
						cfg_all_size |= (buf[m]<<n);																			// формируем значение - реальный размер конф-и в EEPROM (в байтах)
					for(u8 m = 0, n = 0; m < CFG_KEY_SIZE; m++, n+=8)			
						key_start |= (buf[5+m]<<n);																				// формируем значение "ключа" - начало конф-и в EEPROM	

					CFG_map.cnt_size_end = cfg_all_size;																// Запоминаем общий размер кон-ф(в байтах), который прочитали из памяти (отсчет сверху вниз прочитанных байт)
					
					if(key_start > 0)							  																		// Если на месте "ключ" - начало конф-и, есть данные
					{		
						if(key_start == CFG_KEY_START)							  										// Если прочитали верный "ключ" - начало конф-и
						{	
							//*************************************************************
							szoff = CFG_KEY_SIZE;																						// след-ий размер для чтения из памяти (в байтах)
							CFG_map.adr_now += cfg_all_size - szoff;												// определяем следующий адрес чтения
							
							if(CFG_map.adr_now%2)																						// Если адрес не четный
								offset = 1;
							else
								offset = 0;
							
							Flash_Read_Buf(CFG_map.adr_now-offset, (u8*)&buf, szoff);				// Чтение "ключа" - конец конф-и в EEPROM и общая CRC всей конф-и с "кдючами"
							
							for(u8 m = offset, n = 0; m < CFG_KEY_SIZE+offset; m++, n+=8)						
								key_end |= (buf[m]<<n);
							if(key_end != CFG_KEY_END)							  											// Если прочитали не верный "ключ" - конец конф-и во внешней Eeprom
							{
								CFG_setup.stat = CFG_STAT_EPRM_ERR_R_KEY_END;									// отсутствует или неверный "ключ" - конца конф-и во внешней Eeprom
/*выход*/				return;																												// конф-я битая
							}
							//-------------------------------------------------------------						
							if(CFG_map.size_cfg_eeprom != cfg_all_size)											// Проверяем совпадают ли размерности конф-и, текущей и в памяти
								CFG_setup.stat = CFG_STAT_EPRM_WARNING_R_SIZE;								// ничего страшного. яитаем только известные параметры.									
							
							u8  cnt_p_rd = 0;																										 	// счетчик попыток чтения - пакетов конф-и
							u16 buf_size = 0;																										 	// размер буфера 
							CFG_map.cnt_size_end -= sizeof(CFG_map.size_cfg_eeprom)+1+CFG_KEY_SIZE;	// счетчик прочитанных байт из конф-и (начальная часть)
							CFG_map.cnt_size_end -= CFG_KEY_SIZE;			 														 	// счетчик прочитанных байт из конф-и (конечная часть)
							CFG_map.adr_now = CFG_map.adr_start_eeprom + 
																sizeof(CFG_map.size_cfg_eeprom)+1+CFG_KEY_SIZE;			 	// определяем следующий адрес чтения (данные параметров)													
							
							while(CFG_map.cnt_size_end > 0)																	// Читаем данные конф-и из памяти, пока не прочитаем все байты
							{	
								if(CFG_map.cnt_size_end >= CFG_BUF_SIZE)											// Проверяем, сколько нам еще читать данных конф-и из памяти (в байтах)
									buf_size = CFG_BUF_SIZE;																			
								else
									buf_size = CFG_map.cnt_size_end;	
							
								if(cnt_p_rd >= CFG_P_READ_MAX)																// Если было 5 неудачных попыток прочитать пакеты конф-и из памяти
								{
									CFG_setup.stat = CFG_STAT_EPRM_ERR_READ;																					
/*выход*/					return;																											
								}	
								if(CFG_map.adr_now%2)																					// Если адрес не четный
									offset = 1;
								else
									offset = 0;
							
								Flash_Read_Buf(CFG_map.adr_now-offset, (u8*)&buf, buf_size);	// Чтение "ключа" - конец конф-и в EEPROM и общая CRC всей конф-и с "кдючами"
								
								u16 processed_bytes = 0;																			// кол-во обработанных байт (из буфера _em_vars->buf)
								u16 id = 0;
								u8  data_size = 0, pack_size = 0, calc_crc_prm = 0;
								
								for(u16 i = offset; i < buf_size+offset;)											// Обрабатываем прочитанные данные (пакеты с параметрами конф-и)
								{
									if((processed_bytes + CFG_MIN_SIZE_PK) > buf_size)					// Проверяем, не последний ли пакет обрабатываем в буфере
/*выход*/						break;
									
									id = (buf[i+1]<<8) | buf[i]; 																// определить ID параметра																	
									
									for(u16 x = 0; x < CFG_MAP_SIZE; x++)												// По ID найти тип данных параметра
									{
										if(x == CFG_MAP_SIZE)																			// Если параметра с таким ID нет в карте памяти
										{	
										  CFG_setup.stat = CFG_STAT_EPRM_ERR_R_PRM_NO;										
/*выход*/						  return;
										}
										else
										 if(CFG_map.cells[x].id == id)														
										 {
											 data_size = MyLib_Get_Size_Type(CFG_map.cells[x].type);// и по типу узнать размер параметра
											 break;
										 }
									}
									pack_size = CFG_SIZE_PRE_PK + data_size; 										// определить размер пакета параметра
									
									if((processed_bytes + pack_size) > buf_size/*+offset*/)			// Проверяем, целый ли пакет в буфере (в конце может оказаться, что в буфере пол пакета с параметром) 
/*выход*/						break;
																		
									calc_crc_prm = Crc_8_Config_2((u8*)&buf[0], pack_size, i); 	// Расчетная CRC параметра
									processed_bytes += pack_size;																// считаем, сколько до конца буфера с данными
									
									for(u16 j = 0; j < CFG_MAP_SIZE; j++) 											// Перебираем карту параметров, и находим необходимый параметр по ID
									{
										if(CFG_map.cells[j].id == id)															// Если нашли	необходимый параметр																 
										{
											if(calc_crc_prm == buf[i+pack_size-1])									// Проверяем контрольную сумму параметра
											{
												for(u8 m = 0; m < data_size; m++) 				
													CFG_map.cells[j].link_val[m] = buf[i+CFG_SIZE_ID+m];// собираем значение параметра в единое целое	
												
												CFG_setup.cnts.eeprm_prm++;														// увел-ем счетчик прочитанных параметров из EEPROM
											}
											else
											{
											  ASCII_Regs_Status_Cpu.bit._2_cfg_err_crc_prm_read = true;
												CFG_setup.stat = CFG_STAT_EPRM_ERR_R_PRM_CRC;					// Если ошибка CRC параметра конф-и (чтение)
												return;
											}
/*выход*/							break;																				
										}
										if(j == CFG_MAP_SIZE)																			// Проверили все ID и ненашлось такого параметра
										{
											CFG_setup.stat = CFG_STAT_EPRM_ERR_PRM_NO;							// такого параметра, нет в карте параметров
											CFG_setup.cnts.eeprm_prm_no++;													// увеличил счетчик параметров, которых нет в карте параметров
										}
									}
									i += pack_size;																							// определяем след-й элемент массива - начало след-го параметра
								}/* for(u16 i = 0; i < CFG_READ_SIZE;)	*/
								CFG_map.cnt_size_end -= processed_bytes;											// считаем, сколько до конца конф-и (когда прочитаем последний параметр)
								CFG_map.adr_now += processed_bytes;														// адрес с которого прочитать след-ю часть конф-и 
							}/* while(CFG_map.cnt_size < cfg_all_size) */
						}/* if(key_start == CFG_KEY_START) */
						else
							CFG_setup.stat = CFG_STAT_EPRM_ERR_R_KEY_START;									// Данные "Ключа" - начало конф-и не совпадают. Возможно конф-я испорчена.
					}/*	if(key_start > 0)	*/
					else
						CFG_setup.stat = CFG_STAT_EPRM_ERR_R_NO_SETUP;										// Данные "Ключа" - начало конф-и равны нулю. Или нет конф-и в памяти или она повреждена.
				}/* if(crc_cfg_all_size == _em_vars->buf[4]) */
				else
					CFG_setup.stat = CFG_STAT_EPRM_ERR_R_CRC_SIZE;											// Если CRC значения - реальный размер конф-и в EEPROM, не совпадает с расчетным 
} 

/**
	* @brief Записать в EEPROM конфигурацию контроллера(не в прерывании - может тормозить всю программу)
  */
void CFG_Write_While_Eeprom(void)			
{
	if(CFG_setup.code_wr == 0x710F)																						// Если записан код - сохранить текущую конф-ю
	{
			if(CFG_map.size_cfg_eeprom >= 
				(CFG_map.adr_end_eeprom - CFG_map.adr_start_eeprom))								// Проверка - не выходит ли конф-я за выделенную область в EEPROM
			{
				CFG_setup.stat = CFG_STAT_EPRM_ERR_W_SIZE;													// статус - размер конф-и выйдет за предел выделенной области памяти в Eeprom
				ASCII_Regs_Status_Cpu.bit._0_cfg_err_write = true;
				CFG_setup.code_wr = 0;	
				return;
			}
			CFG_setup.cnts.eeprm_prm_no_wr = 0;																		// счетчик незаписанных параметров в EEPROM
			CFG_setup.stat = CFG_STAT_EPRM_WRITE_OK;															// статус - конф-я записана успешно в EEPROM
			CFG_map.adr_now = CFG_map.adr_start_eeprom;														// определяем начальный адрес для записи в EEPROM
			//******************************************************************* 
			u16 szoff = sizeof(CFG_map.size_cfg_eeprom);													// для удобства
			u8  buf[CFG_BUF_SIZE];
			
			for(u16 m = 0, n = 0; m < szoff; m++, n+=8) 		
				buf[m] = (u8)(CFG_map.size_cfg_eeprom>>n);													// формируем часть пакета. Размер конф-и в EEPROM
			
			buf[szoff] = Crc_8_Config_2((u8*)&buf[0], szoff, 0); 									// Расчет CRC, для достоверности значения - реальный размер конф-и
			
			for(u16 m = 0, n = 0; m < CFG_KEY_SIZE; m++, n+=8) 											
				buf[szoff+1+m] = (u8)(CFG_KEY_START>>n); 														// формируем часть пакета. "ключ" - начало конф-и в EEPROM	
			//-------------------------------------------------------------------
			szoff += CFG_KEY_SIZE + CFG_SIZE_CRC;
			
			bool fl_inc = false;
			u8   size_data = 0; 																									// размер данных в пакете
			u16  pack_size = 0;																										// размер пакета для записи в EEPROM
				
			for(u16 j = 0; j < CFG_MAP_SIZE;) 																		// Перебираем карту параметров
			{
				size_data = MyLib_Get_Size_Type(CFG_map_cells[j].type);
				pack_size = CFG_Calc_Size_Pack_Parameter(j);												// вычислить размер пакета параметра конф-и (в байтах)
				
				if((szoff+pack_size) < CFG_BUF_SIZE)																// Проверяем не выходит ли текущий пакет за пределы буфера
				{
					buf[0+szoff] = CFG_map.cells[j].id;
					buf[1+szoff] = CFG_map.cells[j].id>>8;
					
					for(u8 m = 0; m < size_data; m++) 																// Формируем часть пакета - данные параметра
						buf[2+szoff+m] = CFG_map.cells[j].link_val[m];									// от младшего байта к старшему 
					
					buf[2+size_data+szoff] = Crc_8_Config_2((u8*)&buf[0], pack_size, szoff); // CRC пакета
					szoff += pack_size;
					fl_inc = true;
				}
				if((szoff+pack_size) >= CFG_BUF_SIZE || 														// Если буфер - полный (готов к записи в EEPROM) 
					 (j+1) == CFG_MAP_SIZE)																						// или записали последний параметр из карты
				{
					if((j+1) == CFG_MAP_SIZE)																					// Записали последний параметр из карты
						break;
					//!!! Здесь косяк !!! Если еще не все параметры записали, а буфер полон.
					// продолжить запись за пределы массива и сюда, больше не зайдет
				}
				if(fl_inc)
				{
					j++;
					fl_inc = false;
				}
			}
			for(u8 m = 0, n = 0; m < CFG_KEY_SIZE; m++, n+=8)												// Оба значения в цикле по 4 байта и не будут меняться (поэтому CFG_KEY_SIZE)
				buf[szoff+m] = (u8)(CFG_KEY_END>>n);																	// формируем часть пакета. "Ключ" - конец конфига в EEPROM															
			
			szoff += CFG_KEY_SIZE;
			Flash_Clear_Sector(CFG_map.adr_start_eeprom);
			Flash_Write_Buf(CFG_map.adr_start_eeprom, (u8*)&buf, szoff);						// Запись данных из масива

			if(CFG_setup.stat != CFG_STAT_EPRM_WRITE_OK)														// Проверяем статус - записи конф-и в EEPROM
			{
				CFG_setup.cnts.eeprm_p_write++;																				// счетчик попыток записи конф-и в EEPROM
				
				if(CFG_setup.cnts.eeprm_p_write >= CFG_P_WRITE_MAX)										// Если счетчик превысит макс. число попыток записи конф-и в EEPROM
				{
					CFG_setup.code_wr = 0;																							// сбросим флаг - сохранить текущую конф-ю в EEPROM
					CFG_setup.cnts.eeprm_p_write = 0;																		// сбросим счетчик попыток записи конф-и в EEPROM
					ASCII_Regs_Status_Cpu.bit._0_cfg_err_write = true;									// выставим статус бит регистра - ошибка записи конф-и в EEPROM			
				}
			}
			else
			{
				CFG_setup.code_wr = 0;
				CFG_setup.cnts.eeprm_p_write = 0;																			// сбросим счетчик попыток записи конф-и в EEPROM
				ASCII_Regs_Status_Cpu.bit._0_cfg_err_write = false;										// сбросим статус бит регистра - ошибка записи конф-и в EEPROM
				CFG_setup.cnt_save.full++;																						// кол-во перезаписей настроек внешней EEPROM (за все время, по сбросу регистра 0x0003)
			}
	} /* if(CFG_Fl_SWC == 0x710f) 												*/
} 

/**
	* @brief 			По ID взвести флаг - сохранить параметр в EEPROM
	* @param  id: Номер параметра
  */
void CFG_Up_Flag_Save_Prm_In_Eeprom(u16 id)			
{
	for(u16 i = 0; i < CFG_MAP_SIZE; i++)
		if(CFG_map_cells[i].id == id)
			CFG_map_cells[i].fl_wr = true;
} 

/**
  * @brief 			Взвести группу флагов, начиная с указанного ID - сохранить параметр в EEPROM
	* @param  id: Номер параметра
	* @param   n: кол-во параметров из конф-ой карты
  */
void CFG_Up_Flag_Save_Group_Prms_In_Eeprom(u16 id, u8 n)			
{
	for(u16 i = 0; i < CFG_MAP_SIZE; i++)
		if(CFG_map_cells[i].id == id)
			CFG_map_cells[i].fl_wr = true;
} 
//------------------------------------------------------------------------
