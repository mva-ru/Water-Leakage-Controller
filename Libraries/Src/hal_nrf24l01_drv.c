#include "hal_nrf24l01_drv.h"

/***************************************************************************
										Объявляем переменные внутри модуля
****************************************************************************/
NRF24_Struct NRF24_dev;																											// Структура устр-ва nRF24L01
//--------------------------------------------------------------------------

/***************************************************************************
											Прототипы всех функций модуля
****************************************************************************/
void NRF24_Init(SPI_HandleTypeDef* hspi, NRF24_Enum_Mode_Spi spi_mode);			// Инициализация модуля
void NRF24_Handler(void);		 																								// Обработчик модуля (положить в Main)
void NRF24_Handler_Tm(void);																								// Обработчик интервалов для модуля (положить в таймер 1мс)
	
bool NRF24_Spi_Tx_Rx(NRF24_Enum_Mode_Spi spi_mode, u16 size);								// Моя версия передачи данных по SPI
bool NRF24_Write_Command(NRF24_Enum_Cmd cmd, u8* buf, u8 size);							// Записать данные по управляющей команде 
bool NRF24_Write_Data_Reg(u8 reg, u8 size);																	// Записать данные в регистр
bool NRF24_Read_Data_Reg(u8 reg, u8 size, bool fl_wr);											// Прочитать данные из регистра
bool NRF24_Write_Data_In_Buf(u8 reg);																				// Записать данные регистра в массив, для последующей записи в регистр устр-ва 
bool NRF24_Read_Data_In_Buf_And_Write_Reg(u8 reg);													// Переписать принятые данные из буфера в регистр

bool NRF24_Write_Data_In_Fifo_Tx(u8* buf, u8 size, NRF24_Enum_Cmd cmd);			// Записать в FIFO_TX очередь радиотрансивера данные для отправки	
bool NRF24_Read_Data_In_Fifo_Rx(u8* buf, u8 size);													// Прочитать из FIFO_RX очереди радиотрансивера принятые данные 		
bool NRF24_Read_Size_Rx_Pack(void);																					// Прочитать размер принятого пакета

bool NRF24_Check_Connection_Spi(void);																			// Проверить работоспособность связи по SPI с радиотрансивером
void NRF24_Set_Default_Settings(void);																			// Задать конф-ю "NRF24_dev.map" радиотрансивера "по умолчанию"
bool NRF24_Write_Current_Settings(void);																		// Записать текущую конф-ю из "NRF24_dev.map" в радиотрансивер
bool NRF24_Read_Current_Settings(void);																			// Прочитать текущую конф-ю из радиотрансивера
bool NRF24_Verification_Settings(void);																			// Верификация настроек. Прочитать и проверить все регистры конф-и NRF24 с текущей NRF24_dev.map
//--------------------------------------------------------------------------

/***************************************************************************
													Описание функций модуля
****************************************************************************/
/**
	* @brief 						Инициализация модуля
	* @param  	 *hspi: Ссылка на структуру для работы с SPI
  * @param  mode_spi: Метод обмена данными (IT, DMA, WHILE)
  */
void NRF24_Init(SPI_HandleTypeDef* hspi, NRF24_Enum_Mode_Spi spi_mode)
{
	NRF24_CSN_HI();
	NRF24_CE_LO();
	HAL_Delay(15);																															// по даташиту 10.5 мс
	
	NRF24_dev.hspi 		 = hspi;
	NRF24_dev.spi_mode = NRF24_TYPE_RX_TX_WHILE;																// пока идет иниц-я прием\передача по SPI в цикле															
	NRF24_dev.stat_spi = NRF24_STAT_SPI_TXRX_NONE;
	NRF24_dev.alg 	 	 = NRF24_ALG_INIT;
	NRF24_dev.mEnergy  = NRF24_MODE_ENERGY_HIGH;
	NRF24_dev.err  		 = NRF24_ERR_NONE;
	NRF24_dev.mode_tr  = NRF24_MODE_RADIO_RX;
	
	NRF24_dev.cnt_fifo_tx = 0;																									// счетчик заполнения FIFO очереди передатчика радиотрансивера
	NRF24_dev.cnt_fifo_tx_m = 1;																								// макс. значение (!!! настраивается от 1-3)
	
	NRF24_dev.cnt_loss_p_tx_ack = 0;
	NRF24_dev.cnt_err_all = 0;
	
	NRF24_dev.n_bf_radio = 0;									
	NRF24_dev.fl_bf_radio_ready = false;	
	NRF24_dev.fl_verify_cfg = false;	
	
	for(u8 i = 0; i < NRF24_SBUF_SPI_RX; i++)
		NRF24_dev.bf_spi_rx[i] = 0;
	for(u8 i = 0; i < NRF24_SBUF_SPI_TX; i++)
		NRF24_dev.bf_spi_tx[i] = 0;
	for(u8 i = 0; i < NRF24_SBUF_RADIO; i++)
		NRF24_dev.bf_radio[i] = 0;
	
  NRF24_Set_Default_Settings();

	if(NRF24_Check_Connection_Spi())
	{
		NRF24_Read_Current_Settings();
		
		if(NRF24_Write_Current_Settings())																
		{
			if(!NRF24_Verification_Settings())
				NRF24_dev.err = NRF24_ERR_COMPARE_CFG;
			else
			{
				NRF24_dev.fl_verify_cfg = true;
				NRF24_dev.alg	= NRF24_ALG_INIT_OK;
			}
		}
		else
			NRF24_dev.err = NRF24_ERR_WRITE_SETTINGS;
	}
	else
		NRF24_dev.err = NRF24_ERR_CONNECT_SPI;
		
	NRF24_dev.spi_mode = spi_mode;
}

/**
	* @brief Обработчик модуля (положить в Main)
  */
void NRF24_Handler(void)
{
	
}

/**
	* @brief Обработчик интервалов для модуля (положить в таймер 1мс)
  */
void NRF24_Handler_Tm(void)
{
	
}

/**
	* @brief 			      Одновременный прием\передача данных по SPI
  * @param      size: Кол-во передаваемых байт из буфера (NRF24_dev.bf_spi_tx)
  * @param  mode_spi: Метод обмена данными (IT, DMA, WHILE)
	* @retval 	  bool: Результат операции (true - успешно)
  */
bool NRF24_Spi_Tx_Rx(NRF24_Enum_Mode_Spi spi_mode, u16 size)
{
	static bool fl_state; fl_state = false;											// Состояние операции приема\передачи
	
	if(NRF24_dev.hspi->State == HAL_SPI_STATE_READY)
	{
		NRF24_CSN_LO();
		NRF24_dev.stat_spi = NRF24_STAT_SPI_TXRX_IDLE;
		
		if(spi_mode == NRF24_TYPE_RX_TX_WHILE)
		{
			if(HAL_SPI_TransmitReceive(NRF24_dev.hspi, (u8*)NRF24_dev.bf_spi_tx, (u8*)NRF24_dev.bf_spi_rx, size, NRF24_TMO_TXRX_WHILE) == HAL_OK)
				fl_state = true;
		}
		else
		if(spi_mode == NRF24_TYPE_RX_TX_IT)
		{
			if(HAL_SPI_TransmitReceive_IT(NRF24_dev.hspi, (u8*)NRF24_dev.bf_spi_tx, (u8*)NRF24_dev.bf_spi_rx, size) == HAL_OK)
				fl_state = true;
		}
		else
		if(spi_mode == NRF24_TYPE_RX_TX_DMA)
			if(HAL_SPI_TransmitReceive_DMA(NRF24_dev.hspi, (u8*)NRF24_dev.bf_spi_tx, (u8*)NRF24_dev.bf_spi_rx, size) == HAL_OK)
				fl_state = true;
			
		NRF24_CSN_HI();
			
		if(!fl_state)
		{
			NRF24_dev.stat_spi = NRF24_STAT_SPI_TXRX_ERR;
			
			if(NRF24_dev.hspi->State == HAL_TIMEOUT)
				NRF24_dev.cnt_spi_err_tmo++;
			else
				if(NRF24_dev.hspi->State == HAL_ERROR)
					NRF24_dev.cnt_spi_err++;
		}
		else
			NRF24_dev.stat_spi = NRF24_STAT_SPI_TXRX_OK;
	}
	return fl_state;
}

/**
	* @brief 			  Записать данные по управляющей команде 
									(Например, чтение\запись данных из\в очереди(ь))
  * @param   cmd: Команда управления
	* @param   buf: Ссылка на массив с данными
  * @param  size: Кол-во байт на прием\передачу
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Write_Command(NRF24_Enum_Cmd cmd, u8* buf, u8 size)
{
	if( size <= NRF24_IDX_BF_RX_MAX &&
		 (cmd >= NRF24_CMD_R_RX_PLD && cmd < NRF24_CMD_NOP))
	{	
		NRF24_dev.bf_spi_tx[0] = cmd;
		
		if(buf != NULL)
			for(u8 i = 0; i < size; i++)
				NRF24_dev.bf_spi_tx[1+i] = buf[i];
		
		return NRF24_Spi_Tx_Rx(NRF24_dev.spi_mode, size);
	}
	// Выставить статус ошибки - неверно заданы параметры функции
	return false;
}

/**
	* @brief 			  Записать данные в регистр
  * @param   reg: Номер регистра
  * @param  size: Кол-во байт на прием\передачу
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Write_Data_Reg(u8 reg, u8 size)
{
	if(size <= NRF24_IDX_BF_RX_MAX &&
		 reg <= NRF24_REG_1D_FEATURE)
	{	
		NRF24_dev.bf_spi_tx[0] = NRF24_CMD_W_REG|reg;
		NRF24_Write_Data_In_Buf(reg);
		return NRF24_Spi_Tx_Rx(NRF24_dev.spi_mode, size);
	}
	// Выставить статус ошибки - неверно заданы параметры функции
	return false;
}

/**
	* @brief 			   Прочитать данные из регистра
  * @param    reg: Номер регистра
  * @param   size: Кол-во байт на прием\передачу
	* @param  fl_wr: Флаг - сохранить прочитанные данные из буфера в регистр
	* @retval  bool: Результат операции (true - успешно)
  */
bool NRF24_Read_Data_Reg(u8 reg, u8 size, bool fl_wr)
{
	if(size <= NRF24_IDX_BF_RX_MAX &&
		 reg <= NRF24_REG_1D_FEATURE)
	{
		NRF24_dev.bf_spi_tx[0] = NRF24_CMD_R_REG|reg;
		
		for(u8 i = NRF24_ISD; i < size; i++)
			NRF24_dev.bf_spi_tx[i] = NRF24_CMD_NOP;
		
		if(NRF24_Spi_Tx_Rx(NRF24_dev.spi_mode, size))
		{
			if(fl_wr)
				NRF24_Read_Data_In_Buf_And_Write_Reg(reg);
			
			return true;
		}
	}
	// Выставить статус ошибки - неверно заданы параметры функции
	return false;
}

/**
	* @brief 			  Записать данные регистра в массив, для последующей записи в регистр устр-ва 
  * @param   reg: Номер регистра
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Write_Data_In_Buf(u8 reg)
{
	switch(reg) 										
	{ 
		case NRF24_REG_00_CONFIG:   		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x00_cfg.full; 	
			break;	
		case NRF24_REG_01_EN_AA: 		 		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x01_en_aa.full;
			break;
		case NRF24_REG_02_EN_RXADDR: 		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x02_en_rx_addr.full;	
			break;
		case NRF24_REG_03_SETUP_AW: 		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x03_setup_aw.full;	
			break;						
		case NRF24_REG_04_SETUP_RETR: 	NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x04_setup_petr.full;
			break;
		case NRF24_REG_05_RF_CH: 				NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x05_rf_ch.full;
			break;
		case NRF24_REG_06_RF_SETUP: 		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x06_rf_setup.full;
			break;
		case NRF24_REG_07_STATUS: 			NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x07_status.full;     	// регистр для чтения (сброс бит записью)
			break;				
//		case NRF24_REG_08_OBSERVE_TX: 	NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x08_observe_tx.full; // регистр для чтения
//			break;
//		case NRF24_REG_09_CD: 					NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x09_cd.full;				  // регистр для чтения
//			break;
		case NRF24_REG_0A_RX_ADDR_P0: 	for(u8 adr = 0; adr < NRF24_ADR_SZ_5; adr++)
																			NRF24_dev.bf_spi_tx[NRF24_ISD + adr] = NRF24_dev.map._0x0A_rx_addr_p0[adr];	
			break;
		case NRF24_REG_0B_RX_ADDR_P1: 	for(u8 adr = 0; adr < NRF24_ADR_SZ_5; adr++)
																			NRF24_dev.bf_spi_tx[NRF24_ISD + adr] = NRF24_dev.map._0x0B_rx_addr_p1[adr];	
			break;
		case NRF24_REG_0C_RX_ADDR_P2: 	for(u8 adr = 0; adr < NRF24_ADR_SZ_5; adr++)
																			NRF24_dev.bf_spi_tx[NRF24_ISD + adr] = NRF24_dev.map._0x0C_rx_addr_p2[adr];	
			break;
		case NRF24_REG_0D_RX_ADDR_P3: 	for(u8 adr = 0; adr < NRF24_ADR_SZ_5; adr++)
																			NRF24_dev.bf_spi_tx[NRF24_ISD + adr] = NRF24_dev.map._0x0D_rx_addr_p3[adr];	
			break;
		case NRF24_REG_0E_RX_ADDR_P4: 	for(u8 adr = 0; adr < NRF24_ADR_SZ_5; adr++)
																			NRF24_dev.bf_spi_tx[NRF24_ISD + adr] = NRF24_dev.map._0x0E_rx_addr_p4[adr];	
			break;
		case NRF24_REG_0F_RX_ADDR_P5: 	for(u8 adr = 0; adr < NRF24_ADR_SZ_5; adr++)
																			NRF24_dev.bf_spi_tx[NRF24_ISD + adr] = NRF24_dev.map._0x0F_rx_addr_p5[adr];	
			break;
		case NRF24_REG_10_TX_ADDR: 			for(u8 adr = 0; adr < NRF24_ADR_SZ_5; adr++)
																			NRF24_dev.bf_spi_tx[NRF24_ISD + adr] = NRF24_dev.map._0x10_tx_addr[adr];	
			break;
		case NRF24_REG_11_RX_PW_P0: 		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x11_rx_pw_p0.full;
			break;
		case NRF24_REG_12_RX_PW_P1: 		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x12_rx_pw_p1.full;
			break;
		case NRF24_REG_13_RX_PW_P2: 		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x13_rx_pw_p2.full;
			break;
		case NRF24_REG_14_RX_PW_P3: 		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x14_rx_pw_p3.full;
			break;
		case NRF24_REG_15_RX_PW_P4: 		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x15_rx_pw_p4.full;
			break;
		case NRF24_REG_16_RX_PW_P5: 		NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x16_rx_pw_p5.full;
			break;
		case NRF24_REG_17_FIFO_STATUS:	NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x17_fifo_stat.full;			// регистр для чтения (сброс бит записью)
			break;
		case NRF24_REG_1C_DYNPD: 				NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x1C_dynpd.full;
			break;
		case NRF24_REG_1D_FEATURE: 			NRF24_dev.bf_spi_tx[NRF24_ISD] = NRF24_dev.map._0x1D_feature.full;
			break;
		
		default: return false; // добавить статус ошибки - неизвестная функция 
	}
	return true;	
} 

/**
	* @brief 			  Переписать принятые данные из буфера в регистр
  * @param   reg: Номер регистра
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Read_Data_In_Buf_And_Write_Reg(u8 reg)
{
	switch(reg) 										
	{ 
		case NRF24_REG_07_STATUS: 			NRF24_dev.map._0x07_status.full 		= NRF24_dev.bf_spi_rx[NRF24_ISD];     
			break;				
		case NRF24_REG_08_OBSERVE_TX: 	NRF24_dev.map._0x08_observe_tx.full = NRF24_dev.bf_spi_rx[NRF24_ISD]; 
			break;
		case NRF24_REG_09_CD: 					NRF24_dev.map._0x09_cd.full 				= NRF24_dev.bf_spi_rx[NRF24_ISD];				
			break;
		case NRF24_REG_17_FIFO_STATUS:	NRF24_dev.map._0x17_fifo_stat.full 	= NRF24_dev.bf_spi_rx[NRF24_ISD];					
			break;
		
		default: return false; // добавить статус ошибки - неизвестная функция 
	}
	return true;	
} 

/**
	* @brief 			  Записать в FIFO_TX очередь радиотрансивера данные для отправки
	* @param   buf: Ссылка на массив с данными
  * @param  size: Кол-во байт на прием\передачу
  * @param   cmd: Команды управления радиотрансивером
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Write_Data_In_Fifo_Tx(u8* buf, u8 size, NRF24_Enum_Cmd cmd)
{
	if(cmd == NRF24_CMD_W_TX_PLD || cmd == NRF24_CMD_W_TX_PLD_NOACK)
		if(size <= NRF24_IDX_BF_RX_MAX)
		{	
			NRF24_dev.bf_spi_tx[0] = cmd;
			size++; // +1 байт, команда перед данными
			
			for(u8 i = NRF24_ISD; i < size; i++)
				NRF24_dev.bf_spi_tx[i] = buf[i-1];
			
			return NRF24_Spi_Tx_Rx(NRF24_dev.spi_mode, size);
		}
	// Ошибка - неверно заданы параметры функции
	return false;
} 

/**
	* @brief 			  Прочитать из FIFO_RX очереди радиотрансивера принятые данные 
	* @param   buf: Ссылка на массив с данными
  * @param  size: Кол-во байт на прием\передачу
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Read_Data_In_Fifo_Rx(u8* buf, u8 size)
{
	if(size <= NRF24_IDX_BF_RX_MAX)
	{
		bool stat;
		
		NRF24_dev.bf_spi_tx[0] = NRF24_CMD_R_RX_PLD;
		size++; // +1 значение команды NRF24_CMD_R_RX_PLD в нулевом элементу массива
		size++; // +1 ноп ответ на команду NRF24_CMD_R_RX_PLD
		
		for(u8 i = NRF24_ISD; i < size; i++)
			NRF24_dev.bf_spi_tx[i] = NRF24_CMD_NOP;
		
		stat = NRF24_Spi_Tx_Rx(NRF24_dev.spi_mode, size);
		
		if(stat)
			for(u8 i = 0; i < NRF24_IDX_BF_RX_MAX; i++)
				NRF24_dev.bf_radio[i] = NRF24_dev.bf_spi_rx[i+1];
		
		return stat;
	}
	// Выставить статус ошибки - неверно заданы параметры функции
	return false;
}

/**
	* @brief 			  Прочитать размер принятого пакета
									Значение больше 32, означает ошибку 
	* @retval bool: Результат операции (true - успешно)
  */
bool NRF24_Read_Size_Rx_Pack(void)
{
	bool stat;
	NRF24_dev.bf_spi_tx[0] = NRF24_CMD_R_RX_PL_WID;
	NRF24_dev.bf_spi_tx[1] = NRF24_CMD_NOP;
	stat = NRF24_Spi_Tx_Rx(NRF24_dev.spi_mode, 2);
	NRF24_dev.n_bf_radio = NRF24_dev.bf_spi_rx[1];
	return stat;
}

/**
	* @brief 				Проверить работоспособность связи по SPI с радиотрансивером
	* @retval bool: Возвращает флаг - результат операции (true - на связи)
  */
bool NRF24_Check_Connection_Spi(void)
{
	if(NRF24_Read_Data_Reg(NRF24_REG_00_CONFIG, 2, false))
		if(NRF24_dev.bf_spi_rx[NRF24_ISS] != NULL)	
			return true;
	
	return false;
}

/**
	* @brief Задать конф-ю "NRF24_dev.map" радиотрансивера "по умолчанию"
  */
void NRF24_Set_Default_Settings(void)
{
	//********************************************
	// Задаем адреса текущего и удаленных устр-в
	//********************************************
	for(u8 adr = 0; adr < NRF24_ADR_SZ_5; adr++)																
	{
		NRF24_dev.map._0x0A_rx_addr_p0[adr] = 0x11;
		NRF24_dev.map._0x0B_rx_addr_p1[adr] = 0x22;
		
		if(adr == 0)
		{
			NRF24_dev.map._0x0C_rx_addr_p2[adr] = 0x33;
			NRF24_dev.map._0x0D_rx_addr_p3[adr] = 0x44;
			NRF24_dev.map._0x0E_rx_addr_p4[adr] = 0x55;
			NRF24_dev.map._0x0F_rx_addr_p5[adr] = 0x66;
		}
		NRF24_dev.map._0x10_tx_addr[adr] = 0xFF;
	}
	//--------------------------------------------
	//********************************************
	//***** Настройка беспроводного модуля *******
	//********************************************
	NRF24_dev.map._0x00_cfg.full = 0;
	NRF24_dev.map._0x00_cfg.bit._0_prim_rx 		= true;															// R\W Выбор режима 		 (0 - передатчик; 1 - приёмник)
	NRF24_dev.map._0x00_cfg.bit._1_pwr_up  		= false;														// R\W Включение питания (0 - выкл.;  1 - вкл.)
	NRF24_dev.map._0x00_cfg.bit._2_sz_crc  		= true;															// R\W Размер поля CRC 	 (0 - 1 байт; 1 - 2 байта)
	NRF24_dev.map._0x00_cfg.bit._3_en_crc  		= true;															// R\W Включает CRC
	NRF24_dev.map._0x00_cfg.bit._4_mask_mx_rt = true;															// R\W Запрещает прерывание по MAX_RT (превышение числа повторных попыток отправки) 
	NRF24_dev.map._0x00_cfg.bit._5_mask_tx_ds	= true;															// R\W Запрещает прерывание по TX_DS  (завершение отправки пакета)
	NRF24_dev.map._0x00_cfg.bit._6_mask_rx_rd	= true;															// R\W Запрещает прерывание по RX_DR  (получение пакета)	
	//------------
	NRF24_dev.map._0x01_en_aa.bit._0_en_auto_ch0 = true;													// вкл. авто подтверждение успешно принятых данных по каналу 0
	NRF24_dev.map._0x01_en_aa.bit._1_en_auto_ch1 = false;
	NRF24_dev.map._0x01_en_aa.bit._2_en_auto_ch2 = false;
	NRF24_dev.map._0x01_en_aa.bit._3_en_auto_ch3 = false;
	NRF24_dev.map._0x01_en_aa.bit._4_en_auto_ch4 = false;
	NRF24_dev.map._0x01_en_aa.bit._5_en_auto_ch5 = false;
	//------------
	NRF24_dev.map._0x02_en_rx_addr.bit._0_en_ch0 = true;													// настроить 0-ой канал на прием(1)\передачу(0) данных 
	NRF24_dev.map._0x02_en_rx_addr.bit._1_en_ch1 = false;
	NRF24_dev.map._0x02_en_rx_addr.bit._2_en_ch2 = false;	
	NRF24_dev.map._0x02_en_rx_addr.bit._3_en_ch3 = false;	
	NRF24_dev.map._0x02_en_rx_addr.bit._4_en_ch4 = false;	
	NRF24_dev.map._0x02_en_rx_addr.bit._5_en_ch5 = false;		
	//------------	
	NRF24_dev.map._0x03_setup_aw.bit._0_1_adr_sz = NRF24_ADR_LEN_5;								// выбор длины адреса 5 байт	
	//------------
	NRF24_dev.map._0x04_setup_petr.bit._0_3_arc = NRF24_ARC_5;										// выбор кол-ва перезапросов (повторов)
	NRF24_dev.map._0x04_setup_petr.bit._4_7_ard = NRF24_ARD_1000mks;							// выбор времени ожидания авто перезапроса	 
	//------------
	NRF24_dev.map._0x05_rf_ch.bit._0_6_rfch = 99;																	// частота радиоканала (несущая частота). 2400 + RF_CH(От 0 до 125) МГц	
	//------------
	NRF24_dev.map._0x06_rf_setup.bit._1_2_rf_pwr 		= NRF24_GAIN_m06dBm;					// задать мощность передатчика (0dBm - самая высокая мощность сигнала)
	NRF24_dev.map._0x06_rf_setup.bit._3_rf_dr_hight = false;											// вкл. высокую скорость передачи данных (при значении бита RF_DR_LOW = 0: 0 - 1Мбит/с; 1 - 2Мбит/с)
	NRF24_dev.map._0x06_rf_setup.bit._4_pll_lock 		= false;											// заблокировать усиление несущей (для тестирования)
	NRF24_dev.map._0x06_rf_setup.bit._5_rf_dr_low 	= true;												// вкл. низкую скорость передачи данных 250кбит/с
	NRF24_dev.map._0x06_rf_setup.bit._7_cont_wave 	= false;											// вкл. непрерывную передачу несущей (для тестирования) 		
	//------------
	NRF24_dev.map._0x11_rx_pw_p0.bit._0_5_sz_data = 32;
	NRF24_dev.map._0x12_rx_pw_p1.bit._0_5_sz_data = 32;
	NRF24_dev.map._0x13_rx_pw_p2.bit._0_5_sz_data = 32;	
	NRF24_dev.map._0x14_rx_pw_p3.bit._0_5_sz_data = 32;	
	NRF24_dev.map._0x15_rx_pw_p4.bit._0_5_sz_data = 32;		
	NRF24_dev.map._0x16_rx_pw_p5.bit._0_5_sz_data = 32;
	//------------
	NRF24_dev.map._0x1C_dynpd.bit._0_dpl_ch0 = false;															// разрешить приём пакетов произвольной длины по каналу №0
	NRF24_dev.map._0x1C_dynpd.bit._1_dpl_ch1 = false;																 
	NRF24_dev.map._0x1C_dynpd.bit._2_dpl_ch2 = false;																
	NRF24_dev.map._0x1C_dynpd.bit._3_dpl_ch3 = false;																	 
	NRF24_dev.map._0x1C_dynpd.bit._4_dpl_ch4 = false;																	
	NRF24_dev.map._0x1C_dynpd.bit._5_dpl_ch5 = false;															
	//------------
	NRF24_dev.map._0x1D_feature.bit._0_en_dyn_ack = false;												// разрешает передавать пакеты, не требующие подтверждения приёма
	NRF24_dev.map._0x1D_feature.bit._1_en_ack_pay	= false;												// включает поддержку передачи данных с пакетами подтверждения	
	NRF24_dev.map._0x1D_feature.bit._2_en_dpl 		= false;												// включает поддержку приёма и передачи пакетов с размером поля данных произвольной длины
	//------------		
}

/**
	* @brief 				Записать текущую конф-ю из "NRF24_dev.map" в радиотрансивер
	* @retval bool: Возвращает флаг - результат сравнения конф-и (true - совпадает)
  */
bool NRF24_Write_Current_Settings(void)
{
	if(NRF24_Write_Data_Reg(NRF24_REG_00_CONFIG, 		 2))
	if(NRF24_Write_Data_Reg(NRF24_REG_01_EN_AA,			 2))
	if(NRF24_Write_Data_Reg(NRF24_REG_02_EN_RXADDR,  2))		
	if(NRF24_Write_Data_Reg(NRF24_REG_03_SETUP_AW, 	 2))	
	if(NRF24_Write_Data_Reg(NRF24_REG_04_SETUP_RETR, 2))	 
	if(NRF24_Write_Data_Reg(NRF24_REG_05_RF_CH, 		 2))	
	if(NRF24_Write_Data_Reg(NRF24_REG_06_RF_SETUP, 	 2))			
	if(NRF24_Write_Data_Reg(NRF24_REG_0A_RX_ADDR_P0, 6))														
	if(NRF24_Write_Data_Reg(NRF24_REG_0B_RX_ADDR_P1, 6))
	if(NRF24_Write_Data_Reg(NRF24_REG_0C_RX_ADDR_P2, 2))								
	if(NRF24_Write_Data_Reg(NRF24_REG_0D_RX_ADDR_P3, 2))														
	if(NRF24_Write_Data_Reg(NRF24_REG_0E_RX_ADDR_P4, 2))															
	if(NRF24_Write_Data_Reg(NRF24_REG_0F_RX_ADDR_P5, 2))
	if(NRF24_Write_Data_Reg(NRF24_REG_10_TX_ADDR, 	 6))																																				
	if(NRF24_Write_Data_Reg(NRF24_REG_11_RX_PW_P0, 	 2))
	if(NRF24_Write_Data_Reg(NRF24_REG_12_RX_PW_P1, 	 2))	
	if(NRF24_Write_Data_Reg(NRF24_REG_1C_DYNPD, 		 2))
	if(NRF24_Write_Data_Reg(NRF24_REG_1D_FEATURE, 	 2))
		return true;

	return false;	
}

/**
	* @brief 				Прочитать текущую конф-ю из радиотрансивера
	* @retval bool: Возвращает флаг - результат операции
  */
bool NRF24_Read_Current_Settings(void)
{
	if(NRF24_Read_Data_Reg(NRF24_REG_00_CONFIG, 2, false))
		NRF24_dev.map_s._0x00_cfg.full = NRF24_dev.bf_spi_rx[NRF24_ISD];
	if(NRF24_Read_Data_Reg(NRF24_REG_01_EN_AA, 2, false))
		NRF24_dev.map_s._0x01_en_aa.full = NRF24_dev.bf_spi_rx[NRF24_ISD];
	if(NRF24_Read_Data_Reg(NRF24_REG_02_EN_RXADDR, 2, false))
		NRF24_dev.map_s._0x02_en_rx_addr.full = NRF24_dev.bf_spi_rx[NRF24_ISD];
	if(NRF24_Read_Data_Reg(NRF24_REG_03_SETUP_AW, 2, false))
		NRF24_dev.map_s._0x03_setup_aw.full = NRF24_dev.bf_spi_rx[NRF24_ISD];
	if(NRF24_Read_Data_Reg(NRF24_REG_04_SETUP_RETR, 2, false))
		NRF24_dev.map_s._0x04_setup_petr.full = NRF24_dev.bf_spi_rx[NRF24_ISD];
	if(NRF24_Read_Data_Reg(NRF24_REG_05_RF_CH, 2, false))
		NRF24_dev.map_s._0x05_rf_ch.full = NRF24_dev.bf_spi_rx[NRF24_ISD];
	if(NRF24_Read_Data_Reg(NRF24_REG_06_RF_SETUP, 2, false))
		NRF24_dev.map_s._0x06_rf_setup.full = NRF24_dev.bf_spi_rx[NRF24_ISD];

	if(NRF24_Read_Data_Reg(NRF24_REG_0A_RX_ADDR_P0, 6, false))
		for(u8 i = 0; i < NRF24_ADR_SZ_5; i++)
			NRF24_dev.map_s._0x0A_rx_addr_p0[i] = NRF24_dev.bf_spi_rx[NRF24_ISD+i];
	if(NRF24_Read_Data_Reg(NRF24_REG_0B_RX_ADDR_P1, 6, false))
		for(u8 i = 0; i < NRF24_ADR_SZ_5; i++)
			NRF24_dev.map_s._0x0B_rx_addr_p1[i] = NRF24_dev.bf_spi_rx[NRF24_ISD+i];
	if(NRF24_Read_Data_Reg(NRF24_REG_0C_RX_ADDR_P2, 2, false))
		for(u8 i = 0; i < NRF24_ADR_SZ_1; i++)
			NRF24_dev.map_s._0x0C_rx_addr_p2[i] = NRF24_dev.bf_spi_rx[NRF24_ISD+i];
	if(NRF24_Read_Data_Reg(NRF24_REG_0D_RX_ADDR_P3, 2, false))
		for(u8 i = 0; i < NRF24_ADR_SZ_1; i++)
			NRF24_dev.map_s._0x0D_rx_addr_p3[i] = NRF24_dev.bf_spi_rx[NRF24_ISD+i];
	if(NRF24_Read_Data_Reg(NRF24_REG_0E_RX_ADDR_P4, 2, false))
		for(u8 i = 0; i < NRF24_ADR_SZ_1; i++)
			NRF24_dev.map_s._0x0E_rx_addr_p4[i] = NRF24_dev.bf_spi_rx[NRF24_ISD+i];
	if(NRF24_Read_Data_Reg(NRF24_REG_0F_RX_ADDR_P5, 2, false))
		for(u8 i = 0; i < NRF24_ADR_SZ_1; i++)
			NRF24_dev.map_s._0x0F_rx_addr_p5[i] = NRF24_dev.bf_spi_rx[NRF24_ISD+i];
	
	if(NRF24_Read_Data_Reg(NRF24_REG_10_TX_ADDR, 6, false))
		for(u8 i = 0; i < NRF24_ADR_SZ_5; i++)
			NRF24_dev.map_s._0x10_tx_addr[i] = NRF24_dev.bf_spi_rx[NRF24_ISD+i];

	if(NRF24_Read_Data_Reg(NRF24_REG_11_RX_PW_P0, 2, false))
		NRF24_dev.map_s._0x11_rx_pw_p0.full = NRF24_dev.bf_spi_rx[NRF24_ISD];
	if(NRF24_Read_Data_Reg(NRF24_REG_12_RX_PW_P1, 2, false))
		NRF24_dev.map_s._0x12_rx_pw_p1.full = NRF24_dev.bf_spi_rx[NRF24_ISD];
	if(NRF24_Read_Data_Reg(NRF24_REG_1C_DYNPD, 2, false))
		NRF24_dev.map_s._0x1C_dynpd.full = NRF24_dev.bf_spi_rx[NRF24_ISD];
	if(NRF24_Read_Data_Reg(NRF24_REG_1D_FEATURE, 2, false))
		NRF24_dev.map_s._0x1D_feature.full = NRF24_dev.bf_spi_rx[NRF24_ISD];
	
	return true;
}

/**
	* @brief 				Верификация настроек. Прочитать и проверить все регистры конф-и NRF24 с текущей NRF24_dev.map
	* @retval bool: Возвращает флаг - результат сравнения конф-и (true - совпадает)
  */
bool NRF24_Verification_Settings(void)
{
	if(NRF24_Read_Data_Reg(NRF24_REG_00_CONFIG, 2, false))
	if(NRF24_dev.map._0x00_cfg.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_01_EN_AA, 2, false))
	if(NRF24_dev.map._0x01_en_aa.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_02_EN_RXADDR, 2, false))
	if(NRF24_dev.map._0x02_en_rx_addr.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_03_SETUP_AW, 2, false))
	if(NRF24_dev.map._0x03_setup_aw.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_04_SETUP_RETR, 2, false))
	if(NRF24_dev.map._0x04_setup_petr.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_05_RF_CH, 2, false))
	if(NRF24_dev.map._0x05_rf_ch.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_06_RF_SETUP, 2, false))
	if(NRF24_dev.map._0x06_rf_setup.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;

	if(NRF24_Read_Data_Reg(NRF24_REG_0A_RX_ADDR_P0, 6, false))
	for(u8 i = 0; i < NRF24_ADR_SZ_5; i++)
		if(NRF24_dev.map._0x0A_rx_addr_p0[i] != NRF24_dev.bf_spi_rx[NRF24_ISD+i])
			return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_0B_RX_ADDR_P1, 6, false))
	for(u8 i = 0; i < NRF24_ADR_SZ_5; i++)
		if(NRF24_dev.map._0x0B_rx_addr_p1[i] != NRF24_dev.bf_spi_rx[NRF24_ISD+i])
			return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_0C_RX_ADDR_P2, 2, false))
	for(u8 i = 0; i < NRF24_ADR_SZ_1; i++)
		if(NRF24_dev.map._0x0C_rx_addr_p2[i] != NRF24_dev.bf_spi_rx[NRF24_ISD+i])
			return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_0D_RX_ADDR_P3, 2, false))
	for(u8 i = 0; i < NRF24_ADR_SZ_1; i++)
		if(NRF24_dev.map._0x0D_rx_addr_p3[i] != NRF24_dev.bf_spi_rx[NRF24_ISD+i])
			return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_0E_RX_ADDR_P4, 2, false))
	for(u8 i = 0; i < NRF24_ADR_SZ_1; i++)
		if(NRF24_dev.map._0x0E_rx_addr_p4[i] != NRF24_dev.bf_spi_rx[NRF24_ISD+i])
			return false;	
	if(NRF24_Read_Data_Reg(NRF24_REG_0F_RX_ADDR_P5, 2, false))
	for(u8 i = 0; i < NRF24_ADR_SZ_1; i++)
		if(NRF24_dev.map._0x0F_rx_addr_p5[i] != NRF24_dev.bf_spi_rx[NRF24_ISD+i])
			return false;	
	if(NRF24_Read_Data_Reg(NRF24_REG_10_TX_ADDR, 6, false))
	for(u8 i = 0; i < NRF24_ADR_SZ_5; i++)
		if(NRF24_dev.map._0x10_tx_addr[i] != NRF24_dev.bf_spi_rx[NRF24_ISD+i])
			return false;	

	if(NRF24_Read_Data_Reg(NRF24_REG_11_RX_PW_P0, 2, false))
	if(NRF24_dev.map._0x11_rx_pw_p0.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_12_RX_PW_P1, 2, false))
	if(NRF24_dev.map._0x12_rx_pw_p1.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_1C_DYNPD, 2, false))
	if(NRF24_dev.map._0x1C_dynpd.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;
	if(NRF24_Read_Data_Reg(NRF24_REG_1D_FEATURE, 2, false))
	if(NRF24_dev.map._0x1D_feature.full != NRF24_dev.bf_spi_rx[NRF24_ISD])
		return false;
	
	return true;
}
