#include "hal_nrf24l01_api.h"
#include "hal_nrf24l01_drv.h"

/***************************************************************************
										Объявляем переменные внутри модуля
****************************************************************************/

//--------------------------------------------------------------------------

/***************************************************************************
											Прототипы всех функций модуля
****************************************************************************/
void NRF24_Stream_Handler(void);																							// Обработчик для потока ОС (выполняется основной алгоритм радиотрансивера) 
void NRF24_Stream_Handler_Err(void);																					// Обработчик ошибок трансивера
void NRF24_Stream_Handler_Rx(void);																						// Обработчик для потока ОС (выполняется алгоритм приема данных из радиоэфира)
void NRF24_Stream_Handler_Tx(void);																						// Обработчик для потока ОС (выполняется алгоритм приема данных из радиоэфира)

void NRF24_Handle_Mode_Low_Enerdgy_Rx(void);																	// Обработчик алгоритма работы приема данных радиотрансивера ("низкий" уровень энергопотребления)
void NRF24_Handle_Mode_Midle_Enerdgy_Rx(void);																// Обработчик алгоритма работы приема данных радиотрансивера ("средний" уровень энергопотребления)
void NRF24_Handle_Mode_Hight_Enerdgy_Rx(void);																// Обработчик алгоритма работы приема данных радиотрансивера ("высокий" уровень энергопотребления)

void NRF24_Handle_Mode_Low_Enerdgy_Tx(void);																	// Обработчик алгоритма работы передачи данных радиотрансивера ("низкий"  уровень энергопотребления)
void NRF24_Handle_Mode_Midle_Enerdgy_Tx(void);																// Обработчик алгоритма работы передачи данных радиотрансивера ("средний" уровень энергопотребления)
void NRF24_Handle_Mode_Hight_Enerdgy_Tx(void);																// Обработчик алгоритма работы передачи данных радиотрансивера ("высокий" уровень энергопотребления)

void NRF24_Go_To_Standby_1(bool fl_rtx);																			// Перевести радиотрансивер в режим "ожидания-I"
bool NRF24_Go_To_Standby_2(void);																							// Перевести радиотрансивер в режим "ожидания-II"

void NRF24_Set_Pin_Mode_Active(bool fl_active);																// Перевести радиотрансивер в активный\пасивный режим контактом управления

bool NRF24_Set_Setup_Power(bool stat);																				// Вкл.\выкл. питание радиотрансивера (для снижения энергопотребления)
bool NRF24_Set_Setup_Mode_Rtx(bool fl_rtx);																		// Задать в настройках радиотрансивера режим прием\передача данных
bool NRF24_Set_Setup_Frequency(u8 mhz);																				// Задать частоту приема\передачи данных по радиоэфиру
bool NRF24_Set_Setup_Speed(NRF24_Enum_Speed speed);														// Задать скорость приемо/передачи данных по радиоэфиру
bool NRF24_Set_Setup_Gain(NRF24_Enum_Gain gain);															// Задать мощность передатчика (0dBm - самая высокая мощность сигнала)
	
bool NRF24_Get_Observe_Tx(void);																							// Получить значение регистра счетчиков перезапросов и потерянных пакетов
bool NRF24_Get_Carrier_Detect(void);																					// Получить значение регистра обнаружение несущей частоты
bool NRF24_Get_Status(void);																									// Получить значение регистра состояния радиотрансивера
bool NRF24_Get_Fifo_Status(void);																							// Получить значение регистра состояния FIFO очереди радиотрансивера

bool NRF24_Clear_Fifo_Tx(void);																								// Очистить очередь передатчика радиотрансивера
bool NRF24_Clear_Fifo_Rx(void);																								// Очистить очередь приемника радиотрансивера
bool NRF24_Clear_Flags_Reg_Status(bool rx_dr, bool tx_ds, bool max_rt);				// Очистить заданный(е) биты регистра Status (путем записи туда 1)

bool NRF24_Write_Data_In_Queue_Tx(u8* buf, u8 size);													// Записать данные в очередь передатчика радиотрансивера
bool NRF24_Write_Data_In_Queue_Tx_Without_Confirm_Package(u8* buf, u8 size);	// Записать данные в очередь радиотрансивера без пакета подтверждения
bool NRF24_Write_Data_In_Queue_Tx_With_Confirm_Package(u8* buf, u8 size);			// Записать данные в очередь радиотрансивера с пакетом подтверждения

void NRF24_Callback_Process_BufRx(u8* bf_radio, u8 n_channel);								// Обработать принятые данные из FIFO RX радиотрансивера
void NRF24_Callback_Process_BufTx(u8* bf_radio, u8 n_channel);								// Добавить данные в FIFO TX радиотрансивера
void NRF24_Callback_Tx_Completed(u8 n_channel);																// Завершена передача данных радиотрансивером
//--------------------------------------------------------------------------

/***************************************************************************
											Описание функций и методов модуля
****************************************************************************/
/**
	* @brief Обработчик для потока ОС (выполняется основной алгоритм радиотрансивера) 
  */
void NRF24_Stream_Handler(void)
{
	if(NRF24_dev.fl_verify_cfg)
	{
		NRF24_Stream_Handler_Err();
		
		if(!NRF24_dev.err)
		{
			NRF24_Stream_Handler_Rx();
			NRF24_Stream_Handler_Tx();		
		}
	}
}

/**
	* @brief Обработчик ошибок трансивера (Обработчик для потока ОС)
  */
void NRF24_Stream_Handler_Err(void)
{
	if(NRF24_dev.err)
	{
		// ??? 
		NRF24_dev.cnt_err_all++;
		NRF24_dev.err = NRF24_ERR_NONE;
	}
}

/**
	* @brief Выполняется алгоритм приема данных из радиоэфира (Обработчик для потока ОС) 
  */
void NRF24_Stream_Handler_Rx(void)
{
	if(NRF24_dev.mode_tr == NRF24_MODE_RADIO_RX || 
		 NRF24_dev.mode_tr == NRF24_MODE_RADIO_RX_TX)
	{
		if(NRF24_dev.mEnergy == NRF24_MODE_ENERGY_HIGH)
			NRF24_Handle_Mode_Hight_Enerdgy_Rx();
		else	
		if(NRF24_dev.mEnergy == NRF24_MODE_ENERGY_MIDLE)
			NRF24_Handle_Mode_Midle_Enerdgy_Rx();											
		else	
		if(NRF24_dev.mEnergy == NRF24_MODE_ENERGY_LOW)
			NRF24_Handle_Mode_Low_Enerdgy_Rx();	
	}
}

/**
	* @brief Выполняется алгоритм передачи данных в радиоэфир (Обработчик для потока ОС)
  */
void NRF24_Stream_Handler_Tx(void)
{
	if(NRF24_dev.mode_tr == NRF24_MODE_RADIO_TX || 
		 NRF24_dev.mode_tr == NRF24_MODE_RADIO_RX_TX)
	{
		if(NRF24_dev.mEnergy == NRF24_MODE_ENERGY_HIGH)
			NRF24_Handle_Mode_Hight_Enerdgy_Tx();
		else	
		if(NRF24_dev.mEnergy == NRF24_MODE_ENERGY_MIDLE)
			NRF24_Handle_Mode_Midle_Enerdgy_Tx();											
		else	
		if(NRF24_dev.mEnergy == NRF24_MODE_ENERGY_LOW)
			NRF24_Handle_Mode_Low_Enerdgy_Tx();
	}
}

/**
	* @brief Обработчик алгоритма работы приема данных радиотрансивера ("низкий" уровень энергопотребления)
  */
void NRF24_Handle_Mode_Low_Enerdgy_Rx(void)
{
	
}

/**
	* @brief Обработчик алгоритма работы приема данных радиотрансивера ("средний" уровень энергопотребления)
  */
void NRF24_Handle_Mode_Midle_Enerdgy_Rx(void)
{
	
}

/**
	* @brief Обработчик алгоритма работы приема данных радиотрансивера ("высокий" уровень энергопотребления)
  */
void NRF24_Handle_Mode_Hight_Enerdgy_Rx(void)
{
	if(NRF24_dev.alg == NRF24_ALG_INIT_OK)											
		NRF24_dev.alg = NRF24_ALG_STANDBY_1;
																															
	if(NRF24_dev.alg == NRF24_ALG_STANDBY_1)										
		NRF24_Go_To_Standby_1(NRF24_MODE_RX);
																															
	if(NRF24_dev.alg == NRF24_ALG_STANDBY_1_OK)	
	{
		NRF24_Set_Pin_Mode_Active(NRF24_MODE_ACTIVE);
		NRF24_dev.alg = NRF24_ALG_IDLE_RX_DATA;						
	}
	if(NRF24_dev.alg == NRF24_ALG_IDLE_RX_DATA)	
		if(NRF24_dev.map._0x00_cfg.bit._6_mask_rx_rd)						// Если стоит запрет приема через прерывание (контакт IRQ)
			if(NRF24_Get_Fifo_Status())														// Опросить регистр	0x17, чтобы узнать если ли в очереди FIFO принятые данные
				if(!NRF24_dev.map._0x17_fifo_stat.bit._0_rx_empty)		
					NRF24_dev.alg = NRF24_ALG_FIFO_RX_READ_DATA;
	
	if(NRF24_dev.alg == NRF24_ALG_FIFO_RX_READ_DATA || 
		 NRF24_dev.alg == NRF24_ALG_IRQ)					
	{	
		if(NRF24_Read_Size_Rx_Pack())														// Получаем значение NRF24_dev.n_bf_radio							
		{
			if(NRF24_dev.n_bf_radio <= NRF24_SBUF_RADIO)
			{
				if(NRF24_Get_Status())
					NRF24_dev.n_channel = NRF24_dev.map._0x07_status.bit._1_3_rx_p_no;
				if(NRF24_Read_Data_In_Fifo_Rx((u8*)NRF24_dev.bf_radio, NRF24_dev.n_bf_radio))
					NRF24_dev.alg = NRF24_ALG_FIFO_RX_READ_DATA_OK;
			}
			else
				NRF24_dev.err = NRF24_ERR_SIZE_RX_DATA;
		}
		if(NRF24_Clear_Fifo_Rx())
			if(NRF24_dev.alg != NRF24_ALG_FIFO_RX_READ_DATA_OK)
				NRF24_dev.alg = NRF24_ALG_IDLE_RX_DATA;
	}
	if(NRF24_dev.alg == NRF24_ALG_FIFO_RX_READ_DATA_OK)					
	{
		NRF24_Callback_Process_BufRx((u8*)NRF24_dev.bf_radio, NRF24_dev.n_channel);
		NRF24_dev.alg = NRF24_ALG_IDLE_RX_DATA;
//		NRF24_dev.alg = NRF24_ALG_STANDBY_1_OK;
	}	
}

/**
	* @brief Обработчик алгоритма работы передачи данных радиотрансивера ("низкий" уровень энергопотребления)
  */
void NRF24_Handle_Mode_Low_Enerdgy_Tx(void)
{
	
}

/**
	* @brief Обработчик алгоритма работы передачи данных радиотрансивера ("средний" уровень энергопотребления)
  */
void NRF24_Handle_Mode_Midle_Enerdgy_Tx(void)
{
	
}

/**
	* @brief Обработчик алгоритма работы передачи данных радиотрансивера ("высокий" уровень энергопотребления)
  */
void NRF24_Handle_Mode_Hight_Enerdgy_Tx(void)
{
	if(NRF24_dev.alg == NRF24_ALG_INIT_OK)											// 0. Иниц-я и верификация настроек прошла успешна, радиотрансивер на связи (по SPI)					
		NRF24_dev.alg = NRF24_ALG_STANDBY_1;											//---------------------------------------------------------																											
	
	if(NRF24_dev.alg == NRF24_ALG_STANDBY_1)										// 1. Вкл. питание и настроить режим передачи								
		NRF24_Go_To_Standby_1(NRF24_MODE_TX);											// переход в STANDBY_1 -> PRIM_RX = 0, CE = 0, FIFO_TX empty
	
	if(NRF24_dev.alg == NRF24_ALG_STANDBY_1_OK)
		if(NRF24_Get_Fifo_Status())																// Получить значение регистра состояния FIFO очереди радиотрансивера
		{
			if(NRF24_dev.map._0x17_fifo_stat.bit._4_tx_empty)				// FIFO_TX empty
				NRF24_dev.alg = NRF24_ALG_STANDBY_2;
			else
				NRF24_dev.alg = NRF24_ALG_WAIT_TX_DATA_RADIO;
			
			NRF24_Set_Pin_Mode_Active(NRF24_MODE_ACTIVE);
		}																													//---------------------------------------------------------								
	
	if(NRF24_dev.alg == NRF24_ALG_STANDBY_2)										// 2. Вкл. активный режим радиотрансивера, переход в STANDBY_2 -> PRIM_RX = 0, CE = 1, FIFO_TX empty						
		NRF24_dev.alg = NRF24_ALG_STANDBY_2_OK;										//---------------------------------------------------------
	
	if(NRF24_dev.alg == NRF24_ALG_STANDBY_2_OK)									// 3. Ждем пока в промежуточный буфер(NRF24_dev.bf_radio) запишут данные 																										
		if(NRF24_Get_Fifo_Status())																// Получить значение регистра состояния FIFO очереди радиотрансивера
			if(NRF24_dev.map._0x17_fifo_stat.bit._4_tx_empty)				// FIFO_TX empty
				if(NRF24_dev.fl_bf_radio_ready)												// флаг - в промежуточный буфер записали данные для отправки
					NRF24_dev.alg = NRF24_ALG_FIFO_TX_WRITE_DATA;				//---------------------------------------------------------
				
	if(NRF24_dev.alg == NRF24_ALG_FIFO_TX_WRITE_DATA)						// 4. Переписать данные из промежуточного буфера в FIFO_TX
		if(NRF24_Get_Fifo_Status())																// Получить значение регистра состояния FIFO очереди радиотрансивера
			if(!NRF24_dev.map._0x17_fifo_stat.bit._5_tx_full)				// пишем пакеты в FIFO_TX очередь, пока она не заполнится (макс. 3 пакета)
				if(NRF24_dev.fl_bf_radio_ready)												// в промежуточный буфер(NRF24_dev.bf_radio) записаны данные для отправки
				{
					if(!NRF24_Write_Data_In_Queue_Tx((u8*)&NRF24_dev.bf_radio, NRF24_dev.n_bf_radio))	// Записать данные в FIFO_TX радиотрансивера
						return;
					else
					{
						NRF24_Get_Status();
						NRF24_Get_Fifo_Status();
						NRF24_dev.alg = NRF24_ALG_FIFO_TX_WRITE_DATA_OK;
					}
					NRF24_dev.fl_bf_radio_ready = false;					
				}																										
	if(NRF24_dev.alg == NRF24_ALG_FIFO_TX_WRITE_DATA_OK)				
		NRF24_dev.alg = NRF24_ALG_WAIT_TX_DATA_RADIO;							//---------------------------------------------------------
	
	// !!! Не рекомендуется находится в режиме передачи более 4мс
	// !!! по даташиту стр.21  Пункт 6.1.5 TX mode
	if(NRF24_dev.alg == NRF24_ALG_WAIT_TX_DATA_RADIO)						// 5. Ожидание передачи пакета из FIFO_TX в радиоэфир
		if(NRF24_Get_Status())																		// Получить значение регистра состояния радиотрансивера
		{
			if(NRF24_dev.map._0x04_setup_petr.bit._0_3_arc)					// Если настроены повторы пакетов передачи данных
				if(NRF24_dev.map._0x07_status.bit._4_max_rt)					// Превышено установленное число повторов
				{
					if(NRF24_Clear_Fifo_Tx())														// Очистить очередь передатчика радиотрансивера
						if(NRF24_Clear_Flags_Reg_Status(true, true, true))// Без сброса дальнейшая TX невозможа						
						{
							NRF24_dev.alg = NRF24_ALG_WAIT_TX_DATA_RADIO_OK;
							NRF24_dev.cnt_fifo_tx = 0;						
						}
					NRF24_dev.cnt_loss_p_tx_ack++;						
				}
			if(NRF24_dev.alg == NRF24_ALG_WAIT_TX_DATA_RADIO)				
				if(NRF24_dev.map._0x07_status.bit._5_tx_ds)						 // Пакет успешно передан
					if(NRF24_Clear_Flags_Reg_Status(true, true, true))		
						if(NRF24_Get_Fifo_Status())												 // Получить значение регистра состояния FIFO очереди радиотрансивера
							if(NRF24_dev.map._0x17_fifo_stat.bit._4_tx_empty)// FIFO очередь TX пуста, значит находимся в режиме STANDBY_2 (CE = 1)
							{
								NRF24_dev.alg = NRF24_ALG_WAIT_TX_DATA_RADIO_OK;
								NRF24_dev.cnt_fifo_tx = 0;
								NRF24_dev.cnt_p_tx_ack++;									
							}
			NRF24_Get_Status();																			// чтобы в регистре были актуальные флаги (после NRF24_Clear_Flags_Reg_Status)
		}																													//---------------------------------------------------------
	if(NRF24_dev.alg == NRF24_ALG_WAIT_TX_DATA_RADIO_OK)				// 8. Передача пакета из FIFO_TX в радиоэфир успешно завершена
		NRF24_dev.alg = NRF24_ALG_STANDBY_1;											//---------------------------------------------------------	
}

/**
	* @brief 					Перевести радиотрансивер в режим "ожидание-I"
										Этот режим используется для минимизации среднего потребления тока, при сохранении короткого времени пуска.
	* @param 	fl_rtx: Флаг - режим работы радиотрансивера (0 - передатчик; 1 - приёмник)	
  */
void NRF24_Go_To_Standby_1(bool fl_rtx)
{
	// PWR_UP = 1 (Вкл. питание)
	// CE = 0     (Вкл. пассивный режим приемника\передатчика)
	
	NRF24_Set_Pin_Mode_Active(NRF24_MODE_PASSIVE); 				
	
	if(!NRF24_Set_Setup_Mode_Rtx(fl_rtx))
	{
		if(fl_rtx == NRF24_MODE_RX)
			NRF24_dev.err = NRF24_ERR_POWER_ON_RX;
		else
			NRF24_dev.err = NRF24_ERR_POWER_ON_TX;
	}
	else
		NRF24_dev.alg = NRF24_ALG_STANDBY_1_OK;
}

/**
	* @brief Перевести радиотрансивер в режим "ожидания-II"
  */
bool NRF24_Go_To_Standby_2(void)
{
	// PWR_UP = 1 		(Вкл. питание)
	// CE = 1    		  (Вкл. активный режим приемника\передатчика)
	// TX_FIFO empty	(буфер TX пуст)
	
	if(NRF24_Set_Setup_Mode_Rtx(NRF24_MODE_RX))
	 return	true;
	else
	 return	false;
}

/**
	* @brief 					  Перевести радиотрансивер в активный\пасивный режим контактом управления
									    (прием\передача в зависимости от режима работы в настройках бита PRIM_RX)
	* @param fl_active: Результат операции (true - приемник)
  */
void NRF24_Set_Pin_Mode_Active(bool fl_active)
{
	if(fl_active)
		NRF24_CE_HI();
	else
		NRF24_CE_LO();
}

/**
	* @brief 			  Вкл.\выкл. питание радиотрансивера (для снижения энергопотребления)
	* @retval stat: Состояние радиотрансивера (true - вкл.)
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Set_Setup_Power(bool stat)
{
	NRF24_dev.map._0x00_cfg.bit._1_pwr_up = stat;	
	
	if(NRF24_Write_Data_Reg(NRF24_REG_00_CONFIG, 2))
		if(NRF24_Read_Data_Reg(NRF24_REG_00_CONFIG, 2, false))
			if(NRF24_dev.map._0x00_cfg.full == NRF24_dev.bf_spi_rx[NRF24_ISD])
			{
				if(NRF24_dev.spi_mode == NRF24_TYPE_RX_TX_WHILE)
					if(stat)
						HAL_Delay(5);																														// по даташиту от 1.5-4.5мс (в зависимости от индуктивности резонатора)
				
				return true;
			}
	return false;
}

/**
	* @brief 					Задать в настройках NRF24 режим приема данных
	* @param 	fl_rtx: Флаг - режим работы радиотрансивера (0 - передатчик; 1 - приёмник)	
	* @retval 	bool: Результат операции (true - успешно)
  */
bool NRF24_Set_Setup_Mode_Rtx(bool fl_rtx)
{
	NRF24_dev.map._0x00_cfg.bit._0_prim_rx = fl_rtx;																		
	return NRF24_Set_Setup_Power(true);
}

/**
	* @brief 			   Задать частоту приема\передачи данных по радиоэфиру
	* @param 		mhz: Частота смещения (2400GHz + speed(от 0 до 125 MHZ)
	* @retval  bool: Результат операции (true - успешно)
  */
bool NRF24_Set_Setup_Frequency(u8 mhz)
{
	if(mhz > NRF24_OFFSET_HZ_MAX)
		return false;
	
	NRF24_dev.map._0x05_rf_ch.bit._0_6_rfch = mhz;														
			
	if(NRF24_Write_Data_Reg(NRF24_REG_05_RF_CH,2))	
		if(NRF24_Read_Data_Reg(NRF24_REG_05_RF_CH, 2, false))
			if(NRF24_dev.map._0x05_rf_ch.full == NRF24_dev.bf_spi_rx[NRF24_ISD])
				return true;
	
	return false;
}

/**
	* @brief 			   Задать скорость приемо/передачи данных по радиоэфиру
	* @param 	speed: Состояние радиотрансивера (true - вкл.)
	* @retval  bool: Результат операции (true - успешно)
  */
bool NRF24_Set_Setup_Speed(NRF24_Enum_Speed speed)
{
	if(speed > NRF24_SPEED_250Kb)
		return false;
	
	if(speed == NRF24_SPEED_2Mb)
	{
		NRF24_dev.map._0x06_rf_setup.bit._3_rf_dr_hight = true;													// вкл. высокую скорость передачи данных (при значении бита RF_DR_LOW = 0: 0 - 1Мбит/с; 1 - 2Мбит/с)
		NRF24_dev.map._0x06_rf_setup.bit._5_rf_dr_low 	= false;												// вкл. низкую скорость передачи данных 250кбит/с
	}
	else
	if(speed == NRF24_SPEED_1Mb)
	{
		NRF24_dev.map._0x06_rf_setup.bit._3_rf_dr_hight = false;										
		NRF24_dev.map._0x06_rf_setup.bit._5_rf_dr_low 	= false;												
	}
	else
	if(speed == NRF24_SPEED_250Kb)
	{
//		NRF24_dev.map._0x06_rf_setup.bit._3_rf_dr_hight - данный параметр не учитывается (любое значение)											
		NRF24_dev.map._0x06_rf_setup.bit._5_rf_dr_low 	= true;												
	}		
	if(NRF24_Write_Data_Reg(NRF24_REG_06_RF_SETUP,2))
		if(NRF24_Read_Data_Reg(NRF24_REG_06_RF_SETUP, 2, false))
			if(NRF24_dev.map._0x06_rf_setup.full == NRF24_dev.bf_spi_rx[NRF24_ISD])
				return true;
	
	return false;
}

/**
	* @brief 				Задать мощность радиотрансивера (0dBm - самая высокая мощность сигнала)
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Set_Setup_Gain(NRF24_Enum_Gain gain)
{
	NRF24_dev.map._0x06_rf_setup.bit._1_2_rf_pwr = gain;	
	
	if(NRF24_Write_Data_Reg(NRF24_REG_06_RF_SETUP,2))	
		if(NRF24_Read_Data_Reg(NRF24_REG_06_RF_SETUP, 2, false))
			if(NRF24_dev.map._0x06_rf_setup.full == NRF24_dev.bf_spi_rx[NRF24_ISD])
				return true;
	
	return false;
}

/**
	* @brief 			  Получить значение регистра счетчиков перезапросов и потерянных пакетов
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Get_Observe_Tx(void)
{
	return NRF24_Read_Data_Reg(NRF24_REG_08_OBSERVE_TX, 2, true);
}

/**
	* @brief 			  Получить значение регистра обнаружение несущей частоты
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Get_Carrier_Detect(void)
{
	return NRF24_Read_Data_Reg(NRF24_REG_09_CD, 2, true);
}

/**
	* @brief 			  Получить значение регистра состояния радиотрансивера
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Get_Status(void)
{
	if(NRF24_Read_Data_Reg(NRF24_REG_07_STATUS, 2, true))	
		return true;
	else
	{
		NRF24_dev.err = NRF24_ERR_GET_REG_STAT;
		return false;
	}
}

/**
	* @brief 			  Получить значение регистра состояния FIFO очереди радиотрансивера
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Get_Fifo_Status(void)
{
	if(NRF24_Read_Data_Reg(NRF24_REG_17_FIFO_STATUS, 2, true))	
		return true;
	else
	{
		NRF24_dev.err = NRF24_ERR_GET_REG_STAT_FIFO;
		return false;
	}
}

/**
	* @brief 					Очистить заданный(е) биты регистра Status (путем записи туда 1)
	* @param 	 rx_dr: R\W Флаг получения новых данных в FIFO приёмника
	* @param 	 tx_ds: R\W Флаг завершения передачи
	* @param 	max_rt: R\W Флаг превышено установленное число повторов. Без сброса дальнейшая TX невозможа	
	* @retval 	bool: Результат операции (true - успешно)
  */
bool NRF24_Clear_Flags_Reg_Status(bool rx_dr, bool tx_ds, bool max_rt)
{
	NRF24_dev.map._0x07_status.bit._6_rx_dr  = rx_dr;
	NRF24_dev.map._0x07_status.bit._5_tx_ds  = tx_ds;
	NRF24_dev.map._0x07_status.bit._4_max_rt = max_rt;
	
	if(NRF24_Write_Data_Reg(NRF24_REG_07_STATUS,2))	
		return true;
	else
	{
		NRF24_dev.err = NRF24_ERR_CLEAR_FLAGS_REG_STAT;
		return false;
	}
}

/**
	* @brief 			  Записать данные в очередь передатчика радиотрансивера
	* @param   buf: Ссылка на массив с данными
  * @param  size: Кол-во байт на прием\передачу
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Write_Data_In_Queue_Tx(u8* buf, u8 size)
{
	bool stat = false;
	
	if(NRF24_dev.cnt_fifo_tx < NRF24_dev.cnt_fifo_tx_m) 
	{
		// разрешает передавать пакеты, не требующие подтверждения приёма
		if(NRF24_dev.map._0x1D_feature.bit._0_en_dyn_ack)		
			stat = NRF24_Write_Data_In_Queue_Tx_Without_Confirm_Package(buf, size);
		else
			stat = NRF24_Write_Data_In_Queue_Tx_With_Confirm_Package(buf, size);
		
		NRF24_dev.cnt_fifo_tx++;
	}
	return stat;
}

/**
	* @brief 			  Записать данные в очередь радиотрансивера без пакета подтверждения
	* @param   buf: Ссылка на массив с данными
  * @param  size: Кол-во байт на прием\передачу
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Write_Data_In_Queue_Tx_Without_Confirm_Package(u8* buf, u8 size)
{
	if(NRF24_Write_Data_In_Fifo_Tx(buf, size, NRF24_CMD_W_TX_PLD_NOACK))	
		return true;
	else		
	{
		NRF24_dev.err = NRF24_ERR_WR_FIFO_TX_NO_ACK;
		return false;
	}
}

/**
	* @brief 			  Записать данные в очередь радиотрансивера с пакетом подтверждения
	* @param   buf: Ссылка на массив с данными
  * @param  size: Кол-во байт на прием\передачу
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Write_Data_In_Queue_Tx_With_Confirm_Package(u8* buf, u8 size)
{
	if(NRF24_Write_Data_In_Fifo_Tx(buf, size, NRF24_CMD_W_TX_PLD))
		return true;
	else	
	{
		NRF24_dev.err = NRF24_ERR_WR_FIFO_TX_ACK;
		return false;
	}
}

/**
	* @brief 				Очистить очередь передатчика радиотрансивера
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Clear_Fifo_Tx(void)
{
	NRF24_dev.bf_spi_tx[0] = NRF24_CMD_FLUSH_TX;
	NRF24_dev.bf_spi_tx[1] = NRF24_CMD_NOP;
	
	if(NRF24_Spi_Tx_Rx(NRF24_dev.spi_mode, 2))
		return true;
	else
	{
		NRF24_dev.err = NRF24_ERR_CLEAR_FIFO_TX;
		return false;
	}
}

/**
	* @brief 				Очистить очередь приемника радиотрансивера
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Clear_Fifo_Rx(void)
{
	NRF24_dev.bf_spi_tx[0] = NRF24_CMD_FLUSH_RX;
	NRF24_dev.bf_spi_tx[1] = NRF24_CMD_NOP;
	
	if(NRF24_Spi_Tx_Rx(NRF24_dev.spi_mode, 2))
		return true;
	else
	{
		NRF24_dev.err = NRF24_ERR_CLEAR_FIFO_RX;
		return false;
	}
}

/**
	* @brief 						 Обработать принятые данные из FIFO RX радиотрансивера
	* @param   bf_radio: Масссив с принятыми данными
	* @param  n_channel: Номер канала по которому приняли данные (0-5)
  */
__weak void NRF24_Callback_Process_BufRx(u8* bf_radio, u8 n_channel)
{
  // Обработать данные
}

/**
	* @brief 						 Добавить данные в FIFO TX радиотрансивера
	* @param   bf_radio: Масссив с принятыми данными
	* @param  n_channel: Номер канала по которому приняли данные (0-5)
  */
__weak void NRF24_Callback_Process_BufTx(u8* bf_radio, u8 n_channel)
{
  // Добавить данные в буфер для отправки
}

/**
	* @brief 						 Завершена передача данных радиотрансивером
	* @param  n_channel: Номер канала по которому передавали данные (0-5)
  */
__weak void NRF24_Callback_Tx_Completed(u8 n_channel) 
{
  
}
