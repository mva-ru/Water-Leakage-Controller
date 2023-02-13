#include "hal_my_lib.h"
#include "limits.h"
#include "stdlib.h" 
#include "stdio.h"
//#include "string.h"
//#include "time.h" 
//#include <math.h>
//signbit – определение знака числа.
//strftime – перевод даты и времени в текстовую строку заданного формата.
//ceil, ceilf, ceill – округление до наименьшего целого, которое больше или равно аргументу.
//copysign, copysignf, copysignl – копирование знака числа.
//ctime – преобразование времени в текстовую строку.
//ctime_r – преобразование времени в текстовую строку.
//exp, expf, expl – возведение числа в степень.
//isnan, isnanf, isnanl	- проверяют, является ли аргумент не числом (nan).
//modf, modff, modfl	- разделение числа на целую и дробную части.
//atoi	- преобразование строки в число типа int.
//atol	- преобразование строки в число типа long int.
//atof	- преобразование строки в число типа double.
//strtod - преобразование строки в число с плавающей запятой двойной точности

/*********	Пример преобразования STRING->DOUBLE  ***************************************
	char* str =  "+123.45";					// Преобразуемая строка
	volatile double d = 0;					// Переменная для сохранения результата преобразования
	d = strtod (str,&str);						
*****************************************************************************************/

/************************************************************************
									Прототипы всех функций и методов модуля
*************************************************************************/
u8 		 MyLib_Convert_Val_In_Charge_Persent(float val, float valL, float valH); // Преобразовать текущее значение в проценты

char 	 MyLib_Convert_Digit_In_Char(u8 digit);															// Преобразовать цифру в символ
u8     MyLib_Convert_Char_In_Digit(char symbol);													// Преобразовать символ в цифру 
bool	 MyLib_Convert_Buf_u08_In_String(u8 *in, u8 *str, u8 n);						// Преобразовать буфер с данными в строку  
bool	 MyLib_Convert_String_In_Buf_u08(u8 *str, u8 *out, u8 n);						// Преобразовать строку в буфер с данными 

u32 	 MyLib_String_To_U32(char *p_str, u8 size);													// Преобразовать строку в число типа unsigned int
//u16 	 MyLib_String_To_Float(char *p_str, u8 size);												// Преобразовать строку в число типа Float

char   MyLib_TByte_Hex_To_Ascii(u8 tbyte); 																// Преобразовать тетраду байта в символ HEX формата (10 = A)
void   MyLib_Byte_Hex_To_Ascii(u8 byte, char* m_char, u8 n_m); 						// Преобразовать байт в символы HEX формата (10 = 0A)
void   MyLib_MBytes_Hex_To_Ascii(char* m_char, u8* m_hex, u8 size, u8 n_m_char, u8 n_m_hex, bool reverse); // Преобразовать массив байт в строку HEX формата  (10 = 0A)

u8 		 MyLib_Get_Size_Type(Enum_Type_Value type);													// Определить размер типа данных в байтах
u8   	 MyLib_Reverse_Byte(u8 data);																				// Перевернуть байт (зеркально)

u8     MyLib_Dtoa(double d, char* buf, char* sign, u8 n_m, int prcn);												// Преобразование числа с плавающей точкой в строку
u8 		 MyLib_Dtoa_And_Wr_Nulls(double d, char* buf, char* sign, u8 n_m, int prcn, u8 nz_do);// Преобразовать число с плавающей точкой в строку и добавить перед ним нули
double MyLib_Round(double val, u8 prcn);																	// Округление числа с плавающей точкой (тип double), с заданной точностью
double MyLib_Set_Step_Measurement_Value(double val, double step);					// Задать шаг измерения значения

u16 	 MyLib_Identify_Size_Mass(u8* mass);																// Определение длины массива
int 	 MyLib_Find_Byte_In_Mass(u8* mass, u8 byte);												// Найти байт в массиве

u8 		 MyLib_Byte_ToBcd2(u8 val);																					// Converts a 2 digit decimal to BCD format (Взято из модуля rtc)
u8 		 MyLib_Bcd2_ToByte(u8 val);																					// Converts from 2 digit BCD to Binary (Взято из модуля rtc)

double MyLib_Link_Convert_Digit(u8* val_link, Enum_Type_Value val_type);	// Привести ссылку на 1-ый байт к числу с требуемым типом данных
//-----------------------------------------------------------------------

/************************************************************************
										Описание функций и методов модуля
*************************************************************************/
/**
	* @brief 				 Преобразовать текущее значение в проценты
	* @param    val: Текущее значение
	* @param   valL: Нижняя граница значения
	* @param   valH: Верхняя граница значения
	* @retval  			 Возвращает текущее значение переведенное в проценты
									 0xff - ошибка ввода значений в функцию
  */
u8 MyLib_Convert_Val_In_Charge_Persent(float val, float valL, float valH)
{
	if( valL < 	valH || 
		(!valL && !valH))
	{
		float p_1  = (valH - valL)/100.0f;  // шаг значения напряжения для 1%
		
		if(val < valL)
			return 0;
		else
		if(val > valH)	
			return 100;
		
		return (val - valL)/p_1;
	}
	return 0xff;
}

/**
	* @brief 							 Преобразовать цифру в символ
	* @param  [IN]  digit: Цифра для преобразования (0x0X, где X - цифра)
	* @retval	[OUT]  char: Возвращает значение символа
  */
char MyLib_Convert_Digit_In_Char(u8 digit) 
{
	if(digit < 0x0A)
		return 0x30 + digit;
	else
	{
		if(digit == 0x0A)
			return 'A';
		else
		if(digit == 0x0B)
			return 'B';
		else
		if(digit == 0x0C)
			return 'C';
		else
		if(digit == 0x0D)
			return 'D';
		else
		if(digit == 0x0E)
			return 'E';
		else	
		if(digit == 0x0F)
			return 'F';
		else
			return NULL;			
	}
}

/**
	* @brief 							 Преобразовать символ в цифру
	* @param  [IN] symbol: Символ для преобразования
	* @retval	[OUT]    u8: Возвращает значение цифры
  */
u8 MyLib_Convert_Char_In_Digit(char symbol) 
{
	if(symbol>>4 == 3)
		return symbol&0x0f;
	else
	{
		if(symbol == 'A')
			return 0x0A;
		else
		if(symbol == 'B')
			return 0x0B;
		else
		if(symbol == 'C')
			return 0x0C;
		else
		if(symbol == 'D')
			return 0x0D;
		else
		if(symbol == 'E')
			return 0x0E;
		else	
		if(symbol == 'F')
			return 0x0F;
		else
			return NULL;			
	}
}

/**
	* @brief 							Преобразовать буфер с данными в строку 
	* @param	[IN]   *in:	Ссылка на массив - байты данных
	* @param	[OUT] *str:	Ссылка на массив - строка
	* @param	[IN]     n: Кол-во байт данных (для p_in) !!!Только четное!!!
	* @retval	[OUT] bool:	Возвращает флаг - успех операции
  */
bool MyLib_Convert_Buf_u08_In_String(u8 *in, u8 *str, u8 n) 
{
	if(in == NULL || str == NULL || n == NULL)
		return false;
	
	for(u8 i = 0, j = 0; i < n; i++, j+=2)
	{
		str[j] 	 = MyLib_Convert_Digit_In_Char(in[i]>>4);
		str[j+1] = MyLib_Convert_Digit_In_Char(in[i]&0x0f);
	}
	return true;
}

/**
	* @brief 							Преобразовать строку в буфер с данными
	* @param	[IN]  *str:	Ссылка на массив - строку
	* @param	[OUT] *out:	Ссылка на массив - байты данных
	* @param	[IN]     n: Кол-во байт данных (для p_in)
	* @retval	[OUT] bool:	Возвращает флаг - успех операции
  */
bool MyLib_Convert_String_In_Buf_u08(u8 *str, u8 *out, u8 n)
{
	if(out == NULL || str == NULL || n == NULL)
		return false;
	
	for(u8 i = 0, j = 0; i < n; i++, j+=2)
		out[i] = (MyLib_Convert_Char_In_Digit(str[j])<<4) | 
	            MyLib_Convert_Char_In_Digit(str[j+1]);
	
	return true;
}

/**
	* @brief 				  Преобразовать строку в число типа unsigned int
	* @param  *p_str: Ссылка на строку
	* @param  	size: Сколко символов в строке преобразовывать
	* @retval    u16: Возвращает 2 байтовое целое значение
  */
u32 MyLib_String_To_U32(char *p_str, u8 size)
{
	u32 value = 0;

	if(p_str != NULL)
		while(size)
		{
			if(p_str[0] >= 0x30 && p_str[0] <= 0x39)
				value += (u32)((p_str[0] & 0x0F)*pow(10, size-1));

			size--;
			p_str++;
		}
	return value;
}

///**
//	* @brief 				  Преобразовать строку в число типа Float
//	* @param  *p_str: Ссылка на строку
//	* @param  	size: Сколко символов в строке преобразовывать
//	* @retval    u16: Возвращает 2 байтовое целое значение
//  */
//u16 MyLib_String_To_Float(char *p_str, u8 size)
//{
////	atof(p_str);
////	return value;
//}

/**
	* @brief 				Преобразовать символ в число
	* @param     c: Символ, который требуется преобразовать
	* @retval  int: Возвращает значение символа
  */
int MyLib_Char_To_U8(char c)
{
  if(c >= '0' && c <= '9')
    return c - '0';
  else if(c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  else if(c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  else
    return -1;
}

/**
	* @brief 		    Преобразовать тетраду байта в символ HEX формата (10 = A)
	* @param tbyte: Тетрада, которую требуется преобразовать
  */
char MyLib_TByte_Hex_To_Ascii(u8 tbyte)
{
	if(tbyte < 10)
		return '0'+tbyte;
	else
		return 'A'+(tbyte-10);
}

/**
	* @brief 		       Преобразовать байт в символы HEX формата (10 = 0A)
	* @param     byte: Байт, который требуется преобразовать
	* @param  *m_char: Ссылка на массив, куда записать символы
	* @param      n_m: Номер элемента массива, в который записывать символы
  */
void MyLib_Byte_Hex_To_Ascii(u8 byte, char* m_char, u8 n_m)
{
	m_char[n_m]   = MyLib_TByte_Hex_To_Ascii(byte>>4);
	m_char[n_m+1] = MyLib_TByte_Hex_To_Ascii(byte&0x0F);
}

/**
	* @brief 		       Преобразовать массив байт в строку HEX формата  (10 = 0A)
	* @param  *m_char: Ссылка на массив, куда записать символы
	* @param   *m_hex: Ссылка на массив с байтами для преобразования
	* @param     size: Кол-во байт, которое требуется преобразовать
	* @param n_m_char: Номер элемента массива, с которого начинать записывать символы
	* @param  n_m_hex: Номер элемента массива, с которого начинать преобразовать байты в символы
	* @param  reverse: Флаг - пересобрать в обратную сторону
  */
void MyLib_MBytes_Hex_To_Ascii(char* m_char, u8* m_hex, u8 size, u8 n_m_char, u8 n_m_hex, bool reverse)
{
	if(!reverse)
	{
		for(u16 i = 0, j = 0; i < size; i++, j+=2)
			MyLib_Byte_Hex_To_Ascii(m_hex[n_m_hex+i], &m_char[n_m_char], j); 
	}
	else
	{
		for(u16 i = 0, j = 0; i < size; i++, j+=2)
			MyLib_Byte_Hex_To_Ascii(m_hex[n_m_hex-i], &m_char[n_m_char], j); 	
	}
}

/**
	* @brief 				Определить размер типа данных в байтах
	* @param  type: Байт, который требуется преобразовать
	* @retval   u8: Возвращает размер типа данных в байтах
  */
u8 MyLib_Get_Size_Type(Enum_Type_Value type) 
{ 
	switch(type) 																							
  {
		case T_CHAR: 	 		return 1;	
		case T_U8: 		 		return 1;
		case T_S8: 		 		return 1;		
		case T_U16: 	 		return 2;		
		case T_U32: 	 		return 4;		
		case T_U64: 	 		return 8;	
		case T_FLOAT:  		return 4;		
		case T_DOUBLE: 		return 8;
		
		default:   		 		return 0;	
  } 
}

/**
	* @brief 				Перевернуть байт (зеркально)
	* @param  byte: Байт, который требуется преобразовать
	* @retval   u8: Возвращает преобразованный байт
  */
u8 MyLib_Reverse_Byte(u8 byte) 
{ 
	u8 result = 0; 
 
	for(u8 i = 0; i < 8; i++) 
		if(byte & (1 << i)) 
			result |= 1 << (7-i); 

	return result; 
}

///**
//	* @brief        	 Редактирование разрядов, числа с плавающей точкой
//	* @param	   	  d: Число с плавающей точкой (тип double)
//	* @param  	  val: Число, которое надо записатьв разряд
//	* @param  t_digit: Тип разряда (целое\дробное)
//	* @param	   	  n: Номер разряда
//	* @retval        	 Возвращает отредактированное число с плавающей точкой
//  */
//double MyLib_ReToken_Double(vd d, vu8 val, MYLIB_Enum_Type t_digit, vu8 n)
//{	
//	vu64 c_part, d_part;															// целая и дробные части
//	
//	c_part = d;																				// выделяем целую часть
//	d_part = d-c_part;																// выделяем дробную часть												
//	
//	if(t_digit == MYLIB_CEL_DIGIT)
//	{
//		
//	  d = d+(val*(n*10));
//	}
//	else
//	{
//		
//		d = d+(val/(n*10));
//	}
//	return d;
//}

/**
	* @brief        Преобразовать число с плавающей точкой в строку и добавить перед ним нули
	* @param	   d:	Число с плавающей точкой (тип double)
	* @param  *buf: Массив в который записывать строку
	* @param *sign: Куда записать знак
	* @param   n_m: Номер элемента массива с которого начинать записывать преобразованные коды разрядов значения "d"
	* @param  prcn: Кол-во разрядов после запятой (precision). "-1" -> автоматическое определение разрядов по размеру
	* @param nz_do: Кол-во нулей до запятой
	* @retval   u8: Возвращает длину преобразуемого значения (в байтах)
  */
u8 MyLib_Dtoa_And_Wr_Nulls(double d, char* buf, char* sign, u8 n_m, int prcn, u8 nz_do)
{	
	static u8 n_n; n_n = 0;	// на сколько сдвинуть значение (перед ним нули)
	
	if(nz_do > 0)
	{
		if(d < 0)
		{
			if(d > -10)	
				n_n = nz_do - 1;
			else
			if(d > -100)	
				n_n = nz_do - 2;
			else
			if(d > -1000)	
				n_n = nz_do - 3;
			else
			if(d > -10000)	
				n_n = nz_do - 4;
			else
			if(d > -100000)	
				n_n = nz_do - 5;
			else
			if(d > -1000000)	
				n_n = nz_do - 6;
			else
			if(d > -10000000)	
				n_n = nz_do - 7;
		}
		else
		{
			if(d < 10)	
				n_n = nz_do - 1;
			else
			if(d < 100)	
				n_n = nz_do - 2;
			else
			if(d < 1000)	
				n_n = nz_do - 3;
			else
			if(d < 10000)	
				n_n = nz_do - 4;
			else
			if(d < 100000)	
				n_n = nz_do - 5;
			else
			if(d < 1000000)	
				n_n = nz_do - 6;
			else
			if(d < 10000000)	
				n_n = nz_do - 7;
		}
		for(u8 i = n_m; i < n_m + n_n; i++)
			buf[i] = '0';
	}
	return MyLib_Dtoa(d, buf, sign, n_m + n_n, prcn) + n_n;
}

/**
	* @brief        Преобразовать число с плавающей точкой в строку
	* @param	   d:	Число с плавающей точкой (тип double)
	* @param  *buf: Массив в который записывать строку
	* @param *sign: Куда записать знак
	* @param   n_m: Номер элемента массива с которого начинать записывать преобразованные коды разрядов значения "d"
	* @param  prcn: Кол-во разрядов после запятой (precision). "-1" -> автоматическое определение разрядов по размеру
	* @retval   u8: Возвращает длину преобразуемого значения (в байтах)
  */
u8 MyLib_Dtoa(volatile double d, char* buf, char* sign, volatile u8 n_m, volatile int prcn)
{	
	if(buf == NULL)
		return 0;
	
	char* ptr = buf + n_m;
	char* p = ptr;
	char* p1;
	char  c;
	vu64   intPart;																	// целая часть
	vu8    length = 0;																// длина преобразуемого значения (в байтах)
	
	if(d < 0)																				// Определяем знак числа
	{
		length++;
		
		if(sign > 0)
			*sign++ = '-';
		else
			*ptr++ = '-';
	}
	intPart = d;																		// выделяем целую часть
	
	if(d < 0)
		d *= -1;
	
	d -= intPart;																		// выделяем дробную часть (Внимание!!! Сдесь теряеться точность дробной части, поэтому ниже округляем до заданного значения prcn)															
	
	if(!intPart)																		// Проверка целой части на нуль
	{
		length++;
		*ptr++ = '0';
	}
	else
	{
		p = ptr;																			// сохраняем указатель начала

		while(intPart)																// Преобразование целой части (обратный порядок)
		{
			*p++ = '0' + intPart % 10;
			intPart /= 10;
			length++;
		}
		p1 = p;																				// сохраняем конечную позицию

		while(p > ptr)																// Переворачиваем результат
		{
			c = *--p;
			*p = *ptr;
			*ptr++ = c;
		}
		ptr = p1;																			// восстанавливаем конечную позицию
	}
	if(prcn > 0 || prcn < 0)												// Если требуется преобразовать дробную часть
	{
		if(prcn > MYLIB_MAX_PRECISION && prcn != 0xff)// Если заданное значение чисел после запятой превышает максимальное
			prcn = MYLIB_MAX_PRECISION;
		
		if(d > 0 || prcn > 0)													// Если дробная часть не равна нулю
		{
			*ptr++ = '.';																// Ставим разделитель между дробной и целой частью
			length++;
		}
		if(prcn > 0 && prcn != 0xff)									// Если задано фиксированное кол-во цифр после запятой
		{			
			while(d > 0)																// Пока дробная часть больше нуля или ->
			{
				d = MyLib_Round(d, prcn);									// Восстановить потерю точности, после разделения дробной и целой части
				d *= 10.0;
				c = d;
				*ptr++ = '0' + c;
				d -= c;
				length++;
				
				if(prcn > 0)															// -> или отсчитали заданное кол-во цифр после запятой	
				{
					prcn--;																	// Отсчитываем кол-во знаков поле запятой
					
					if(!prcn)
						return length;
				}
			}
			if(prcn > 0)																// Если последние нули после запятой, записываем их
				while(prcn > 0)														
				{
					*ptr++ = '0';
					prcn--;																	// Отсчитываем кол-во знаков поле запятой
					length++;	
					
					if(!prcn)
						return length;
				}
		}
		else																					// Автопреобразование цифр после запятой (если их значения не равны нулю)															
			while(d > 0)														
			{
				d *= 10.0;
				c = d;
				*ptr++ = '0' + c;		
				d -= c;
				length++;
			}
	}
	return length;
}

/**
	* @brief          Округление числа с плавающей точкой (тип double), с заданной точностью
	* @param	   val:	Число с плавающей точкой (тип double), которое необходимо округлить
	* @param    prcn: Кол-во разрядов после запятой (precision) - точность округления
	* @retval double: Возвращает округлённое значение
  */
double MyLib_Round(double val, u8 prcn)
{
   u64 mul = 1;
   prcn++;
	
   for(u8 i = 0; i < prcn; i++)
     mul *= 10;
	
   if(val > 0)
     return floor(val * mul + .5) / mul;
   else
     return ceil(val * mul - .5) / mul;
}

/**
	* @brief 			 		Задать шаг измерения значения
	* @param	   val:	Число с плавающей точкой (тип double), которое необходимо преобразовать
	* @param    step: Шаг преобразования (шаг должен быть целым или дробным и не отрицательным)
	* @retval double: Возвращает значение с заданным шагом
  */	
double MyLib_Set_Step_Measurement_Value(double val, double step)
{
	static u32 		n_steps;																				// кол-во шагов в значении
	static double residue;																				// остаток от целых шагов
	static double border;																					// граница перехода
	static double whole_part, fractional_part; 										// целая и дробная часть "val"

	if(step <= 0)																									// шаг не отрицательное значение
		return val;
	
	fractional_part = modf(step, &whole_part);
	
	if(whole_part > 0 && fractional_part != 0)										// шаг должен быть целым или дробным (например "10" или "0.5")
		return 0;
	
	border = step/2;
	fractional_part = modf(val, &whole_part);
	
	if(step < 1 && step)																									
	{
		n_steps = fractional_part/step;
		residue = fractional_part - n_steps*step;
		
		if(residue < border)
			val = whole_part + n_steps*step;
		else
			val = whole_part + (n_steps + 1)*step;
	}
	else
	{
		n_steps = whole_part/step;
		residue = whole_part - n_steps*step;
		
		if((residue + fractional_part) < border)
			val = n_steps*step;
		else
			val = (n_steps + 1)*step;
	}
	return val;
}

/**
	* @brief         Определение длины массива
	* @param	*mass: Ссылка на массив
	* @retval   u16: Длины массива (в байтах)
  */
u16 MyLib_Identify_Size_Mass(u8* mass)
{
	u16 len = 0U;
	
	while (*mass)
	{
		mass++;
		len++;
	}
	return(len);
}


/**
	* @brief         Найти байт в массиве
	* @param	*mass: Ссылка на массив
	* @retval   u16: Возвращает индекс байта в массиве (-1, если не нашел)
  */
int MyLib_Find_Byte_In_Mass(u8* mass, u8 byte)
{
	u16 len = MyLib_Identify_Size_Mass(mass);
	u16 tmp = 0U;
	
	while(len--)
	{
		if(*mass == byte) 
			return((s16)tmp);

		mass++;
		tmp++;
	}
	return(-1); 																		// значение не найдено
}

/**
  * @brief  		 Converts a 2 digit decimal to BCD format (Взято из модуля rtc)
  * @param  val: Byte to be converted
  * @retval 		 Converted BCD format byte
  */
u8 MyLib_Byte_ToBcd2(u8 val)
{
  volatile static u32 bcd; bcd = 0U;
  
  while(val >= 10U)
  {
    bcd++;
    val -= 10U;
  }
  return ((u8)(bcd << 4U) | val);
}

/**
  * @brief 			 Converts from 2 digit BCD to Binary (Взято из модуля rtc)
  * @param  val: BCD value to be converted
  * @retval 		 Converted word
  */
u8 MyLib_Bcd2_ToByte(u8 val)
{
  static u8 tmp; tmp = 0U;
  tmp = ((u8)(val & (u8)0xF0U) >> (u8)0x4U) * 10U;
  return (tmp + (val & (u8)0x0FU));
}

/**
	* @brief 						 Привести ссылку на 1-ый байт к числу с требуемым типом данных
	* @param	*val_link: Ссылка на конвертируемое значение
	* @param	 val_type: Тип данных, для преобразования
	* @retval	   double: Возвращает приведенное значение
  */
double MyLib_Link_Convert_Digit(u8* val_link, Enum_Type_Value val_type)
{
	double val = 0;
	
	switch((u8)val_type) 																		
	{
		case T_U8:     val = (double) ((u8*)val_link)[0]; 		  break;
		case T_S8:     val = (double) ((s08*)val_link)[0]; 		  break;		
		case T_U16:    val = (double) ((u16*)val_link)[0]; 		  break;		
		case T_U32:    val = (double) ((u32*)val_link)[0];		  break;
		case T_U64:    val = (double) ((u64*)val_link)[0];		  break;
		case T_FLOAT:  val = (double) ((float*)val_link)[0];	  break;
		case T_DOUBLE: val = (double) (((double*)val_link)[0]);	break;
		default: return 0; 
	} 
	return val;
}

//// чтение DCB числа и запись его в  строковом формате
////           откуда           куда     сколько цифр
//void Dcb2str (u08 * rd, char * wr, u08 len){
//u08 i = 0;
//	if (len & 1) i++;
//    while (len){
//		*wr = Get_num_chislo(rd, i) + 0x30;
//        wr++;
//        len--;
//		i++;
//    }
//}


//// Преобразование HEX в строку
//void Hex2str(u08 *_hex, u08 *_str, u08 len){
//u08 ost=0;
//    while (len){
//        ost = *_hex >> 4;
//        if (ost > 9)
//            *_str = ost + 0x37;
//        else *_str = ost + 0x30;
//        _str++;

//        ost = *_hex & 0x0f;
//        if (ost > 9)
//            *_str = ost + 0x37;
//        else *_str = ost + 0x30;
//        _str++;
//        _hex++;
//        len--;
//    }
//}

//-----------------------------------------------------------------------
