#ifndef HAL_TYPES_H
#define HAL_TYPES_H

#include "stm32f1xx_hal.h"
#include "stdbool.h"											// определяет тип bool
#include "stddef.h"												// определяет макросы NULL и offsetof, а также типы ptrdiff_t, wchar_t и size_t
#include "stdlib.h"												// exit, malloc
#include "math.h"

#define PROGMEM

/*
		float  -> 4 байт ->  7 символов (+- 12345.12)
		double -> 8 байт -> 15 символов (+- 1234567890.12345)
*/

#define _1_BYTE_ true
#define _2_BYTE_ false

#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define s08 int8_t
#define s16 int16_t
#define s32 int32_t
#define s64 int64_t

#define vs08 volatile s08
#define vs16 volatile s16
#define vs32 volatile s32
#define vs64 volatile s64	

#define cu8  const uint8_t
#define cu16 const uint16_t
#define cu32 const uint32_t
#define cu64 const uint64_t

#define vu8  volatile uint8_t
#define vu16 volatile uint16_t
#define vu32 volatile uint32_t
#define vu64 volatile uint64_t
	
#define vcu8  volatile const uint8_t
#define vcu16 volatile const uint16_t
#define vcu32 volatile const uint32_t
#define vcu64 volatile const uint64_t
	
#define vi  volatile int
#define vf  volatile float
#define vd  volatile double
#define vb  volatile bool
	
#define vс  volatile char
#define vсc volatile const char
	
volatile typedef union
{
 struct 
 {
	 u8 _lsb:1;
	 u8 _msb:1;
 } tetr;																		// значения тетрад
 u8 full;																		// значение байта
} S_Byte_Tr;
		
volatile typedef union
{
 struct 
 {
	 u8 _0:1;
	 u8 _1:1;
	 u8 _2:1;
	 u8 _3:1;
	 u8 _4:1;
	 u8 _5:1;
	 u8 _6:1;
	 u8 _7:1;
 } bit;																			// значения битов
 u8 full;																		// значение байта
} S_Byte;

volatile typedef union 
{
	struct 
	{
		u8 _0;																	// мл. байт
		u8 _1;																	// ст. байт
	} byte;
	u16 full;																	// соединенные 2 байта (ст+мл)
} S_Word;

volatile typedef union 
{
	struct 
	{
	 u8 _0:1;
	 u8 _1:1;
	 u8 _2:1;
	 u8 _3:1;
	 u8 _4:1;
	 u8 _5:1;
	 u8 _6:1;
	 u8 _7:1;
	 u8 _8:1;
	 u8 _9:1;
	 u8 _10:1;
	 u8 _11:1;
	 u8 _12:1;
	 u8 _13:1;
	 u8 _14:1;
	 u8 _15:1;
	} bit;
	u16 full;																	// соединенные 16 бит
} S_Word_Bits;

volatile typedef union 
{
	struct 
	{
		u8 _0;																	// мл. байт (крайний правый) 
		u8 _1;																	
		u8 _2;																
		u8 _3;																	// ст. байт (крайний левый)																	
	} byte;
	u32 full;																	// соединенные 4 байта 
} S_DWord;

volatile typedef union 
{
	struct 
	{
	 u8 _0:1;
	 u8 _1:1;
	 u8 _2:1;
	 u8 _3:1;
	 u8 _4:1;
	 u8 _5:1;
	 u8 _6:1;
	 u8 _7:1;
	 u8 _8:1;
	 u8 _9:1;
	 u8 _10:1;
	 u8 _11:1;
	 u8 _12:1;
	 u8 _13:1;
	 u8 _14:1;
	 u8 _15:1;
	 u8 _16:1;
	 u8 _17:1;
	 u8 _18:1;
	 u8 _19:1;
	 u8 _20:1;
	 u8 _21:1;
	 u8 _22:1;
	 u8 _23:1;
	 u8 _24:1;
	 u8 _25:1;
	 u8 _26:1;
	 u8 _27:1;
	 u8 _28:1;
	 u8 _29:1;
	 u8 _30:1;
	 u8 _31:1;
	} bit;
	u32 full;																	// соединенные 32 бита
} S_DWord_Bits;

volatile typedef union 
{
	struct 
	{
		u16 lt;																	// мл. тетрада (крайняя правая) 
		u16 ht;																	// ст. тетрада (крайняя левая) 																																	
	} trd;																	
	u32 full;																	// соединенные 2 тетрады 
} S_DWord_Reg;

volatile typedef union 
{
	struct 
	{
		u8 _0;																	// мл. байт (крайний правый) 
		u8 _1;																	
		u8 _2;																
		u8 _3;																	// ст. байт (крайний левый)																	
	} byte;
	float full;																// соединенные 4 байта 
} S_Float;

volatile typedef union 
{
	struct 
	{
		u8 _0;																	// мл. байт (крайний правый) 
		u8 _1;																	
		u8 _2;																
		u8 _3;																
		u8 _4;																
		u8 _5;																	
		u8 _6;																
		u8 _7;																	// ст. байт (крайний левый)		
	} byte;
	double full;															// соединенные 8 байт
} S_Double;

volatile typedef union 
{
	struct 
	{
		struct 
		{
			u8 _1k;																// тысячные доли (0.001)																
			u8 _100;																
			u8 _10;																	
		} drob;																	// дробная часть (3 разряда)
		struct
		{
			u8 _0;																															
		} dev;																	// разделитель целой и дробной части
		struct 
		{
			u8 _1;																// единицы 
			u8 _10;																	
			u8 _100;															// сотни																
			u8 _1k;																
			u8 _10k;																
			u8 _100k;															// сотни тысяч																	
			u8 _1m;																
			u8 _10m;															// десятки миллионов																		
		} full;																	// целая часть (8 разрядов)	
	} _;																			// для работы с разными частями строки(целой,дробной частью и знаком)	
	u8 cell[12];															// соединенные 12 байт строки
} S_BufChar_12;															

volatile typedef enum												// тип переменной
{
	T_CHAR 			= 0x00,												// символьные
	T_U8 				= 0x01,												// без знаковые													
	T_U16 			= 0x02,												
	T_U32 			= 0x03,
	T_U64 			= 0x04,		
	T_S8 				= 0x05,												// со знаком 													
	T_S16 			= 0x06,												
	T_S32 			= 0x07,
	T_S64 			= 0x08,		
	T_FLOAT 		= 0x09,
	T_DOUBLE 		= 0x0A,
	T_SBYTE			= 0x0B,												// структура - объединение битов в байт
	T_SWORD 		= 0x0C,													
	T_SWORD2 		= 0x0D,
	T_SFLOAT 		= 0x0E,
	T_SDOUBLE 	= 0x0F,
	T_M_U8 			= 0x10,												// массив с элементами типа u8												
	T_M_U16 		= 0x11,												
	T_M_U32 		= 0x12,
	T_M_U64 		= 0x13,		
	T_M_S8 			= 0x14,													  													
	T_M_S16 		= 0x15,												
	T_M_S32 		= 0x16,
	T_M_S64 		= 0x17,		
	T_M_FLOAT 	= 0x18,
	T_M_DOUBLE 	= 0x19,
	
	T_NONE 			= 0xFF,												// не определен
} Enum_Type_Value;

volatile typedef enum												// форматы переменной (для последующего преобразования)
{
	F_HEX 		= 0x00,																								
	F_BCD 		= 0x01,												
	F_DEC 		= 0x02,
	F_CHAR 		= 0x03,
} Enum_Format;

#endif

