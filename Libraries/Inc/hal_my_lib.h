#ifndef HAL_MY_LIB_H
#define HAL_MY_LIB_H

#include "stm32f1xx_hal.h"
#include "hal_types.h"

#define MYLIB_MAX_PRECISION	7																							// Сколько разрядов после запятой отдавать (значение по умолчанию)

#define MYLIB_SET_BIT(REG, BIT) 	((REG) |=  (1<<BIT))
#define MYLIB_READ_BIT(REG, BIT) 	((REG) &   (1<<BIT))
#define MYLIB_CLEAR_BIT(REG, BIT) ((REG) &= ~(1<<BIT))

#define MYLIB_LOBYTE_U16(x)  		  ((u8) (x & 0x00FF))											// Выделить младший байт из 2-х байтового значения 
#define MYLIB_HIBYTE_U16(x)  		  ((u8)((x & 0xFF00) >>8))								// Выделить старший байт из 2-х байтового значения

#define MYLIB_MIN(a, b)  				  (((a) < (b)) ? (a) : (b))								// Определить наименьшее значение из двух
#define MYLIB_MAX(a, b)  				  (((a) > (b)) ? (a) : (b))								// Определить наибольшее значение из двух

volatile typedef enum 																										// Статусы работы с внешней EEPROM по шине I2C
{
  MYLIB_CEL_DIGIT = 0x00U,																								// целый разряд
  MYLIB_DRB_DIGIT = 0x01U																									// дробный разряд
} MYLIB_Enum_Type;

volatile typedef enum 																										// До какого знака округлять значение
{
	MYLIB_ROUND_VAL_1_0 = 0,																								// до десятых долей	
	MYLIB_ROUND_VAL_0_1,
	MYLIB_ROUND_VAL_0_01,
	MYLIB_ROUND_VAL_0_001,
	MYLIB_ROUND_VAL_0_0001,
	MYLIB_ROUND_VAL_0_00001,
	MYLIB_ROUND_VAL_0_000001,
} MYLIB_Enum_round;	

/************************************************************************
							 Прототипы глобальных функций модуля
*************************************************************************/
u8 		 MyLib_Convert_Val_In_Charge_Persent(float val, float valL, float valH); // Преобразовать текущее значение в проценты

char 	 MyLib_Convert_Digit_In_Char(u8 digit);															// Преобразовать цифру в символ
u8     MyLib_Convert_Char_In_Digit(char symbol);													// Преобразовать символ в цифру 
bool	 MyLib_Convert_Buf_u08_In_String(u8 *in, u8 *str, u8 n);						// Преобразовать буфер с данными в строку  
bool	 MyLib_Convert_String_In_Buf_u08(u8 *str, u8 *out, u8 n);						// Преобразовать строку в буфер с данными 

u32 	 MyLib_String_To_U32(char *p_str, u8 size);													// Преобразовать строку в число типа unsigned int

char   MyLib_TByte_Hex_To_Ascii(u8 tbyte); 																// Преобразовать тетраду байта в символ HEX формата (10 = A)
void   MyLib_Byte_Hex_To_Ascii(u8 byte, char* m_char, u8 n_m); 						// Преобразовать байт в символы HEX формата (10 = 0A)
void   MyLib_MBytes_Hex_To_Ascii(char* m_char, u8* m_hex, u8 size, u8 n_m_char, u8 n_m_hex, bool reverse); // Преобразовать массив байт в строку HEX формата  (10 = 0A)

u8 		 MyLib_Get_Size_Type(Enum_Type_Value type);													// Определить размер типа данных в байтах
u8   	 MyLib_Reverse_Byte(u8 data);																				// Перевернуть байт (зеркально)

u8     MyLib_Dtoa(double d, char* buf, char* sign, u8 n_m, int prcn);												// Преобразование числа с плавающей точкой в строку
u8 		 MyLib_Dtoa_And_Wr_Nulls(double d, char* buf, char* sign, u8 n_m, int prcn, u8 nz_do);// Преобразовать число с плавающей точкой в строку и добавить перед ним нулиdouble MyLib_Round(double val, u8 prcn);																	// Округление числа с плавающей точкой (тип double), с заданной точностью
double MyLib_Round(double val, u8 prcn);																	// Округление числа с плавающей точкой (тип double), с заданной точностью
double MyLib_Set_Step_Measurement_Value(double val, double step);					// Задать шаг измерения значения

u16 	 MyLib_Identify_Size_Mass(u8* mass);																// Определение длины массива
int 	 MyLib_Find_Byte_In_Mass(u8* mass, u8 byte);												// Найти байт в массиве

u8 		 MyLib_Byte_ToBcd2(u8 val);																					// Converts a 2 digit decimal to BCD format (Взято из модуля rtc)
u8 		 MyLib_Bcd2_ToByte(u8 val);																					// Converts from 2 digit BCD to Binary (Взято из модуля rtc)

double MyLib_Link_Convert_Digit(u8* val_link, Enum_Type_Value val_type);	// Привести ссылку на 1-ый байт к числу с требуемым типом данных
//------------------------------------------------------------------------

#endif
