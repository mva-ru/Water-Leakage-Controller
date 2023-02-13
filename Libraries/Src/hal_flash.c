#include "hal_flash.h"

/************************************************************************
									 Прототипы всех функций и методов модуля
*************************************************************************/
u16  Flash_Read_16(u32 adress); 							  			 	 // прочитать 2 байта флеш памяти (адрес ячейки)
u32  Flash_Read_32(u32 adress); 							  			 	 // прочитать 4 байта флеш памяти (адрес ячейки)
void Flash_Read_Buf(u32 adr_r, u8* link_buf, u16 size);	 // Чтение данных из массива
	
void Flash_Write_16(u32 adress, u16 data);  				 		 // запись во флеш данные 2 байта по указанному адресу
void Flash_Write_32(u32 adress, u32 data);					 		 // запись во флеш данные 4 байта по указанному адресу
void Flash_Write_Buf(u32 adr_w, u8* link_buf, u16 size); // Запись данных из масива

void Flash_Clear_Sector(u32 sector);                   	 // стереть сектор флеш в соответствии с таблицей

void Flash_Jump_Adress(u32 adress);											 // перейти по адресу в область другой программы
//------------------------------------------------------------------------

//************************************************************************
//									Описание	всех функций и методов модуля
//************************************************************************
/**
	* @brief         		Запись данных из массива
	* @param		 adr_w: Начальный адрес для записи		
	* @param	link_buf: Ссылка на массив	
	* @param	    size: Размер массива в байтах
  */
void Flash_Write_Buf(u32 adr_w, u8* link_buf, u16 size)
{
	HAL_FLASH_Unlock();
	
	u16 data;
	
	for(u16 i = 0; i < size; i+=2)
	{
		data = link_buf[i]<<8 | link_buf[i+1];
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_S_ADR_FLASH+adr_w+i, data);
	}
	HAL_FLASH_Lock();
}

/**
	* @brief         		Чтение данных из массива
	* @param		 adr_r: Начальный адрес для чтения		
	* @param	link_buf: Ссылка на массив	
	* @param	    size: Размер массива в байтах
  */
void Flash_Read_Buf(u32 adr_r, u8* link_buf, u16 size)
{
	u16 data;
	adr_r += FLASH_S_ADR_FLASH;
	
	for(u16 i = 0, j = 0, s = (size/2)+1; i < s; i++, j+=2)
	{
		data = *(__IO uint16_t *)adr_r;
		link_buf[j] 	= data>>8;
		link_buf[j+1] = data;
		adr_r += 2;
	}
}

/**
	* @brief           Функция чтения двух байт из флеш памяти контроллера
	* @param	 adress: Адрес ячейки флеш памяти, которую требуется прочитать	
  * @retval     u16: Возвращает прочитанное 2 байтное значение
  */
u16 Flash_Read_16(u32 adress)
{
	return *(u16*) adress;
}

/**
	* @brief           Функция чтения четырех байт из флеш памяти контроллера
	* @param	 adress: Адрес ячейки флеш памяти, которую требуется прочитать	
  * @retval     u16: Возвращает прочитанное 4 байтное значение
  */
u32 Flash_Read_32(u32 adress)
{
	return *(uint32_t*) adress;
}

/**
	* @brief           Метод записи двух байт данных во флеш память контроллера
	* @param	 adress: Адрес ячейки флеш памяти, куда требуется записать данные	
	* @param	   data: Данные, которые требуется записать во флеш память
  */
void Flash_Write_16(u32 adress, u16 data)
{
	HAL_FLASH_Unlock();
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, adress, data);
	HAL_FLASH_Lock();
}

/**
	* @brief           Метод записи четырех байт данных во флеш память контроллера
	* @param	 adress: Адрес ячейки флеш памяти, куда требуется записать данные	
	* @param	   data: Данные, которые требуется записать во флеш память
  */
void Flash_Write_32(u32 adress, u32 data)
{
	HAL_FLASH_Unlock();
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, adress, 	data);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, adress+2, data>>16);
	HAL_FLASH_Lock();
}

/**
	* @brief           Метод записи двух байт данных во флеш память контроллера
	* @param	 adress: Адрес ячейки флеш памяти, которую требуется стереть	
  */
void Flash_Clear_Sector(uint32_t sector)
{
	/* (1) Set the PER bit in the FLASH_CR register to enable page erasing */
  /* (2) Program the FLASH_AR register to select a page to erase */
  /* (3) Set the STRT bit in the FLASH_CR register to start the erasing */
  /* (4) Wait until the BSY bit is reset in the FLASH_SR register */
  /* (5) Check the EOP flag in the FLASH_SR register */
  /* (6) Clear EOP flag by software by writing EOP at 1 */
  /* (7) Reset the PER Bit to disable the page erase */
	
//  FLASH->CR |= FLASH_CR_PER; 										/* (1) */
//  FLASH->AR =  FLASH_S_ADR_FLASH+sector; 				/* (2) */
//  FLASH->CR |= FLASH_CR_STRT; 									/* (3) */
//  
//	while ((FLASH->SR & FLASH_SR_BSY) != 0)  			/* (4) */
//  {
//    /* For robust implementation, add here time-out management */
//  }
//  if ((FLASH->SR & FLASH_SR_EOP) != 0)  				/* (5) */
//  {
//    FLASH->SR |= FLASH_SR_EOP; 									/* (6)*/
//  }
//  /* Manage the error cases */
//  else if((FLASH->SR & FLASH_SR_WRPERR) != 0) 	/* Check Write protection error */
//  {
//    FLASH->SR |= FLASH_SR_WRPERR; 							/* Clear the flag by software by writing it at 1*/
//  }
//  FLASH->CR &= ~FLASH_CR_PER; 									/* (7) */
	 
	
//	Вариант - 2
//	 
	uint32_t PAGEError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.NbPages = 1;
	EraseInitStruct.PageAddress = FLASH_S_ADR_FLASH+sector;
	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&EraseInitStruct,&PAGEError);
	HAL_FLASH_Lock();
}

/**
	* @brief          Метод перехода в область флеш памяти, для выполнения другого алгоритма (прыжок в другую программу)
	* @param	adress: Адрес ячейки флеш памяти, которую требуется перейти	
  */
void Flash_Jump_Adress(u32 adress)															// Переход в область другой программы во флеш памяти
{
	__set_PRIMASK(1);																							// Отключаем глобальные прерывания(обязательно перед переходом)																						 					
	
	typedef 	void (*pFunction)(void);														// Объявляем тип функции-ссылки
	pFunction Jump_To_Application;																// Объявляем функцию-ссылку

  u32 JumpAddress = *(__IO uint32_t*) (adress + 4); 						// Адрес перехода на вектор (Reset_Handler) 		
  Jump_To_Application = (pFunction) JumpAddress;  							// Указатель на функцию перехода
	__set_MSP(*(volatile u32*) adress);														// Указываем адрес вектора стека(Stack Pointer)	
  Jump_To_Application();                          							// Переходим на основную программу
}	
//------------------------------------------------------------------------

