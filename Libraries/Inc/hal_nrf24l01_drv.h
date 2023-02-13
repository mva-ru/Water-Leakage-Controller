#ifndef HAL_NRF24L01_DRV
#define HAL_NRF24L01_DRV

#include "hal_types.h"

/******************************************************************************************************************
****************************************    Описание модуля nRF24L01    *******************************************
******************************************************************************************************************/
// Радиотрансивер nRF24L01 работает по SPI. 
// Возможна одновременная работа по 6 радиоканалам (1-приемник и 5 передатчиков).

// Назначение выводов: 
// VCC - Питание от 1.9 до 3.6 Вольт; 
// CE  - Прием(1)\передача(0) данных по радиоэфиру 2.4 ГГц (Chip Enable);
// CSN - Устанавливает границы сеанса обмена данными по шине SPI (Chip Select);
// IRQ - Прерывания по событиям от nRF24L01 (необходимые события настраиваются);

// Внимание!!!
// Если при чтении регистра размером 5 байт указать, чтение 6 и более байт, то
// в посылке значение всех последующих байт начиная с 6-го будут равны значению 1-го байта.
//-----------------------------------------------------------------------------------------------------------------

/******************************************************************************************************************
****************************    Настраиваемые макроопределения для модуля nRF24L01    *****************************
******************************************************************************************************************/
#define NRF24_PORT_CE  					GPIOA					//*****!!Внести изменения!!******
#define NRF24_PIN_CE 						GPIO_PIN_4		//*****!!Внести изменения!!******
#define NRF24_CE_HI()   				HAL_GPIO_WritePin (NRF24_PORT_CE, NRF24_PIN_CE, GPIO_PIN_SET)  		
#define NRF24_CE_LO()  					HAL_GPIO_WritePin (NRF24_PORT_CE, NRF24_PIN_CE, GPIO_PIN_RESET)
#define NRF24_READ_CE()   			HAL_GPIO_ReadPin  (NRF24_PORT_CE, NRF24_PIN_CE) 

#define NRF24_PORT_CSN  				GPIOB					//*****!!Внести изменения!!******
#define NRF24_PIN_CSN 					GPIO_PIN_0		//*****!!Внести изменения!!******
#define NRF24_CSN_HI()   				HAL_GPIO_WritePin (NRF24_PORT_CSN, NRF24_PIN_CSN, GPIO_PIN_SET)  		
#define NRF24_CSN_LO()  				HAL_GPIO_WritePin (NRF24_PORT_CSN, NRF24_PIN_CSN, GPIO_PIN_RESET) 

#define NRF24_PORT_IRQ  				GPIOB 				//*****!!Внести изменения!!******
#define NRF24_PIN_IRQ 					GPIO_PIN_1		//*****!!Внести изменения!!******
#define NRF24_READ_IRQ()   			HAL_GPIO_ReadPin  (NRF24_PORT_IRQ, NRF24_PIN_IRQ)  									
//-----------------------------------------------------------------------------------------------------------------
#define NRF24_DEV_0     		  	0			// удаленное устр-во №1
#define NRF24_DEV_1     		  	1							
#define NRF24_DEV_2     		  	2					
#define NRF24_DEV_3     		  	3							
#define NRF24_DEV_4     		  	4
#define NRF24_DEV_5     		  	5

#define NRF24_ISS     		  		0			// индекс первого байта в массиве
#define NRF24_ISD     		  		1			// индекс начала данных в массиве приема\передачи (в [0] номер регистра)

#define NRF24_MAX_DEV						6			// макс. кол-во удаленных устр-в (с которыми будет работать текущее в режиме передатчика)

#define NRF24_ADR_SZ_1					1			// размер массива адреса приемника
#define NRF24_ADR_SZ_5					5			// размер массива адреса приемника\передатчика
#define NRF24_SBUF_SPI_TX     	33		// размер буфера передатчика (в байтах)
#define NRF24_SBUF_SPI_RX       33		// размер буфера приемника 	 (в байтах)
#define NRF24_SBUF_RADIO       	32		// размер буфера данных для прием\передачи по радиоэфиру (в байтах)
#define NRF24_IDX_BF_TX_MAX   	32		// значение индекса последнего элемента массива в буфере передатчика
#define NRF24_IDX_BF_RX_MAX   	32		// значение индекса последнего элемента массива в буфере приемника

#define NRF24_TMO_TXRX_WHILE 		1000	// время ожидания приёма данных в цикле (мс)
//-----------------------------------------------------------------------------------------------------------------
#define NRF24_OFFSET_HZ_MIN			0			// частота смещения от несущей, мин. значение
#define NRF24_OFFSET_HZ_MAX			125		// частота смещения от несущей, макс. значение
//-----------------------------------------------------------------------------------------------------------------
#define NRF24_MODE_RX						true	// вкл. режим передатчика радиотрансивера (PRIM_RX = 0 - передатчик)
#define NRF24_MODE_TX						false	// вкл. режим приёмника радиотрансивера (PRIM_RX = 1 - приёмник)
//-----------------------------------------------------------------------------------------------------------------
#define NRF24_MODE_ACTIVE				true	// вкл. активный режим радиотрансивера
#define NRF24_MODE_PASSIVE			false	// вкл. пассивный режим радиотрансивера
//-----------------------------------------------------------------------------------------------------------------
#define NRF24_POWER_ON					true	
#define NRF24_POWER_OFF					false	
//-----------------------------------------------------------------------------------------------------------------
volatile typedef enum
{
	NRF24_MODE_MASTER	= 0,						// устр-во работает в режиме ведущего
	NRF24_MODE_SLAVE,									// устр-во работает в режиме ведомого
} NRF24_Enum_Mode_Dev;							// Режим работы устр-ва

volatile typedef enum
{
	NRF24_TYPE_RX_TX_IT = 0,					// передача данных в прерывании
	NRF24_TYPE_RX_TX_DMA,							// передача данных через DMA
	NRF24_TYPE_RX_TX_WHILE,						// передача данных в цикле (может тормазить программу)
} NRF24_Enum_Mode_Spi;							// Метод передачи данных по SPI

volatile typedef enum
{
	NRF24_STAT_SPI_TXRX_NONE = 0,			// Контроллер не работает по шине SPI	
	NRF24_STAT_SPI_TXRX_IDLE,					// Идёт прием\передача данных
	NRF24_STAT_SPI_TXRX_OK,						// Прием\передача данных успешно завершена
	NRF24_STAT_SPI_TXRX_ERR,					// Ошибка в процессе приема\передачи данных
} NRF24_Enum_Spi;										// Состояния приема\передачи данных между радиопередатчиком и контроллером

volatile typedef enum
{
	NRF24_ERR_NONE = 0,								// Нет ошибок
	NRF24_ERR_CONNECT_SPI,						// Нет связи по SPI
	NRF24_ERR_WRITE_SETTINGS,					// Не удалось записать настройки
	NRF24_ERR_COMPARE_CFG,						// Конф-я в nfr24 не совпадает с текущей (после чтения и сравнения)
	NRF24_ERR_POWER_ON_RX,						// Не выполнена команда вкл. питание и перейти в режим приемника
	NRF24_ERR_POWER_ON_TX,						// Не выполнена команда вкл. питание и перейти в режим передатчика
	NRF24_ERR_STANDBY_1,							// Не выполнена команда перехода в режим "ожидание-I"
	NRF24_ERR_STANDBY_2,							// Не выполнена команда перехода в режим "ожидание-II"
	
	NRF24_ERR_CLEAR_FIFO_TX,					// Команда для очистки очереди передатчика радиотрансивера не выполнена
	NRF24_ERR_CLEAR_FIFO_RX,					// Команда для очистки очереди передатчика радиотрансивера не выполнена
	
	NRF24_ERR_GET_REG_STAT,						// Команда на чтение регистра статуса радиотрансивера не выполнена
	NRF24_ERR_GET_REG_STAT_FIFO,			// Команда на чтение регистра статуса очереди радиотрансивера не выполнена
	
	NRF24_ERR_CLEAR_FLAGS_REG_STAT,		// Команда на запись регистра Status для сброса флагов не выполнена
	
	NRF24_ERR_WR_FIFO_TX_NO_ACK,			// Ошибка записи данных в очередь радиотрансивера с пакетом подтверждения
	NRF24_ERR_WR_FIFO_TX_ACK,					// Ошибка записи данных в очередь радиотрансивера без пакета подтверждения	
	
	NRF24_ERR_CMD_UNKNOWN,						// Неизвестная команда
	NRF24_ERR_RADIO_CRC,							// Неверная CRC принятого пакета с данными радиопередатчика
	NRF24_ERR_RADIO_SETUP,						// Сбросились\повредились\изменились настройки радиопередатчика
	NRF24_ERR_CMD_RADIO_DO_NOT_TX,		// Операция по отправке команды в радиопередатчик не выполнена
	NRF24_ERR_CMD_RADIO_DO_NOT_RX,		// Операция по приему данных от радиопередатчика, после отправки команды не выполнена

	NRF24_ERR_CMD_GET_SIZE_BF_RX,			// Ошибка выолнения команды по запросу кол-ва принятых данных в приемном буфере	
	NRF24_ERR_SIZE_RX_DATA,						// Ошибка - радиотрансивер принял больше 32 байт данных в пакете
	
} NRF24_Enum_Err;										// Cостояния ошибок драйвера

volatile typedef enum
{
	NRF24_MODE_RADIO_RX = 0,					// только прием данных
	NRF24_MODE_RADIO_TX,							// только передача данных
	NRF24_MODE_RADIO_RX_TX,						// HALF DUPLEX
} NRF24_Enum_modeTR;								// Режим работы радиотрансивера

volatile typedef enum
{
	NRF24_MODE_ENERGY_HIGH = 0,				// большое потребление, быстрая скорость готовности приема\передачи
	NRF24_MODE_ENERGY_MIDLE,					// среднее потребление, средняя скорость готовности приема\передачи
	NRF24_MODE_ENERGY_LOW,						// маленькое потребление, медленноая скорость готовности приема\передачи
} NRF24_Enum_mEnergy;								// Режим энергопотребления устр-ва

volatile typedef enum
{
	NRF24_SPEED_2Mb = 0,							// 2 Мбит/с
	NRF24_SPEED_1Mb,
	NRF24_SPEED_250Kb,				
} NRF24_Enum_Speed;									// Скорость радио приема\передачи данных

volatile typedef enum
{
	NRF24_ALG_INIT = 0,									//0 Идет иниц-я радиотрансивера
	NRF24_ALG_INIT_OK,									//1 Иниц-я радиотрансивера прошла успешно
	
	NRF24_ALG_POWER_OFF,								//2 Выкл. радиотрансивер
	
	NRF24_ALG_STANDBY_1,								//3 Перейти в режим ожидания "ожидание-I"
	NRF24_ALG_STANDBY_2,								//4 Перейти в режим ожидания "ожидание-II"
  NRF24_ALG_STANDBY_1_OK,							//5 Перешли успешно в режим "ожидание-I"
	NRF24_ALG_STANDBY_2_OK,							//6 Перешли успешно в режим "ожидание-II"
	
	NRF24_ALG_FIFO_RX_READ_DATA,				//7 Пора читать данные из FIFO очереди 
	NRF24_ALG_FIFO_RX_READ_DATA_OK,			//8 Данные успешно записаны
	
	NRF24_ALG_FIFO_TX_WRITE_DATA,				//9 Пора записать данные для отправки в радиоэфир, в очередь
	NRF24_ALG_FIFO_TX_WRITE_DATA_OK,		//10 Данные успешно записаны
	
	NRF24_ALG_WAIT_TX_DATA_RADIO,				//11 Ожидание окончания передачи данных в радиоэфир
	NRF24_ALG_WAIT_TX_DATA_RADIO_OK,		//12 Данные успешно переданы
	
	NRF24_ALG_TX_CONFIRM_IDLE,					//13 Ожидание подтверждения отправленных данных, удаленной стороной
	NRF24_ALG_RX_DATA_RADIO,						//14 Ожидание данных от удаленной стороны из радиоэфира
	
	NRF24_ALG_IRQ,											//15 Сработало прерывание по событию от радиотрансивера, пора его обработать
	NRF24_ALG_IDLE_RX_DATA,							//16 Ожидание приема данных

	NRF24_ALG_IDLE_BUF_MEDIUM_FULL,			//17 Ожидание заполнения промежуточного буфера данными
	
} NRF24_Enum_Alg;											// Состояния для реализации алгоритма радиотрансивера

volatile typedef enum 								 		
{
	NRF24_CMD_R_REG 		 		 = 0x00, 	// Прочитать регистр
	NRF24_CMD_W_REG 				 = 0x20, 	// Записать регистр
	NRF24_CMD_R_RX_PLD 			 = 0x61, 	// Используется в режиме приема. Принять данные данные из верхнего слота очереди приёмника 
	NRF24_CMD_W_TX_PLD 			 = 0xA0, 	// Используется в режиме передачи. Записать в очередь передатчика данные для отправки
	NRF24_CMD_FLUSH_TX     	 = 0xE1, 	// Сбросить очередь передатчика
	NRF24_CMD_FLUSH_RX     	 = 0xE2, 	// Сбросить очередь приёмника
	NRF24_CMD_REUSE_TX_PL  	 = 0xE3, 	// Использовать повторно последний переданный пакет
	NRF24_CMD_R_RX_PL_WID  	 = 0x60, 	// Прочитать размер данных принятого пакета в начале очереди приёмника 
	NRF24_CMD_W_ACK_PLD      = 0xA8, 	// Используется в режиме приема. Записать данные для отправки в пакет подтверждения. 
	NRF24_CMD_W_TX_PLD_NOACK = 0xB0, 	// Используется в режиме передачи. Записать в очередь передатчика данные, для отправки без подтверждения
	NRF24_CMD_NOP            = 0xFF, 	// Нет операции. Может быть использовано для чтения регистра статуса
} NRF24_Enum_Cmd;									 	// Команды управления NRF2424L01

volatile typedef enum 								 		 
{
	NRF24_REG_00_CONFIG      = 0x00, 	// Регистр настроек
	NRF24_REG_01_EN_AA     	 = 0x01, 	// Выбор автоподтверждения
	NRF24_REG_02_EN_RXADDR   = 0x02, 	// Выбор каналов приёмника
	NRF24_REG_03_SETUP_AW    = 0x03, 	// Настройка размера адреса
	NRF24_REG_04_SETUP_RETR  = 0x04, 	// Настройка повторной отправки
	NRF24_REG_05_RF_CH       = 0x05, 	// Частота радиоканала, на котором осуществляется работа. От 0 до 125. 
	NRF24_REG_06_RF_SETUP    = 0x06, 	// Настройка радиоканала
	NRF24_REG_07_STATUS      = 0x07, 	// Регистр статусов 
	NRF24_REG_08_OBSERVE_TX  = 0x08, 	// Количество повторов передачи и потерянных пакетов
	NRF24_REG_09_CD          = 0x09, 	// Обнаружение несущей частоты
	NRF24_REG_0A_RX_ADDR_P0  = 0x0A, 	// Адрес канала приёмника №0
	NRF24_REG_0B_RX_ADDR_P1  = 0x0B, 	// Адрес канала приёмника №1
	NRF24_REG_0C_RX_ADDR_P2  = 0x0C, 	// Адрес канала приёмника №2
	NRF24_REG_0D_RX_ADDR_P3  = 0x0D, 	// Адрес канала приёмника №3 
	NRF24_REG_0E_RX_ADDR_P4  = 0x0E, 	// Адрес канала приёмника №4 
	NRF24_REG_0F_RX_ADDR_P5  = 0x0F, 	// Адрес канала приёмника №5 
	NRF24_REG_10_TX_ADDR     = 0x10, 	// Адрес удалённого устройства для передачи
	NRF24_REG_11_RX_PW_P0    = 0x11, 	// Размер данных при приёме по каналу 0: от 1 до 32. 0 - канал не используется.
	NRF24_REG_12_RX_PW_P1    = 0x12, 	// Размер данных при приёме по каналу 1: от 1 до 32. 0 - канал не используется.
	NRF24_REG_13_RX_PW_P2    = 0x13, 	// Размер данных при приёме по каналу 2: от 1 до 32. 0 - канал не используется.
	NRF24_REG_14_RX_PW_P3    = 0x14, 	// Размер данных при приёме по каналу 3: от 1 до 32. 0 - канал не используется.
	NRF24_REG_15_RX_PW_P4    = 0x15, 	// Размер данных при приёме по каналу 4: от 1 до 32. 0 - канал не используется.
	NRF24_REG_16_RX_PW_P5    = 0x16, 	// Размер данных при приёме по каналу 5: от 1 до 32. 0 - канал не используется.
	NRF24_REG_17_FIFO_STATUS = 0x17, 	// Состояние очередей FIFO приёмника и передатчика
	NRF24_REG_1C_DYNPD       = 0x1C, 	// Выбор каналов приёмника для которых используется произвольная длина пакетов.
	NRF24_REG_1D_FEATURE     = 0x1D, 	// Регистр опций
} NRF24_Enum_Adr_Regs;						 	// Адреса и соответствующие названия регистров NRF2424L01

volatile typedef enum
{
	NRF24_ADR_LEN_NOT = 0,				// не задано (нельзя писать нуль)
	NRF24_ADR_LEN_3,							// 3 байта
	NRF24_ADR_LEN_4,							// 4 байта
	NRF24_ADR_LEN_5,							// 5 байта			 
} NRF24_Enum_Adr_Len;						// Выбор размера адреса устр-ва

volatile typedef enum
{
	NRF24_ARD_250mks  = 0x00,			// 250мкс
	NRF24_ARD_500mks  = 0x01,								
	NRF24_ARD_750mks  = 0x02,	
	NRF24_ARD_1000mks = 0x04,	
	NRF24_ARD_4000mks = 0x0f,			 
} NRF24_Enum_Ard;								// Выбор времени ожидания авто перезапроса (повторной передачи данных)

volatile typedef enum
{
	NRF24_ARC_NOT = 0,						// выкл. повторные запросы
	NRF24_ARC_1,									// 1 раз													
	NRF24_ARC_2,
	NRF24_ARC_3,
	NRF24_ARC_4,
	NRF24_ARC_5,
	NRF24_ARC_6,
	NRF24_ARC_7,
	NRF24_ARC_8,
	NRF24_ARC_9,
	NRF24_ARC_10,
	NRF24_ARC_11,
	NRF24_ARC_12,
	NRF24_ARC_13,
	NRF24_ARC_14,
	NRF24_ARC_15,			 
} NRF24_Enum_Arс;								// Выбор кол-ва перезапросов

volatile typedef enum
{
	NRF24_GAIN_m18dBm = 0x00,			// -18dBm
	NRF24_GAIN_m12dBm = 0x01,								
	NRF24_GAIN_m06dBm = 0x02,	
	NRF24_GAIN_00dBm  = 0x03,			// 0dBm	самая высокая мощность сигнала			 
} NRF24_Enum_Gain;							// Значения усиления сигнала

volatile typedef union					// CONFIG - 0x00 - Регистр настроек
{
 struct 
 { 
	 u8 _0_prim_rx:1;							// R\W Выбор режима 		 (0 - передатчик; 1 - приёмник)
	 u8 _1_pwr_up:1;							// R\W Включение питания (0 - выкл.; 1 -вкл.)
	 u8 _2_sz_crc:1;							// R\W Размер поля CRC 	 (0 - 1 байт; 1 - 2 байта)
	 u8 _3_en_crc:1;						  // R\W Включает CRC
	 u8 _4_mask_mx_rt:1;					// R\W Запрещает прерывание по MAX_RT (превышение числа повторных попыток отправки) 
	 u8 _5_mask_tx_ds:1;					// R\W Запрещает прерывание по TX_DS  (завершение отправки пакета)
	 u8 _6_mask_rx_rd:1;					// R\W Запрещает прерывание по RX_DR  (получение пакета)
	 u8 _7_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Cfg;								// Назначение битов каждого из регистров NRF2424L01

volatile typedef union					// EN_AA - 0x01 - Выбор автоподтверждения
{
 struct 
 { 
	 u8 _0_en_auto_ch0:1;					// R\W Включает автоподтверждение данных, полученных по каналу 0
	 u8 _1_en_auto_ch1:1;					// R\W Включает автоподтверждение данных, полученных по каналу 1							
	 u8 _2_en_auto_ch2:1;					// R\W Включает автоподтверждение данных, полученных по каналу 2						
	 u8 _3_en_auto_ch3:1;					// R\W Включает автоподтверждение данных, полученных по каналу 3						 
	 u8 _4_en_auto_ch4:1;					// R\W Включает автоподтверждение данных, полученных по каналу 4						
	 u8 _5_en_auto_ch5:1;					// R\W Включает автоподтверждение данных, полученных по каналу 5						
	 u8 _6_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю				
	 u8 _7_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_En_Aa;

volatile typedef union					// EN_RXADDR - 0x02 - Выбор каналов приёмника
{
 struct 
 { 
	 u8 _0_en_ch0:1;							// R\W Включает (ON) канал 0 приёмника (RX)
	 u8 _1_en_ch1:1;							// R\W Включает (ON) канал 1 приёмника (RX)							
	 u8 _2_en_ch2:1;							// R\W Включает (ON) канал 2 приёмника (RX)						
	 u8 _3_en_ch3:1;							// R\W Включает (ON) канал 3 приёмника (RX)						 
	 u8 _4_en_ch4:1;							// R\W Включает (ON) канал 4 приёмника (RX)						
	 u8 _5_en_ch5:1;							// R\W Включает (ON) канал 5 приёмника (RX)						
	 u8 _6_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю						
	 u8 _7_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_En_RxAddr; 

volatile typedef union					// SETUP_AW - 0x03 - Настройка размера адреса
{
 struct 
 { 
	 u8 _0_1_adr_sz:2;						// R\W 0-1 биты. Выбор размера поля адреса: 1- 3 байта; 2- 4 байта; 3- 5 байт														
	 u8 _2_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю					
	 u8 _3_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю					 
	 u8 _4_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю					
	 u8 _5_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю					
	 u8 _6_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю						
	 u8 _7_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Setup_AW;

volatile typedef union					// SETUP_RETR - 0x04 - Настройка повторной отправки
{
 struct 
 { 
	 u8 _0_3_arc:4;								// R\W 0-3 биты. Количество повторных попыток отправки, 0 - повторная отправка отключена (от 0 до 15)										 
	 u8 _4_7_ard:4;								// R\W 4-7 биты. Значение задержки перед повторной отправкой пакета: 250 x (n + 1) мкс	(от 250 до 4000 мкс)								
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Setup_Petr;

volatile typedef union					// RF_CH - 0x05 - Частота радиоканала (несущая частота). 2400 + RF_CH(От 0 до 125) МГц
{
 struct 
 { 
	 u8 _0_6_rfch:7;							// R\W 0-6 биты. Задает частоту каналов b000010 (только для NRF2424L01+)													
	 u8 _7_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю						
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Rf_Ch;

volatile typedef union					// RF_SETUP - 0x06 - Настройка радиоканала
{
 struct 
 { 
	 u8 _0_none:1;								// don't care (не волнует; не важно)
	 u8 _1_2_rf_pwr:2;						// R\W 1-2 биты. Выбор мощности передатчика: 0- (-18dBm); 1- (-16dBm); 2- (-6dBm); 3- (0dBm)																			
	 u8 _3_rf_dr_hight:1;					// R\W Выбор скорости обмена (при значении бита RF_DR_LOW = 0): 0- 1Мбит/с; 1- 2Мбит/с										 
	 u8 _4_pll_lock:1;	 					// R\W force lock pll signal(сила несущей частоты сигнала). Используется для тестов.							
	 u8 _5_rf_dr_low:1;						// R\W Включает скорость 250кбит/с. RF_DR_HIGH должен быть 0.	(Только для NRF2424L01+)										 				
	 u8 _6_reservd:1;							// R\W Резерв. Всегда должно быть равно нулю		
	 u8 _7_cont_wave:1;						// R\W Непрерывная передача несущей. Используется для тестов. (Только для NRF2424L01+)	 
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Rf_Setup;

volatile typedef union					// STATUS - 0x07 - Регистр статусов 
{
 struct 
 { 
	 u8 _0_tx_full:1;							// R Признак заполнения FIFO передатчика: 1 - заполнено; 0 - есть доступные слоты 
	 u8 _1_3_rx_p_no:3;						// R 1-3 биты. Номер канала, данные для которого доступны в FIFO приёмника. 7 -  FIFO пусто.																				 
	 u8 _4_max_rt:1;	 						// R\W Флаг превышено установленное число повторов. Без сброса (записать 1) обмен невозможен					
	 u8 _5_tx_ds:1;								// R\W Флаг завершения передачи. Для сброса флага нужно записать 1								 				
	 u8 _6_rx_dr:1;								// R\W Флаг получения новых данных в FIFO приёмника. Для сброса флага нужно записать 1
	 u8 _7_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Status;

volatile typedef union					// OBSERVE_TX - 0x08 - Кол-во повторов передачи и потерянных пакетов 
{
 struct 
 { 
	 u8 _0_arc_cnt:1;							// R
	 u8 _1_arc_cnt:1;										
	 u8 _2_arc_cnt:1;												
	 u8 _3_arc_cnt:1;																	 
	 u8 _4_plos_cnt:1;	 					// R	
	 u8 _5_plos_cnt:1;													 				
	 u8 _6_plos_cnt:1;						 
	 u8 _7_plos_cnt:1;						 
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Observe_Tx;

volatile typedef union					// CD - 0x09 - Обнаружение несущей. Если младший бит = 1, то уровень мощности более -64dBm   
{
 struct 
 { 
	 u8 _0_cd:1;									// R Carrier Detect - обнаружение несущей (частоты 2400 GHz)
	 u8 _1_reserve:1;							// R Резерв. Всегда должно быть равно нулю 		
	 u8 _2_reserve:1;							// R Резерв. Всегда должно быть равно нулю 				
	 u8 _3_reserve:1;							// R Резерв. Всегда должно быть равно нулю 																 
	 u8 _4_reserve:1;							// R Резерв. Всегда должно быть равно нулю 	
	 u8 _5_reserve:1;							// R Резерв. Всегда должно быть равно нулю 												 				
	 u8 _6_reserve:1;							// R Резерв. Всегда должно быть равно нулю 					 
	 u8 _7_reserve:1;							// R Резерв. Всегда должно быть равно нулю 				 
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Cd;

volatile typedef union					// RX_PW_P0 - 0x11 - Размер данных при приёме по каналу 0
{
 struct 
 { 
	 u8 _0_5_sz_data:6;						// R\W 0-5 биты. Значение от 1 до 32.										
	 u8 _6_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 					 
	 u8 _7_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 					 
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Rx_Pw_P0;

volatile typedef union					// FIFO_STATUS - 0x17 - Состояние очередей FIFO приёмника и передатчика
{
 struct 
 { 
	 u8 _0_rx_empty:1;						// R Флаг освобождения FIFO очереди приёмника: 0 - в очереди есть данные; 1 - очередь пуста.
	 u8 _1_rx_full:1;							// R Флаг переполнения FIFO очереди приёмника: 0 - есть свободное место в очереди; 1 - очередь переполнена.											
	 u8 _2_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 													
	 u8 _3_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 																										 
	 u8 _4_tx_empty:1;						// R Флаг освобождения FIFO очереди передатчика: 0 - в очереди есть данные; 1 - очередь пуста.										
	 u8 _5_tx_full:1;							// R Флаг переполнения FIFO очереди передатчика: 0 - есть свободное место в очереди; 1 - очередь переполнена.											
	 u8 _6_tx_reuse:1;						// R Признак готовности последнего пакета для повтрной отправки. Устанавливается командой REUSE_TX_PL.					 
	 u8 _7_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 					 
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Fifo_Stat;

volatile typedef union					// DYNPD - 0x1C - Выбор каналов приёмника для которых используется произвольная длина пакетов
{
 struct 
 { 
	 u8 _0_dpl_ch0:1;							// разрешить приём пакетов произвольной длины по каналу №0
	 u8 _1_dpl_ch1:1;															
	 u8 _2_dpl_ch2:1;																		
	 u8 _3_dpl_ch3:1;																																 
	 u8 _4_dpl_ch4:1;														
	 u8 _5_dpl_ch5:1;																
	 u8 _6_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 	
	 u8 _7_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 					 
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Dynpd;

volatile typedef union					// FEATURE - 0x1D - Регистр опций
{
 struct 
 { 
	 u8 _0_en_dyn_ack:1;					// R\W Разрешает передавать пакеты, не требующие подтверждения приёма 
	 u8 _1_en_ack_pay:1;					// R\W Включает поддержку передачи данных с пакетами подтверждения.										
	 u8 _2_en_dpl:1;							// R\W Включает поддержку приёма и передачи пакетов с размером поля данных произвольной длины.												
	 u8 _3_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 																										 
	 u8 _4_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 
	 u8 _5_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 										
	 u8 _6_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 
	 u8 _7_reserve:1;							// R\W Резерв. Всегда должно быть равно нулю 					 
 } bit;											 	  // значения битов
 u8 full;												// значение байта
} NRF24_Reg_Feature;
//-----------------------------------------------------------------------------------------------------------------

/******************************************************************************************************************
*******************************   Структура - карта регистров NRF2424L01   ****************************************
******************************************************************************************************************/
volatile typedef struct 								
{																		
	NRF24_Reg_Cfg 				_0x00_cfg;												// CONFIG 	   - Регистр настроек
	NRF24_Reg_En_Aa				_0x01_en_aa;											// EN_AA  	   - Выбор автоподтверждения
	NRF24_Reg_En_RxAddr		_0x02_en_rx_addr; 								// EN_RXADDR   - Выбор каналов приёмника
	NRF24_Reg_Setup_AW	  _0x03_setup_aw;										// SETUP_AW    - Настройка размера адреса
	NRF24_Reg_Setup_Petr  _0x04_setup_petr; 								// SETUP_RETR  - Настройка повторной отправки
	NRF24_Reg_Rf_Ch				_0x05_rf_ch;											// RF_CH 			 - Частота радиоканала (несущая частота). 2400 + RF_CH(От 0 до 125) МГц
	NRF24_Reg_Rf_Setup		_0x06_rf_setup;										// RF_SETUP 	 - Настройка радиоканала
	NRF24_Reg_Status			_0x07_status;											// STATUS 	   - Регистр статусов
	NRF24_Reg_Observe_Tx	_0x08_observe_tx;									// OBSERVE_TX  - Кол-во повторов передачи и потерянных пакетов
	NRF24_Reg_Cd					_0x09_cd;													// CD 				 - Обнаружение несущей частоты. Если младший бит = 1, то уровень мощности более -64dBm
	u8										_0x0A_rx_addr_p0[NRF24_ADR_SZ_5];	// RX_ADDR_P0  - Адрес канала приёмника №0 (5 байт)
	u8										_0x0B_rx_addr_p1[NRF24_ADR_SZ_5];	// RX_ADDR_P1  - Адрес канала приёмника №1 (5 байт)
	u8										_0x0C_rx_addr_p2[NRF24_ADR_SZ_1];	// RX_ADDR_P2  - Адрес канала приёмника №2 (1 байт)
	u8										_0x0D_rx_addr_p3[NRF24_ADR_SZ_1];	// RX_ADDR_P3  - Адрес канала приёмника №3 (1 байт)
	u8										_0x0E_rx_addr_p4[NRF24_ADR_SZ_1];	// RX_ADDR_P4  - Адрес канала приёмника №4 (1 байт)
	u8										_0x0F_rx_addr_p5[NRF24_ADR_SZ_1];	// RX_ADDR_P5  - Адрес канала приёмника №5 (1 байт)
	u8										_0x10_tx_addr		[NRF24_ADR_SZ_5];	// TX_ADDR 		 - Адрес удалённого устройства для передачи (5 байт)
	NRF24_Reg_Rx_Pw_P0		_0x11_rx_pw_p0;										// RX_PW_P0 	 - Размер данных при приёме по каналу 0
	NRF24_Reg_Rx_Pw_P0		_0x12_rx_pw_p1;										// RX_PW_P1 	 - Размер данных при приёме по каналу 1
	NRF24_Reg_Rx_Pw_P0		_0x13_rx_pw_p2;										// RX_PW_P2 	 - Размер данных при приёме по каналу 2
	NRF24_Reg_Rx_Pw_P0		_0x14_rx_pw_p3;										// RX_PW_P3 	 - Размер данных при приёме по каналу 3
	NRF24_Reg_Rx_Pw_P0		_0x15_rx_pw_p4;										// RX_PW_P4 	 - Размер данных при приёме по каналу 4
	NRF24_Reg_Rx_Pw_P0		_0x16_rx_pw_p5;										// RX_PW_P5 	 - Размер данных при приёме по каналу 5
	NRF24_Reg_Fifo_Stat		_0x17_fifo_stat;									// FIFO_STATUS - Состояние очередей FIFO приёмника и передатчика
	NRF24_Reg_Dynpd				_0x1C_dynpd;											// DYNPD 		   - Выбор каналов приёмника для которых используется произвольная длина пакетов
	NRF24_Reg_Feature			_0x1D_feature;										// FEATURE  	 - Регистр опций 
} NRF24_Struct_Map_Regs;
//-----------------------------------------------------------------------------------------------------------------

volatile typedef struct
{
	// Параметры для работы с SPI
	SPI_HandleTypeDef* 		hspi;													// ссылка на структуру для работы с SPI	
	NRF24_Enum_Mode_Spi 	spi_mode;											// режим обмена данными по шине SPI
	NRF24_Enum_Spi 		 		stat_spi;											// состояния передачи данных по SPI
	u8										bf_spi_rx[NRF24_SBUF_SPI_RX];	// буфер для приема данных по SPI
	u8										bf_spi_tx[NRF24_SBUF_SPI_TX];	// буфер для передачи данных по SPI
	// Диагностика SPI
	u32										cnt_spi_err;									// счетчик ошибок - приема\передачи данных по SPI (HAL_ERROR = 0x01U)
	u32										cnt_spi_err_tmo;							// счетчик ошибок - превышение времени ожидания приема\передачи данных по SPI (HAL_TIMEOUT = 0x03U)	
	
	// Работа с буфером данных для радиоэфира
	u8										bf_radio[NRF24_SBUF_RADIO];		// буфер для приема\передачи данных по радиоэфиру (32 байта)	
	u8										n_bf_radio;										// сколько данных в буфере bf_radio (в байтах)
	u8										n_channel;										// номер канала по которому приняли\передали данные (0-5)
	bool									fl_bf_radio_ready;						// флаг - в буфере есть данные (пора отправлять\читать)
	
	u8										cnt_fifo_tx;									// счетчик заполнения FIFO очереди передатчика радиотрансивера
	u8										cnt_fifo_tx_m;								// макс. значение (настраивается от 1-3)
	u64										cnt_p_tx_ack;									// счетчик успешно отправленных пакетов в радиоэфир (с подтвреждением доставки)
	u64										cnt_loss_p_tx_ack;						// счетчик пакетов, которые не смогли отправить в радиоэфир (с подтвреждением доставки)
	
	// Параметры для работы с радиопередатчиком
	NRF24_Struct_Map_Regs map;													// карта регистров с текущими настройками
	NRF24_Struct_Map_Regs map_s;												// стартовые настройки прочитанные из радиотрансивера
	
	NRF24_Enum_modeTR			mode_tr;											// флаг - режим работы радиотрансивера (false - прием, true - передача)
	NRF24_Enum_mEnergy		mEnergy;											// режим энергопотребления устр-ва (соотношение потребление-скорость обмена)
	
	NRF24_Enum_Alg 		 	 	alg;													// алгоритм работы радиопередатчика (состояния)
	NRF24_Enum_Err  			err;													// коды ошибок
	u64										cnt_err_all;									// счетчик всех видов ошибок (общий)
	
	bool									fl_verify_cfg;								// флаг - совпадает ли тек-ая конф-я с данными из регистров nRF24L01 (true - совпадает)
	bool									fl_alg_restart;								// флаг - начать алгоритм заново	
	
}NRF24_Struct;																				// Структура устр-ва NRF24

/************************************************************************
							Прототипы глобальных переменных модуля
*************************************************************************/
extern NRF24_Struct NRF24_dev;																						// Структура устр-ва nRF24L01

//------------------------------------------------------------------------

/************************************************************************
							 Прототипы глобальных функций модуля
*************************************************************************/
void NRF24_Init(SPI_HandleTypeDef* hspi, NRF24_Enum_Mode_Spi spi_mode);		// Инициализация модуля
void NRF24_Handler(void);		 																							// Обработчик модуля (положить в Main)
void NRF24_Handler_Tm(void);																							// Обработчик интервалов для модуля (положить в таймер 1мс)

bool NRF24_Spi_Tx_Rx     (NRF24_Enum_Mode_Spi spi_mode, u16 size);				// Моя версия передачи данных по SPI
bool NRF24_Write_Command (NRF24_Enum_Cmd cmd, u8* buf, u8 size);					// Записать данные по управляющей команде 
bool NRF24_Write_Data_Reg(u8 reg, u8 size);																// Записать данные в регистр
bool NRF24_Read_Data_Reg (u8 reg, u8 size, bool fl_wr);										// Прочитать данные из регистра

bool NRF24_Write_Data_In_Fifo_Tx(u8* buf, u8 size, NRF24_Enum_Cmd cmd);		// Записать в FIFO_TX очередь радиотрансивера данные для отправки	
bool NRF24_Read_Data_In_Fifo_Rx	(u8* buf, u8 size);												// Прочитать из FIFO_RX очереди радиотрансивера принятые данные 		
bool NRF24_Read_Size_Rx_Pack		(void);																		// Прочитать размер принятого пакета

bool NRF24_Check_Connection_Spi	 (void);																	// Проверить работоспособность связи по SPI с радиотрансивером
void NRF24_Set_Default_Settings	 (void);																	// Задать конф-ю "NRF24_dev.map" радиотрансивера "по умолчанию"
bool NRF24_Write_Current_Settings(void);																	// Записать текущую конф-ю из "NRF24_dev.map" в регистры NRF24
bool NRF24_Verification_Settings (void);																	// Верификация настроек. Прочитать и проверить все регистры конф-и NRF24 с текущей NRF24_dev.map
//------------------------------------------------------------------------

#endif
