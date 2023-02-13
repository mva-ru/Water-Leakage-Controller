#include "hal_i2c_1602.h"

/************************************************************************
											Глобальные переменные модуля
*************************************************************************/
LCD1602_Obj LCD1602;

cu8 LCD1602_init_4bit[8] = 
{
	// Инициализация 4-битного режима 
	// 0b001 DL 0000 (0x20) -> DL выбор шины 4 бита (0) или 8 бит (1)
	
	0x04,	// выставили строб(En = 1) передача 1 тетрады
	0x00,	// сбросили строб (En = 0) передача 1 тетрады
	0x24, // выставили строб(En = 1) передача 2 тетрады
	0x20, // сбросили строб (En = 0) передача 2 тетрады
	
	0x04, // выставили строб(En = 1) передача 1 тетрады
	0x00, // сбросили строб (En = 0) передача 1 тетрады
	0x24, // выставили строб(En = 1) передача 2 тетрады
	0x20, // сбросили строб (En = 0) передача 2 тетрады
	//-------------------------------------
};

cu8 LCD1602_setup_bytes[16] = 
{	
	//Определяем параметры работы дисплея
	0x04, // старшая тетрада(En = 1) - 0b00101000-0x08 -> выключаем дисплей
	0x00, // сбросили строб (En = 0)
	0x84, // младшая тетрада(En = 1) - 0b00101000-0x08 -> выключаем дисплей
	0x80, // сбросили строб (En = 0)
	
	0x24, // старшая тетрада(En = 1) - 0b00101100-0x2C -> разрядность шины 4 бит; кол-во строк 2; шрифт 0-5х7, 1-5х10(+);
	0x20, // сбросили строб (En = 0)
	0xC4, // младшая тетрада(En = 1) - 0b00101100-0x2C -> разрядность шины 4 бит; кол-во строк 2; шрифт 0-5х7, 1-5х10(+);
	0xC0, // сбросили строб (En = 0)
	
	0x04, // старшая тетрада(En = 1) - 0b00000001-0x01 -> очищаем дисплей
	0x00, // сбросили строб (En = 0)
	0x14, // младшая тетрада(En = 1) - 0b00000001-0x01 -> очищаем дисплей
	0x10, // сбросили строб (En = 0)
	
	0x04, // старшая тетрада(En = 1) - 0b00001100-0x0C -> включаем дисплей(1); включение курсора(0); мигание курсора(0);
	0x00, // сбросили строб (En = 0)
	0xC4, // младшая тетрада(En = 1) - 0b00001100-0x0C -> включаем дисплей(1); включение курсора(0); мигание курсора(0);
	0xC0, // сбросили строб (En = 0)
};
//-----------------------------------------------------------------------

/************************************************************************
											Прототипы всех функций модуля
*************************************************************************/
void LCD1602_Scan_I2C(void);             																// Cканируем шину I2C, и определяем ареса устройства(на шине только 1 LCD!!!)
void LCD1602_Init(I2C_HandleTypeDef* hi2c); 														// Инициализация LCD_1602 через шину I2C
void LCD1602_Handler_Tm(void);																					// Обработчик данных модуля в таймере (положить в таймер 1мс)

void LCD1602_Write_Cmd(u8 cmd);																					// Запись команды в дисплей
void LCD1602_Write_Data(u8* data, u8 size);															// Вывод данных на дисплей
void LCD1602_Jump_Cursor(u8 row, u8 col);																// Переместить курсор по адресу
void LCD1602_Jump_Cursor_And_Write_Data(u8 row, u8 col, u8* data, u8 size); // Переместить курсор по адресу и записать данные

void LCD1602_Led_On(void);																							// Включить подсветку дисплея  
void LCD1602_Led_Off(void);																							// Выключить подсветку дисплея  
bool LCD1602_Get_State_Led(void);																				// Получить сост-е подсветки дисплея
//-----------------------------------------------------------------------

//***********************************************************************
//								 		Описание всех функций модуля
//***********************************************************************
/**
	* @brief Сканер подключенных устройств на шине I2C (узнать адрес LCD)  
  */
void LCD1602_Scan_I2C(void) 						       
{
	for(u8 i = 0; i < 127; i++)
    if(HAL_I2C_IsDeviceReady(LCD1602.hi2c, i << 1, 2, LCD1602_I2C_SCAN_MS) == HAL_OK)
		{
			LCD1602.adr = i;
			LCD1602.fl_cn = LCD1602_CONNECT;
			break;
		}
}

/**
	* @brief 		 				 Инициализация LCD_1602 через шину I2C
	* @param	[IN] hi2c: Структура для работы с шиной i2c
  */
void LCD1602_Init(I2C_HandleTypeDef* hi2c)
{
	LCD1602.hi2c 	= hi2c;
	LCD1602.adr 	= LCD1602_I2C_ADRESS_DEF;
	LCD1602.fl_cn = LCD1602_DISCONNECT;
	
	LCD1602.fl_bk = LCD1602_BACKLIGHT_ON;
	LCD1602.cnt_bkl	= NULL;
	
	LCD1602_Scan_I2C();
	HAL_Delay(LCD1602_START_MS);
	
	for(u8 i = 0; i < sizeof(LCD1602_init_4bit); i++)
	{
		HAL_I2C_Master_Transmit(LCD1602.hi2c, LCD1602.adr<<1, (u8*)&LCD1602_init_4bit[i], 1, LCD1602_I2C_TM_MS);
		HAL_Delay(LCD1602_INIT_MS);
	}		
	for(u8 i = 0; i < sizeof(LCD1602_setup_bytes); i++)
	{
		HAL_I2C_Master_Transmit(LCD1602.hi2c, LCD1602.adr<<1, (u8*)&LCD1602_setup_bytes[i], 1, LCD1602_I2C_TM_MS);
		HAL_Delay(LCD1602_SETUP_MS);
	}
}

/**
	* @brief Обработчик данных модуля в таймере (положить в таймер 1мс)
  */
void LCD1602_Handler_Tm(void)
{
	if(LCD1602.fl_bk == LCD1602_BACKLIGHT_ON)
	{
		LCD1602.cnt_bkl++;
		if(LCD1602.cnt_bkl == LCD1602_BACKLIGHT_TM_MS)
		{
			LCD1602.cnt_bkl = NULL;
			LCD1602.fl_bk = LCD1602_BACKLIGHT_OFF;
		}
	}
}

/**
	* @brief 		 				Запись команды в дисплей
	* @param	[IN] cmd: Массив с данными для отправки
  */
void LCD1602_Write_Cmd(u8 cmd) 								
{
	LCD1602.mass[0] = (cmd>>4)<<4 | LCD1602_BYTE_EN1_RW0_RS0;		// старшая тетрада; En = 1
	LCD1602.mass[1] = (cmd>>4)<<4 | LCD1602_BYTE_NULL;					// старшая тетрада; En = 0
	LCD1602.mass[2] = (cmd<<4) 		| LCD1602_BYTE_EN1_RW0_RS0; 	// младшая тетрада; En = 1
	LCD1602.mass[3] = (cmd<<4) 		| LCD1602_BYTE_NULL; 					// младшая тетрада; En = 0
	
	if(LCD1602.fl_bk)
	{
		LCD1602.mass[0] |= LCD1602_BYTE_LED_ON;	
		LCD1602.mass[1] |= LCD1602_BYTE_LED_ON;	
		LCD1602.mass[2] |= LCD1602_BYTE_LED_ON; 
		LCD1602.mass[3] |= LCD1602_BYTE_LED_ON;	
	}
	if(HAL_I2C_Master_Transmit(LCD1602.hi2c, LCD1602.adr<<1, (u8*)LCD1602.mass, 4, LCD1602_I2C_TM_MS) == HAL_OK)
		LCD1602.fl_cn = LCD1602_CONNECT;
	else
		LCD1602.fl_cn = LCD1602_DISCONNECT;
}

/**
	* @brief 		 				  Запись данных в дисплей
	* @param	[IN] *data: Массив с данными для отправки
	* @param	[IN]  size: Размер массива (data)
  */
void LCD1602_Write_Data(u8* data, u8 size)													
{
	for(u8 i = 0, j = 0; j < size; i += 4, j++)
	{
		LCD1602.mass[i]   = (data[j]>>4)<<4 | LCD1602_BYTE_EN1_RW0_RS1;	
		LCD1602.mass[i+1] = (data[j]>>4)<<4 | LCD1602_BYTE_EN0_RW0_RS1;	
		LCD1602.mass[i+2] = (data[j]<<4)    | LCD1602_BYTE_EN1_RW0_RS1; 
		LCD1602.mass[i+3] = (data[j]<<4)    | LCD1602_BYTE_EN0_RW0_RS1;

		if(LCD1602.fl_bk)
		{
			LCD1602.mass[i]   |= LCD1602_BYTE_LED_ON;	
			LCD1602.mass[i+1] |= LCD1602_BYTE_LED_ON;	
			LCD1602.mass[i+2] |= LCD1602_BYTE_LED_ON; 
			LCD1602.mass[i+3] |= LCD1602_BYTE_LED_ON;	
		}
	}
	if(HAL_I2C_Master_Transmit(LCD1602.hi2c, LCD1602.adr<<1, (u8*)LCD1602.mass, size*4, LCD1602_I2C_TM_MS) == HAL_OK)
			LCD1602.fl_cn = LCD1602_CONNECT;
	else
		LCD1602.fl_cn = LCD1602_DISCONNECT;
}

/**
	* @brief 		 				Переместить курсор по адресу
	* @param	[IN] row: Номер строки
	* @param	[IN] col: Номер колонки
  */
void LCD1602_Jump_Cursor(u8 row, u8 col)						 							
{ 
	LCD1602.mass[0] = row | col;
	LCD1602_Write_Cmd(LCD1602.mass[0]);
}

/**
	* @brief 		 				  Переместить курсор по адресу и записать данные
	* @param	[IN]   row: Номер строки
	* @param	[IN]   col: Номер колонки
	* @param	[IN] *data: Массив с данными для отправки
	* @param	[IN]  size: Размер массива (data)
  */
void LCD1602_Jump_Cursor_And_Write_Data(u8 row, u8 col, u8* data, u8 size)
{ 
	LCD1602_Jump_Cursor(row, col);
	LCD1602_Write_Data(data, size);
}

/**
	* @brief Включить подсветку дисплея
  */
void LCD1602_Led_On(void) 
{
//	if(HAL_I2C_Master_Transmit(LCD1602.hi2c, LCD1602.adr<<1, (u8*)LCD1602_BYTE_LED_ON, 1, LCD1602_I2C_TM_MS) == HAL_OK)
//	{
//		LCD1602.fl_cn = LCD1602_CONNECT;
		LCD1602.fl_bk = LCD1602_BACKLIGHT_ON;
//	}
//	else
//		LCD1602.fl_cn = LCD1602_DISCONNECT;
}

/**
	* @brief Выключить подсветку дисплея
  */
void LCD1602_Led_Off(void) 
{
//	if(HAL_I2C_Master_Transmit(LCD1602.hi2c, LCD1602.adr<<1, (u8*)LCD1602_BYTE_NULL, 1, LCD1602_I2C_TM_MS) == HAL_OK)
//	{
//		LCD1602.fl_cn = LCD1602_CONNECT;
		LCD1602.fl_bk = LCD1602_BACKLIGHT_OFF;
//	}
//	else
//		LCD1602.fl_cn = LCD1602_DISCONNECT;
}

/**
	* @brief Получить сост-е подсветки дисплея
  */
bool LCD1602_Get_State_Led(void) 
{
	return LCD1602.fl_bk;
}
//-----------------------------------------------------------------------
