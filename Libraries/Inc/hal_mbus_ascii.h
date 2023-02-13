#ifndef HAL_MBUS_ASCII_H
#define HAL_MBUS_ASCII_H

#include "stm32f1xx_hal.h"
#include "hal_types.h"

/************************************************
						Протокол Modbus ASCII
*************************************************		
	Структура посылки:
	   начало       адрес      функция        данные         LRC8                конeц
	(0x3A - ":") (0x3X 0x3X) (0x3X 0x3X) (0x3X .... 0x3X) (0xXX 0xXX) (0x0D - "CR" и 0x0A - "LF")
		
	Коды ошибок:
	0x41 - пакет успешно принят и обработан;
	0x42 - если неправильная контрольная сумма;
	0x43 - если такая функция не реализована;
	0x44 - функцию не возможно выполнить;
	
	Статусы флага - PL_Fl[x].stat_rx:
	0 - не принято ни одного байта;
	1 - пакет принят, CRC верна;
	2 - принят байт ошибки (master mode);
	3 - неверное "начало пакета" (1 байт);
	4 - пакет принят частично, истек тайм-аут ожидания следующего байта;
	5 - совпадение байта "конца пакета" и байта данных, ошибка CRC принятого пакета;
*/

#define ASCII_ADR_DEF  1												// Адрес по умолчанию

// !!! Установить требуемые значения !!!
#define ASCII_UART_ON  0												// Вкл. код работы протокола по верх UART
#define ASCII_USB_ON   1												// Вкл. код работы протокола по верх USB

#define ASCII_N_PORTS  1												// Макс. кол-во портов

#define ASCII_IPORT_1  0												// Индекс порта №1
#define ASCII_IPORT_2  1	
#define ASCII_IPORT_3  2	
#define ASCII_IPORT_4  3	
#define ASCII_IPORT_5  4	
#define ASCII_IPORT_6  5	

#define ASCII_BF_RX    255											// Размер массива bfRxData
#define ASCII_BF_TX    255											// Размер массива bfTxData

#define ASCII_CTR_IDLE_RX_PACK_MAX 500					// Макс. значение счетчика ожидания посылки

volatile typedef enum														// Статусы приема данных
{
	ASCII_STAT_INIT_OK = 0,												// ждем новый пакет
	ASCII_STAT_INIT_ERR_N_PORT,										// не верный номер порта
	ASCII_STAT_INIT_ERR_LINK_PORT,								// не задана ссылка на порт
	
} ASCII_StatInit;

volatile typedef enum														// Тип порта по которому будет работать протокол
{
	ASCII_TYPE_PORT_UART = 0,											// физический интерфейс UART
	ASCII_TYPE_PORT_USB_COM,											// физический интерфейс USB
} ASCII_TypePort;

volatile typedef enum														// Режим работы протокола обмена данных через порт
{
	ASCII_MODE_PRTKL_MASTER = 0,									// режим работы протокола - master
	ASCII_MODE_PRTKL_SLAVE,												// режим работы протокола - slave
} ASCII_ModePrtkl;

volatile typedef enum														// Статусы приема данных
{
	ASCII_STAT_RX_WAIT = 0,												// ждем новый пакет
	ASCII_STAT_RX_YES,														// пакет успешно принят, но не обработан
	ASCII_STAT_RX_YES_HANDLE,											// пакет успешно принят и обработан
	ASCII_STAT_RX_ERR_BADPACK,										// пакет принят частично (истек тайм-аут)
	ASCII_STAT_RX_ERR_CRC,												// ошибка CRC
	ASCII_STAT_RX_ERR_ADR,												// пакет адресован не нашему устр-ву
	ASCII_STAT_RX_ERR_TMO,												// истек тайм-аут ожидания (пакет принят частично)
	ASCII_STAT_RX_ERR_START_BYTE,									// не верный начальный байт пакета (1 байт)
	ASCII_STAT_RX_ERR_END_BYTES,									// не верные конечные байты пакета (2 байта)
	ASCII_STAT_RX_SLAVE_ERR_BYTE,									// подчиненное устр-во прислало ответ - байт с кодом ошибки (для режима MASTER)
} ASCII_StatRx;

volatile typedef enum														// Статусы приема данных
{
	ASCII_STAT_TX_NONE = 0,												// ничего не делаем. ждем
	ASCII_STAT_TX_SEND,														// необходимо отправить ответ на запрос (или код ошибки)
	ASCII_STAT_TX_DURING,													// идет передача ответа на запрос (или код ошибки)
	ASCII_STAT_TX_OK,															// ответ на запрос отправлен (или код ошибки)
} ASCII_StatTx;

volatile typedef enum														// Статусы команды записи и чтения регистров карты памяти
{
	ASCII_MAP_WRITE_OK = 0,												// запись данных в регистр - ок
	ASCII_MAP_READ_OK,														// чтение данных из регистра - ок
	ASCII_MAP_ERR_WRITE_ADR,											// указанный адрес не используется
	ASCII_MAP_ERR_WRITE_RRO,											// невозможно записать данные (регистр только для чтения)
	ASCII_MAP_ERR_READ_ADR,												// указанный адрес не используется
	ASCII_MAP_ERR_SIZE_PACK,											// неверный размер пакета
	ASCII_MAP_ERR_CMD,														// неверная функция в запросе
} ASCII_StatMap;

volatile typedef struct 												// Параметры для приема и обработки посылки через заданный порт								
{	
	ASCII_TypePort  type;													// тип порта по которому будет работать протокол
	u8* 						link_p;	             					// указатель на структуру порта с которым будем работать (uart, usb и др.)	
	ASCII_ModePrtkl mode;													// режим работы (master, slave)
	u8 							adr;	             						// адрес порта	
	
	ASCII_StatRx  statRx;				          				// состояние приема пакета
	ASCII_StatTx  statTx;													// состояние передачи пакета
	ASCII_StatMap statMap;												// состояние операции с регистрами карты памяти
	
	bool fl_newPack;															// флаг - запроса нового пакета
	bool fl_startRx;	             								// флаг - начало приема пакета (получен 1 байт)
	bool fl_enableTm;				          						// флаг - разрешение принятия данных в прерывании таймера
	
	u8   ctrBytesRx;	             								// счетчик кол-ва принятых байт в пакете
	u8   ctrLBytesRx;				      								// счетчик кол-ва принятых байт в пакете в прошлый раз
	u16  ctrIdleRxPack;				           					// счетчик ожидания посылки (тайм-аут)
	
	u8*  link_bfRx;															  // буфер с принятыми данными с порта (в ASCII)
	u8*  link_bfTx;															  // буфер с данными для отправки в порт (в ASCII)	 	
	u8   bfRxData[ASCII_BF_RX];										// буфер c принятой и преобразованной посылкой из ASCII в U8
	u8   bfTxData[ASCII_BF_TX];										// буфер c ответом в U8
	
	u8   sizeTx;																	// размер пакета для передачи
	u8   itrPack;																	// межпакетный интервал (задержка перед отправкой пакета)
	
} ASCII_Struct;

/************************************************************************
											Прототипы глобальных переменных
*************************************************************************/
extern ASCII_Struct ASCII_Port[ASCII_N_PORTS];														// порты для приема и обработки посылки

//-----------------------------------------------------------------------	

/************************************************************************
														Прототипы функций
*************************************************************************/
ASCII_StatInit ASCII_Init_Port(u8  n_port, 				ASCII_TypePort  type, 
                               u8* link, 			ASCII_ModePrtkl mode,
															 u8* link_bfRx,	u8* 						link_bfTx); // инициализация порта	
void ASCII_Handler(void);																									// обработчик модуля (положить в Main)													 
void ASCII_Handler_Tm(void);																							// обработчик данных модуля в таймере (1 мс)																 

void ASCII_Process_Receive(u8 n_port);																		// прием посылок c порта
void ASCII_Transmit_Answer(u8 n_port, u8 code);														// Отправляем ответ в порт 
//-----------------------------------------------------------------------

#endif
