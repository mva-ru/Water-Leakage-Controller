#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include "stm32f1xx_hal.h"
#include "hal_types.h"

/************************************************************************
										Макроопределения для данного модуля
*************************************************************************/
#define CFG_MAP_SIZE 		(u8)5								// Размер карты памяти конфигурации

#define CFG_PK_PORT 		(u8)0 							// Номер порта UART от 0-5 (связь с ПК)

#define CFG_KEY_START 	(u32)0x12345678 		// "Ключ" - начала конфига
#define CFG_KEY_END  		(u32)0x87654321 		// "Ключ" - конца конфига
#define CFG_KEY_SIZE 	  (u8) 4 							// Размер "ключей" (по 4 байта)
//------------------------------------------------------------------------
#define CFG_BUF_SIZE 		(u16)256						// Макс. размер буфера для чтения\записи (Меньше можно, больше нельзя!!!)
//------------------------------------------------------------------------
#define CFG_P_WRITE_MAX (u8) 3							// Макс.  значение счетчика кол-ва попыток записи конф-и в EEPROM
//------------------------------------------------------------------------
#define CFG_P_READ_MAX  (u8) 5							// Макс.  значение счетчика кол-ва попыток чтения конф-и в EEPROM
//------------------------------------------------------------------------
#define CFG_SIZE_ID 		(u8) 2							// Размер ID (в байтах)
#define CFG_MIN_SIZE_DT (u8) 1							// Мин. размер значения параметра (в байтах)
#define CFG_SIZE_CRC 		(u8) 1							// Размер контрольной суммы пакета параметра (в байтах)
#define CFG_SIZE_PRE_PK (u8)(CFG_SIZE_ID + CFG_SIZE_CRC) 									 // Размер пакета параметра конф-и, без рамера значения параметра	
#define CFG_MIN_SIZE_PK (u8)(CFG_SIZE_ID + CFG_MIN_SIZE_DT + CFG_SIZE_CRC) // Мин. размер пакета параметра конф-и	
//------------------------------------------------------------------------

volatile typedef enum												// Тип данных - коды ошибок в работе устр-ва
{											
	CFG_STAT_NONE 			 	 		 		 = 0x00,		// Начальное сост-е при иниц-и																
	CFG_STAT_EPRM_ERR_NOT_FOUND  	 = 0x02,		// Не найдена Eeprom (не подключена или не отвечает)
	CFG_STAT_EPRM_ERR_W_NOPASS		 = 0x03,		// Попытка записи в Eeprom без пароля
	CFG_STAT_EPRM_ERR_TYPE       	 = 0x04,		// Неизвестный тип внешней Eeprom
	CFG_STAT_EPRM_ERR_CRC_SETUP  	 = 0x05,		// Неверная контрольная сумма конф-и из внешней Eeprom
	CFG_STAT_EPRM_ERR_PRM_CRC		 	 = 0x06,		// Ошибка контрольной суммы параметра при чтении
	CFG_STAT_EPRM_ERR_PRM_NO  	 	 = 0x07,		// Такого параметра, нет в карте параметров
	CFG_STAT_EPRM_ERR_W_PRM_NO 		 = 0x08,		// Один из параметров не удалось записать в Eeprom
	CFG_STAT_EPRM_ERR_READ 			 	 = 0x09,		// Ошибка при чтении конф-и из Eeprom		
	CFG_STAT_EPRM_ERR_WRITE 		 	 = 0x0A,		// Ошибка при записи конф-и в Eeprom
	CFG_STAT_EPRM_ERR_W_KEY_START  = 0x0B,		// Ошибка при запросе на запись "ключа" - начало конф-и во внешней Eeprom
	CFG_STAT_EPRM_ERR_W_KEY_END    = 0x0C,		// Ошибка при запросе на запись "ключа" - конца конф-и во внешней Eeprom
	CFG_STAT_EPRM_ERR_W_SIZE			 = 0x0D,		// Ошибка при запросе на запись конф-и(размер конф-и выйдет за предел выделенной области памяти в Eeprom)
	CFG_STAT_EPRM_ERR_R_KEY_START  = 0x0E,		// Отсутствует или неверный "ключ" - начала конф-и во внешней Eeprom (чтение)
	CFG_STAT_EPRM_ERR_R_KEY_END    = 0x0F,		// Отсутствует или неверный "ключ" - конца конф-и во внешней Eeprom (чтение)
	CFG_STAT_EPRM_WARNING_R_SIZE	 = 0x10,		// Если не совпадают размерности конф-и, текущей и в памяти (чтение)
	CFG_STAT_EPRM_ERR_R_CRC_SIZE	 = 0x11,		// Если CRC значения - реальный размер конф-и в EEPROM, не совпадает с расчетным (чтение)
	CFG_STAT_EPRM_ERR_R_NO_SETUP 	 = 0x12,		// В Eeprom нет конф-и устр-ва (чтение)
	CFG_STAT_EPRM_ERR_R_PRM_NO  	 = 0x13,		// Нет такого параметра в карте при чтении конф-и
	CFG_STAT_EPRM_ERR_R_PRM_CRC	 	 = 0x14,		// Если ошибка CRC парамтера конф-и (чтение)
	
	CFG_STAT_EPRM_WRITE_OK  			 = 0x20,		// Конф-я записана успешно
	CFG_STAT_EPRM_READ_OK  		 		 = 0x21,		// Конф-я прочитана успешно
	
	CFG_STAT_HANDLER_STAT_FIL  		 = 0xf0,		// Неизвестный тип состояния PF_Stat_Fill функции PF_Handler
} CFG_Enum_Stat;

volatile typedef struct 										// Структура - счетчики ошибок при чтении\записи конф-и
{
	u8	eeprm_p_write;												// счетчик кол-ва попыток записи конф-и в EEPROM
	u8  eeprm_prm;  													// счетчик прочитанных\записанных параметров конф-и в EEPROM	
	u8  eeprm_prm_no;  												// счетчик неизвестных параметров при чтении из EEPROM
	u8  eeprm_prm_no_wr;  										// счетчик незаписанных параметров в EEPROM
	u8	flash_prm_crc;												// счетчик параметров с ошибочной CRC при чтении из FLASH				
	u8  flash_prm_no;  												// счетчик неизвестных параметров при чтении из FLASH
	u8  flash_prm_no_wr;  										// счетчик незаписанных параметров в FLASH
} CFG_Cnt_Struct;

volatile typedef struct 										// Структура - ячейки карты параметров котроллера
{
	u16						 	 id;											// идентиф-ый номер параметра
	u8							*link_val;								// ссылка на значение параметра				 					
	Enum_Type_Value	 type;										// тип значения параметра (u8, u16, u32 и т.д.) 
	u8							 fl_wr:1;									// флаг - записать параметр в EEPROM (для записи по одному параметру)
	u16							 adr;											// начальный адрес пакета с параметром в EEPROM
} CFG_Map_Cell_Struct;

volatile typedef struct 										// Структура - карта конф-и котроллера
{
	cu16							  adr_start_eeprom;			// Адрес EEPROM - начала конф-и 
	cu16							  adr_end_eeprom;				// Адрес EEPROM - макс-ое место конца конф-и 
	cu16							  size_eeprom;					// Общий размер данных отведенный под конф-ю в EEPROM (в байтах)
	u32							  	size_cfg_eeprom;			// Реальный размер текущей конфигурации в EEPROM (в байтах)
	
	u32							  	adr_now;							// Адрес для записи\чтения параметра конф-и
	u16							  	cnt_size_end;					// Сколько байт осталось до конца чтения\записи конф-и
	CFG_Map_Cell_Struct *cells;								// Ссылка на ячейки карты параметров контроллера
} CFG_Map_Struct;

volatile typedef struct 										// Структура - карта конф-и котроллера
{
	CFG_Enum_Stat 	   stat;									// Статусы - в процессе работы с конф-ей контроллера
	CFG_Cnt_Struct 		 cnts;									// Счетчики ошибок при чтении\записи конф-и
	
	u32                uid_cpu[3];						// Уникальный ID контроллера																			
	
	S_DWord  				 	 software_vr;						// Версия программы
	S_DWord  				 	 hardware_vr;						// Версия "железа" 
	
	char  				 	   software_vr_char;			// Версия программы 
	char  				 	   hardware_vr_char;			// Версия "железа" 
	char  	 					 software_date_char;		// Дата создания программы 
	char  	 					 hardware_date_char;		// Дата создания "железа" 	
	
	S_DWord 				 	 cnt_iwdg;							// Кол-во зависаний контроллера (сработал iwdg)
	S_DWord 				 	 cnt_on;								// Кол-во включений устр-ва (за все время - только по питанию)
	S_DWord 				 	 cnt_save;							// Кол-во перезаписей настроек контроллера во внешнюю EEPROM (за все время, по сбросу регистра 0x0003)
	
	u16 				 	 		 cnt_tm_s;							// Счетчик - задержка перед записью конф-и в EEPROM (мс)
	u16 				 	 	 	 cnt_tm_s_m;						// Макс. значение счетчика 1сек. (мс)
	
	u8 				 	 		 	 cnt_tm_1p_wr;					// Счетчик - задержка перед записью 1го параметра в EEPROM (мс)
	u16 				 	 	 	 cnt_tm_1p_wr_m;				// Макс. значение счетчика (мс)
	bool							 fl_block_1p_wr;				// Вкл задержку между записями по 1-му параметру в EEPROM
	
	u16 							 code_re_init;					// флаг - изменены параметры конф-и (адрес или ID), переинициализировать модули (код 0x30a6)
	u16   	 				 	 code_pre_wr;		 	     	// флаг - изменен параметр конф-и, начать отсчет интервала времени (код 0xf017)
	u16   	 				 	 code_wr;		 	     		  // флаг - записать текущую конф-ю (код 0x710f)
	
	u8	 						   cnt_err;								// Счетчик до 3, если была ошибка на запрос чтения\записи данных
	u8	 					 	 	 cnt_err_m;							// Макс. значение счетчика 
	
	u16							 	 re_cfg_code;						// Код слово - идет режим конф-я контроллера (0x80C4)
	bool						 	 fl_re_cfg_ok;					// Флаг - конф-е закончено, пора записать настройки 
	bool							 fl_re_lpc; 						// Флаг - изменились ссылки на листы (были операции удаления\добавления)
	
	//******************* Для записи по 1 параметру в EEPROM *************** 
	u16 							 cnt_nmap;							// счетчик по массиву - с какого элемента начать
	u16 							 cnt_nmap_m;						// макс. значение (по сколько значений перебирать из карты)
	// чтобы каждый проход цикла не перебирать всю карту, а только ее часть
	//----------------------------------------------------------------------
} CFG_Struct;

/************************************************************************
							 Прототипы глобальных переменных модуля
*************************************************************************/
extern CFG_Struct  		CFG_setup;			 				 														// все переменные для работы с модулем 
extern CFG_Map_Struct CFG_map; 																						// карта параметров контроллера
//----------------------------------------------------------------------

/************************************************************************
								Прототипы глобальных функций модуля
*************************************************************************/
void CFG_Init(void);																											// Инициализация модуля	
void CFG_Handler(void);																										// Обработчик модуля конфигурации (положить в main)																					
void CFG_Handler_Tm(void);																								// Обработчик событий для модуля конфигурации и записи во флеш (положить в таймер 1мс)

void CFG_Read_All_Cfg_And_Check_Stat(void);																// Прочитать всю конф-ю и выставить статус операции

void CFG_Up_Flag_Save_Prm_In_Eeprom(u16 id);															// Взвести флаг - сохранить параметр в EEPROM его по ID			
//------------------------------------------------------------------------

#endif
