#include "hal_mbus_ascii.h"
#include "hal_mbus_ascii_slave.h"
#include "hal_my_lib.h"
#include "hal_my_usb.h"
#include "hal_my_crc.h"

/************************************************************************
										Глобальные перемeнные модуля
*************************************************************************/
ASCII_Struct 	ASCII_Port[ASCII_N_PORTS];																	// Порты для приема и обработки посылки
//-----------------------------------------------------------------------
					
/************************************************************************
										Прототипы всех функций модуля
*************************************************************************/
ASCII_StatInit ASCII_Init_Port(u8  n_port, 		ASCII_TypePort  type, 
                               u8* link, 			ASCII_ModePrtkl mode,
															 u8* link_bfRx,	u8* 						link_bfTx); // инициализация порта
void ASCII_Handler(void);																									// обработчик модуля (положить в Main)													 
void ASCII_Handler_Tm(void);																							// обработчик данных модуля в таймере (1 мс)																 
	
void ASCII_Handler_Packets(u8 n_port);																		// Обработчик принятой с порта посылки
void ASCII_Request_To_Receive(u8 n_port); 																// запрос на прием данных с порта
void ASCII_Port_Reset(u8 n_port);																					// сбрасываем все счетчики, флаги и запросы

void ASCII_Transmit_Answer(u8 n_port, u8 code);														// Отправляем ответ в порт 
void ASCII_Process_Receive(u8 n_port);																		// прием посылок c порта
void ASCII_Process_Receive_USB (u8 n_port);																// прием посылок c порта (USB)
void ASCII_Process_Receive_UART(u8 n_port);																// прием посылок c порта (UART)
//-----------------------------------------------------------------------													
														
/************************************************************************
											 Описание функций модуля
*************************************************************************/
/**
	* @brief  								Инициализация порта, по верх которого будет работать протокол 
	* @param  [IN]    n_port: номер порта
  * @param  [IN] 			type: тип порта
	* @param  [IN] 			link: указатель на структуру порта (uart, usb и др.)
	* @param  [IN] 			mode: режим работы протокола (master, slave)
	* @param  [IN] link_bfRx: указатель на массив с принятыми данными
	* @param  [IN] link_bfTx: указатель на массив с данными для передачи
	* @retval [OUT]      			возвращает статус инициализации
  */
ASCII_StatInit ASCII_Init_Port(u8  n_port, 		ASCII_TypePort  type, 
                               u8* link_p, 		ASCII_ModePrtkl mode,
															 u8* link_bfRx,	u8* 						link_bfTx)
{
	if(n_port >= ASCII_N_PORTS)
		return ASCII_STAT_INIT_ERR_N_PORT;
	else
	if(link_p == NULL)
		return ASCII_STAT_INIT_ERR_LINK_PORT;	
	else
	if(link_bfRx == NULL)
		return ASCII_STAT_INIT_ERR_LINK_PORT;	
	else
	if(link_bfTx == NULL)
		return ASCII_STAT_INIT_ERR_LINK_PORT;	
	
	ASCII_Port[n_port].type   = type;
	ASCII_Port[n_port].link_p = link_p;
	ASCII_Port[n_port].mode   = mode;
	ASCII_Port[n_port].adr    = ASCII_ADR_DEF;
	
	ASCII_Port[n_port].link_bfRx = link_bfRx;
	ASCII_Port[n_port].link_bfTx = link_bfTx;
	
	ASCII_Port[n_port].statRx  = ASCII_STAT_RX_WAIT; // ASCII_STAT_RX_YES_HANDLE
	ASCII_Port[n_port].statTx  = ASCII_STAT_TX_NONE;
	ASCII_Port[n_port].statMap = ASCII_MAP_WRITE_OK;
	
	ASCII_Port[n_port].fl_newPack  = true;
	ASCII_Port[n_port].fl_startRx  = false;
	ASCII_Port[n_port].fl_enableTm = true;
	
	ASCII_Port[n_port].ctrBytesRx 	  = 0;
	ASCII_Port[n_port].ctrLBytesRx   = 0;
	ASCII_Port[n_port].ctrIdleRxPack = 0;
	
	ASCII_Port[n_port].sizeTx 	= 0;
	ASCII_Port[n_port].itrPack = 50;
	
	return ASCII_STAT_INIT_OK;
}

/**
	* @brief Обработчик модуля (положить в Main)
  */
void ASCII_Handler(void)
{
	for(u8 n_port = NULL; n_port < ASCII_N_PORTS; n_port++)
	{
		ASCII_Handler_Packets(n_port); 							
		
		if(ASCII_Port[n_port].statTx == ASCII_STAT_TX_OK &&
			 ASCII_Port[n_port].statRx == ASCII_STAT_RX_YES_HANDLE)
		{
			ASCII_Port_Reset(n_port);
			ASCII_Request_To_Receive(n_port);
		}	
	}
}

/**
	* @brief       	 			Запрос на прием данных с порта
	* @param [IN] n_port: номер порта
  */
void ASCII_Request_To_Receive(u8 n_port) 															
{	
	bool fl_ok = false;
	
	#if ASCII_USB_ON
	if(ASCII_Port[n_port].type == ASCII_TYPE_PORT_USB_COM)
	{
		// перезапрос не требуется, он делается атоматически в дебрях HAL
	}
	#endif
	
	#if ASCII_UART_ON
	if(ASCII_Port[n_port].type == ASCII_TYPE_PORT_UART)
	{
		UART_HandleTypeDef* link = (UART_HandleTypeDef*)ASCII_Port[n_port].link;
		
		__HAL_UART_FLUSH_DRREGISTER(link);
		__HAL_UART_CLEAR_PEFLAG(link);
		
		link->ErrorCode = HAL_UART_ERROR_NONE;
		link->gState 	  = HAL_UART_STATE_READY;
		link->RxState 	= HAL_UART_STATE_READY;
		
		// Запрос данных с порта UART по его номеру
		if(HAL_UART_Receive_IT(link, (u8*)ASCII_Port[n_port].bfRx, ASCII_BUF_RX))
			fl_ok = true;																		
	}	
	#endif	
	
	if(fl_ok)
		ASCII_Port[n_port].fl_newPack = true;							
	else
	{
		ASCII_Port[n_port].fl_newPack  = false;										
		ASCII_Port[n_port].fl_enableTm = true;
		ASCII_Port[n_port].statRx = ASCII_STAT_RX_WAIT;	
	}	
}

/**
  * @brief    		 			Сбрасываем все счетчики, флаги и запросы
	* @param [IN] n_port: номер порта			
  */
void ASCII_Port_Reset(u8 n_port) 																
{
	#if ASCII_USB_ON
	if(ASCII_Port[n_port].type == ASCII_TYPE_PORT_USB_COM)
	{
		USBD_CDC_HandleTypeDef* hcdc = (USBD_CDC_HandleTypeDef*)ASCII_Port[n_port].link_p;
		hcdc->RxLength = 0;
	}
	#endif
	
	#if ASCII_UART_ON
	if(ASCII_Port[n_port].type == ASCII_TYPE_PORT_UART)
	{
		__HAL_UART_FLUSH_DRREGISTER(ASCII_Port[n_port].huart);
		__HAL_UART_CLEAR_PEFLAG(ASCII_Port[n_port].huart);
		
		__HAL_UART_DISABLE_IT(ASCII_Port[n_port].huart, UART_IT_RXNE);
		__HAL_UART_DISABLE_IT(ASCII_Port[n_port].huart, UART_IT_PE);
		__HAL_UART_DISABLE_IT(ASCII_Port[n_port].huart, UART_IT_ERR);
		
		ASCII_Port[n_port].huart->ErrorCode 	= HAL_UART_ERROR_NONE;
		ASCII_Port[n_port].huart->gState 		  = HAL_UART_STATE_READY;
		ASCII_Port[n_port].huart->RxXferCount = ASCII_Port[n_port].huart->RxXferSize;
	}
	#endif
	
	ASCII_Port[n_port].ctrIdleRxPack = 0;
	ASCII_Port[n_port].ctrBytesRx    = 0;
	ASCII_Port[n_port].ctrLBytesRx   = 0;	
	ASCII_Port[n_port].fl_enableTm   = false;																		
	ASCII_Port[n_port].fl_startRx    = false; 																																									
	ASCII_Port[n_port].fl_newPack 	 = false;
}
	
/**
  * @brief     		 			Прием посылок c порта
	* @param [IN] n_port: номер порта			
  */
void ASCII_Process_Receive(u8 n_port) 
{
	#if ASCII_USB_ON
	if(ASCII_Port[n_port].type == ASCII_TYPE_PORT_USB_COM)
		ASCII_Process_Receive_USB(n_port);
	#endif
	
	#if ASCII_UART_ON
	if(ASCII_Port[n_port].type == ASCII_TYPE_PORT_UART)
		ASCII_Process_Receive_UART(n_port);
	#endif
}

/**
  * @brief     		 			Прием посылок c порта (USB)
	* @param [IN] n_port: номер порта			
  */
#if ASCII_USB_ON
void ASCII_Process_Receive_USB(u8 n_port)
{
	USBD_CDC_HandleTypeDef* hcdc = (USBD_CDC_HandleTypeDef*)ASCII_Port[n_port].link_p;	// Ссылка на структуру для работы с USB портом
		
	if(ASCII_Port[n_port].fl_enableTm) 																						 	  	// Разрешаем прием пакетов
	{		
		if(hcdc->RxLength >= 1 && !ASCII_Port[n_port].fl_startRx)	
		{
			if(ASCII_Port[n_port].link_bfRx[0] != 0x3A) 																		
				ASCII_Port[n_port].statRx = ASCII_STAT_RX_ERR_START_BYTE;											// Выставляем флаг - стартовый байт пакета принят успешно, принимаем весь пакет
			else
				ASCII_Port[n_port].fl_startRx = true;
		}
		if(ASCII_Port[n_port].fl_startRx && ASCII_Port[n_port].statRx != ASCII_STAT_RX_YES) 		
		{
			if(ASCII_Port[n_port].link_bfRx[hcdc->RxLength-1] == 0x0A &&
				 ASCII_Port[n_port].link_bfRx[hcdc->RxLength-2] == 0x0D)											// Проверка конца пакета "CR" + "LF"		
			{
				static u8 c_lrc_8, p_lrc_8;
				
				c_lrc_8 = Lrc_8_Ascii((u8*)ASCII_Port[n_port].link_bfRx, hcdc->RxLength);
				MyLib_Convert_String_In_Buf_u08(&ASCII_Port[n_port].link_bfRx[hcdc->RxLength-4], (u8*)&p_lrc_8, 1);
				
				if(c_lrc_8 == p_lrc_8)
				{						
					ASCII_Port[n_port].statRx = ASCII_STAT_RX_YES;															// пакет успешно принят пакет, но не обработан
					ASCII_Port[n_port].ctrBytesRx = (hcdc->RxLength-3)/2;
					MyLib_Convert_String_In_Buf_u08(&ASCII_Port[n_port].link_bfRx[1], (u8*)&ASCII_Port[n_port].bfRxData, hcdc->RxLength-3);
				}
				else
					ASCII_Port[n_port].statRx = ASCII_STAT_RX_ERR_CRC;
			}
			else
				ASCII_Port[n_port].statRx = ASCII_STAT_RX_ERR_END_BYTES;
		}
	}
	if(ASCII_Port[n_port].statRx > ASCII_STAT_RX_YES_HANDLE) 														// обработка ошибок
	{
		ASCII_Port_Reset(n_port);
		ASCII_Request_To_Receive(n_port);
	}
}
#endif

/**
  * @brief     		 			Прием посылок c порта (UART)
	* @param [IN] n_port: номер порта			
  */
#if ASCII_UART_ON
void ASCII_Process_Receive_UART(u8 n_port)
{
//	UART_HandleTypeDef* link_p = (UART_HandleTypeDef*)ASCII_Port[n_port].link;
//	
//	if(ASCII_Port[n_port].statTx == ASCII_STAT_TX_SEND) 																	// Если выставлен статус - отправить ответ
//	{
//		ASCII_Port[n_port].ctrTm++;																											
//		
//		if(ASCII_Port[n_port].ctrTm == ASCII_Port[n_port].itrPack)													// Если счетчик превысил, пора передавать ответ
//		{
//			ASCII_Port[n_port].ctrTm = 0;																								
//			ASCII_Port[n_port].statTx = ASCII_STAT_TX_DURING;										
//			CDC_Transmit_FS((u8*)ASCII_Port[n_port].link_bfTx, ASCII_Port[n_port].sizeTx);
//		}
//	}
//	if(ASCII_Port[n_port].fl_enableTm) 																						 	  	// Разрешаем прием пакетов
//	{		
//		if(ASCII_Port[n_port].ctrBytesRx != NULL && !ASCII_Port[n_port].fl_startRx)										
//			if(ASCII_Port[n_port].ctrBytesRx == 1)
//				if(ASCII_Port[n_port].link_bfRx[0] != 0x3A) 																		
//				{	
//					ASCII_Port[n_port].statRx = ASCII_STAT_RX_ERR_START_BYTE;										// Выставляем флаг - стартовый байт пакета принят успешно, принимаем весь пакет
//					ASCII_Port_Reset(n_port);						
//				}
//		if(ASCII_Port[n_port].fl_startRx && ASCII_Port[n_port].statRx != ASCII_STAT_RX_YES) 		
//			if(link_p->RxXferSize != link_p->RxXferCount)																// Если приняли еще байт			
//				if(ASCII_Port[n_port].link_bfRx[ASCII_Port[n_port].ctrBytesRx-1] == 0x0A &&
//					 ASCII_Port[n_port].link_bfRx[ASCII_Port[n_port].ctrBytesRx-2] == 0x0D)			// Ищем конец пакета "CR" + "LF"		
//				{
//					static vu8 lrc_8 = 0;
//					lrc_8 = Lrc_8_Ascii((u8*)ASCII_Port[n_port].link_bfRx, ASCII_Port[n_port].ctrBytesRx);
//					
//					if(lrc_8 == ASCII_Port[n_port].link_bfRx[ASCII_Port[n_port].ctrBytesRx-2])		
//					{
//						ASCII_Port[n_port].statRx = ASCII_STAT_RX_YES;														// выставляем статус - успешно принят пакет, но не обработан
//						ASCII_Port_Reset(n_port);
//					}
//				}
//		ASCII_Port[n_port].ctrBytesRx = link_p->RxXferSize - link_p->RxXferCount;	
//	}	
}
#endif

/**
  * @brief Обработчик данных модуля в таймере (1 мс)
  */
void ASCII_Handler_Tm(void) 
{
	for(u8 n_port = 0; n_port < ASCII_N_PORTS; n_port++)
	{
		if(ASCII_Port[n_port].fl_startRx)
		{
			ASCII_Port[n_port].ctrIdleRxPack++;
			
			if(ASCII_Port[n_port].ctrIdleRxPack == ASCII_CTR_IDLE_RX_PACK_MAX)
				ASCII_Port_Reset(n_port);
		}
		else
		if(ASCII_Port[n_port].statTx == ASCII_STAT_TX_SEND)
			if(!CDC_Check_Transmit_End())										
				ASCII_Port[n_port].statTx = ASCII_STAT_TX_OK;
	}
}

/**
  * @brief 							Обработчик принятой с порта посылки
	* @param [IN] n_port: номер порта		
  */
void ASCII_Handler_Packets(u8 n_port) 
{
	if(ASCII_Port[n_port].statRx > ASCII_STAT_RX_WAIT)
		if(ASCII_Port[n_port].statRx != ASCII_STAT_RX_YES_HANDLE)
		{					
			if(ASCII_Port[n_port].statRx == ASCII_STAT_RX_YES)												
			{
				if(ASCII_Port[n_port].mode == ASCII_MODE_PRTKL_SLAVE)													
					ASCII_Analiz_Pack_Slave((u8*)ASCII_Port[n_port].bfRxData, 
																	(u8*)ASCII_Port[n_port].bfTxData, 
																			 ASCII_Port[n_port].ctrBytesRx, 
																			 n_port);	 																
				//else
					//ASCII_Analiz_Pack_Master();
				
				ASCII_Port[n_port].statRx = ASCII_STAT_RX_YES_HANDLE;						
			}
			else
			{
				if(ASCII_Port[n_port].statRx == ASCII_STAT_RX_ERR_CRC)										
					ASCII_Transmit_Answer(n_port, 0x08);
				else					
					ASCII_Port[n_port].statTx = ASCII_STAT_TX_OK;
				
				ASCII_Port[n_port].statRx = ASCII_STAT_RX_YES_HANDLE;																			
			}					
		}
} 

/**
  * @brief        	Отправляем ответ в порт
	* @param	n_port: номер порта
	* @param		code:	код ошибки							 
  */
void ASCII_Transmit_Answer(u8 n_port, u8 code) 															
{
	static u16 len;
	
	if(!ASCII_Port[n_port].bfRxData[0])																								// Если широковещательный пакет
		return;
	
	#if ASCII_USB_ON
		if(ASCII_Port[n_port].type == ASCII_TYPE_PORT_USB_COM)
		{
			ASCII_Port[n_port].statTx = ASCII_STAT_TX_SEND;																// Флаг - идет передача ответа
			ASCII_Port[n_port].link_bfTx[0] = 0x3A;
			
			if(code != NULL)
			{
				ASCII_Port[n_port].sizeTx = 4;
				ASCII_Port[n_port].bfTxData[0] = ASCII_Port[n_port].bfRxData[0];
				ASCII_Port[n_port].bfTxData[1] = ASCII_Port[n_port].bfRxData[1]|0x80;
				ASCII_Port[n_port].bfTxData[2] = code;
				ASCII_Port[n_port].bfTxData[3] = Lrc_8((u8*)ASCII_Port[n_port].bfTxData, ASCII_Port[n_port].sizeTx-1);
			}
			MyLib_Convert_Buf_u08_In_String((u8*)ASCII_Port[n_port].bfTxData,
																					&ASCII_Port[n_port].link_bfTx[1],
																					 ASCII_Port[n_port].sizeTx);
			len = ASCII_Port[n_port].sizeTx*2;
			len += 3;
			ASCII_Port[n_port].link_bfTx[len-2] = 0x0D;
			ASCII_Port[n_port].link_bfTx[len-1] = 0x0A;
			CDC_Transmit_FS(ASCII_Port[n_port].link_bfTx, len);														// Отправить данные в порт USB
		}
	#endif
//	#if ASCII_UART_ON
//		if(ASCII_Port[n_port].type == ASCII_TYPE_PORT_UART)
//		{
//			if(ASCII_Port[n_port].bfTx[2] != 0 && ASCII_Port[n_port].sizeTx > 0)				// Если адрес не "0" и есть данные на отправку
//				ASCII_Port[n_port].statTx = ASCII_STAT_TX_SEND;																	
//			else
//			{																					
//				ASCII_Port[n_port].newPack = true;																				// перезапрос нового пакета
//				ASCII_Port[n_port].statTx  = ASCII_STAT_TX_NONE;
//				ASCII_Port[n_port].statRx  = ASCII_STAT_RX_YES_HANDLE;										// пакет успешно принят и обработан
//			}
//		}
//	#endif
}
//-----------------------------------------------------------------------
