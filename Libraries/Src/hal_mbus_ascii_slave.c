#include "hal_mbus_ascii_slave.h"
#include "hal_mbus_ascii.h"
#include "hal_control_devs.h"
#include "hal_algorithm.h"
#include "hal_my_lib.h"
#include "hal_my_crc.h"
#include "hal_config.h"
#include "hal_my_rtc.h"
#include "hal_my_adc.h"

/************************************************************************
													Глобальные переменные
*************************************************************************/
u16 						  ASCII_Regs_Jump_BootLdr;			 				 									// регистр - флаг перейти в загрузчик
u16 						  ASCII_Regs_Restart_Cpu;			 			 	 		 								// регистр - флаг перезагрузка контроллера
ASCII_StatRegBits ASCII_Regs_Status_Cpu;			 				 										// регистр - ошибок контроллера
//-----------------------------------------------------------------------

//***********************************************************************
//										 Карта памяти "MODBUS ASCII"
//***********************************************************************
const u16 ASCII_Size_Map = 35;																 						// размер массива карты регистров для протокола модбас
const ASCII_MapObj ASCII_Map[ASCII_Size_Map] = 																
{		
	{0x0000, (u16*)&ASCII_Regs_Restart_Cpu,  		  					 	 1,  ASCII_WRITE, ASCII_PORT_ALL, 0xF00F, 0xF00F, _2_BYTE_},
	{0x0001, (u16*)&ASCII_Regs_Jump_BootLdr,  							 	 1,  ASCII_WRITE, ASCII_PORT_ALL, 0xA55A, 0xA55A, _2_BYTE_},
	{0x0002, (u16*)&CFG_setup.uid_cpu,  			 	 						 	 6,   ASCII_READ, ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
	{0x0008, (u16*)&ASCII_Regs_Status_Cpu.full,  						 	 1,   ASCII_READ, ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},

	{0x0020, (u16*)&RTC_obj.tm_now.Hours,  									 	 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},
	{0x0021, (u16*)&RTC_obj.tm_now.Minutes,  								 	 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},
	{0x0022, (u16*)&RTC_obj.tm_now.Seconds,  								 	 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},	
	{0x0023, (u16*)&RTC_obj.tm_setup.Hours,  									 1,  ASCII_WRITE, ASCII_PORT_ALL, 0x0000, 0x0017, _1_BYTE_},
	{0x0024, (u16*)&RTC_obj.tm_setup.Minutes,  								 1,  ASCII_WRITE, ASCII_PORT_ALL, 0x0000, 0x003B, _1_BYTE_},
	{0x0025, (u16*)&RTC_obj.tm_setup.Seconds,  								 1,  ASCII_WRITE, ASCII_PORT_ALL, 0x0000, 0x003B, _1_BYTE_},
	
	{0x002E, (u16*)&ALG_obj.n_off,  							 						 1,  ASCII_WRITE, ASCII_PORT_ALL, 0x0000, 0x0005, _1_BYTE_},
  {0x0030, (u16*)&ALG_obj.valve_tm_turn.Hours,  						 1,  ASCII_WRITE, ASCII_PORT_ALL, 0x0000, 0x0017, _1_BYTE_},
	{0x0031, (u16*)&ALG_obj.valve_tm_turn.Minutes,  					 1,  ASCII_WRITE, ASCII_PORT_ALL, 0x0000, 0x003B, _1_BYTE_},

	{0x0040, (u16*)&ADC_obj.vbat_cal,  	 											 1,  ASCII_WRITE, ASCII_PORT_ALL, 0x0000, 0xffff, _2_BYTE_},
	{0x0042, (u16*)&ADC_obj.pbat_cal,  	 											 1,  ASCII_WRITE, ASCII_PORT_ALL, 0x0000, 0xffff, _2_BYTE_},
	{0x0044, (u16*)&ADC_obj.vbat,  														 1,  ASCII_READ,  ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
	{0x0046, (u16*)&ADC_obj.pbat,  			 											 1,  ASCII_READ,  ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
	{0x0049, (u16*)&ADC_obj.vbat_state, 											 1,  ASCII_READ,  ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},
	{0x004A, (u16*)&ADC_obj.pbat_state, 											 1,  ASCII_READ,  ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},	
	
	{0x0050, (u16*)&ALG_obj.rf_s_diag.full,  									 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
	{0x0051, (u16*)&ALG_obj.rf_s_stat.full,  									 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
	
	{0x0060, (u16*)&ALG_obj.rf_s_bat_charge[0],  							 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},
	{0x0061, (u16*)&ALG_obj.rf_s_bat_charge[1],  							 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},
	{0x0062, (u16*)&ALG_obj.rf_s_bat_charge[2],  							 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},
	{0x0063, (u16*)&ALG_obj.rf_s_bat_charge[3],  							 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},
	{0x0064, (u16*)&ALG_obj.rf_s_bat_charge[4],  							 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},
	{0x0065, (u16*)&ALG_obj.rf_s_bat_charge[5],  							 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},
	
	{0x0070, (u16*)&ALG_obj.rf_s_bat_voltage[0],  						 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
	{0x0072, (u16*)&ALG_obj.rf_s_bat_voltage[1],  						 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
	{0x0074, (u16*)&ALG_obj.rf_s_bat_voltage[2],  						 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
	{0x0076, (u16*)&ALG_obj.rf_s_bat_voltage[3],  						 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
	{0x0078, (u16*)&ALG_obj.rf_s_bat_voltage[4],  						 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
	{0x007A, (u16*)&ALG_obj.rf_s_bat_voltage[5],  						 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _2_BYTE_},
		
	{0x0110, (u16*)&CDD_Obj[CDD_NAME_OUT_VALVE_CH_1].state,  	 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},
	{0x0111, (u16*)&CDD_Obj[CDD_NAME_OUT_VALVE_CH_2].state, 	 1,  ASCII_READ, 	ASCII_PORT_ALL, 0x0000, 0x0000, _1_BYTE_},	
};
//-----------------------------------------------------------------------

/************************************************************************
												Прототипы всех функций модуля
*************************************************************************/
void ASCII_Init(void);																										// Инициализация модуля (положить main)

void ASCII_Analiz_Pack_Slave(u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port);// Анализ полученного пакета и определение функции

void ASCII_Read_Regs_03 (u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port);		// Чтение нескольких регистров
void ASCII_Write_Reg_06 (u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port);		// Запись 1-го регистра
void ASCII_Echo_08 			(u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port);		// Диагностика (эхо)
void ASCII_Write_Regs_16(u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port);		// Запись нескольких регистров
void ASCII_Read_Info_17 (u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port);		// ID устр-ва

bool ASCII_Get_Stat_Allow_Port(u8 nReg);																	// Получить статус разрешения, для обращения к регистру карты памяти через порт

void ASCII_Answer_ReTx(u8 type, u8 err_code);															// Отправить пакет с ответом mastery, на его запрос о ретрансляции пакета 

void ASCII_Set_Flags								(u16 idx, u16 val);										// Выставляем необходимые флаги, в соответствии с регистрами
bool ASCII_Set_Map_Prm_Kostil_Rd_Reg(u16 idx, u16 val);										// Если надо сделать костыль для чтения значения из определенного регистра
bool ASCII_Set_Map_Prm_Kostil_Wr_Reg(u16 idx, u16 val);										// Если надо сделать костыль для записи значения в определенный регистр
bool ASCII_Check_Map_Prm_Val				(u16 idx, u8* bufRx, u16 ne);					// Если надо сделать фильтр для определенного регистра	
//------------------------------------------------------------------------	

//************************************************************************
//											Описание всех функций модуля
//************************************************************************
/**
  * @brief Инициализация модуля 
  */ 
void ASCII_Init(void)
{
	ASCII_Regs_Status_Cpu.full = NULL;
 	ASCII_Regs_Jump_BootLdr = NULL;			 				 								
 	ASCII_Regs_Restart_Cpu  = NULL;			 			 	 		 										 				 									
}

/**
  * @brief      		 Определяем какую функцию запросил "Мастер" и выполняем ее
	* @param	 *bufRx: Буфер для приема данных	
	* @param	 *bufTx: Буфер для передачи данных	
	* @param  n_bytes: Кол-во принятых байт в буфере Rx	
	* @param   n_port: Номер порта
  */ 
void ASCII_Analiz_Pack_Slave(u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port) 										
{
	static u8 adr; adr = bufRx[0];
	static u8 fnc; fnc = bufRx[1];																						
	
	if((adr > NULL && adr == ASCII_Port[n_port].adr) || 											// Если не широковещательный пакет и адрес совпадает
		 (!adr && (fnc == 0x06 || fnc == 0x10))) 																// Если широковещательный и функция записи
	{
		switch(fnc)
		{
			case 0x03: ASCII_Read_Regs_03 (bufRx, bufTx, n_bytes, n_port);	break;// Чтение значений из нескольких регистров хранения
			case 0x06: ASCII_Write_Reg_06 (bufRx, bufTx, n_bytes, n_port);	break;// Запись значения в один регистр хранения
			case 0x08: ASCII_Echo_08 			(bufRx, bufTx, n_bytes, n_port);	break;// Диагностика (эхо)
			case 0x10: ASCII_Write_Regs_16(bufRx, bufTx, n_bytes, n_port);	break;// Запись значений в несколько регистров хранения
			case 0x11: ASCII_Read_Info_17 (bufRx, bufTx, n_bytes, n_port);	break;// Чтение информации об устройстве
			default:   ASCII_Transmit_Answer(n_port, 0x01);     						break;// Ошибка! Используется неизвестная функция
		} 
	}
}

/**
	* @brief        Получить статус разрешения, для обращения к регистру карты памяти через порт
	* @param	nReg: Номер регистра, к котрому обращается Master			
	* @retval 			Возвращает статус разрешения, для обращения к регистру
  */
bool ASCII_Get_Stat_Allow_Port(u8 nReg) 																											
{
	return MYLIB_READ_BIT(ASCII_Map[nReg].nRs, 0);
}

/**
  * @brief      		 Прочитать несколько регистров из карты Modbus и сформировать буфер ответа
	* @param	 *bufRx: Буфер для приема данных	
	* @param	 *bufTx: Буфер для передачи данных	
	* @param  n_bytes: Кол-во принятых байт в буфере Rx		
	* @param   n_port: Номер порта
  */
void ASCII_Read_Regs_03(u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port) 																											
{
	if(n_bytes == 7) 																				 								// защита от неправильного размера пакета
	{ 			
		u16 nRegs    = (u16)((bufRx[4]<<8) | bufRx[5]); 											// сколько регистров читать
		u16 nBytes   = nRegs*2; 																							// сколько байт читать
		u16 startAdr = (u16)((bufRx[2]<<8) | bufRx[3]);    										// адрес начального регистра для чтения		
		u16 cntRegs  = NULL;																									// счетчик запрошенных регистров
		u16 sizeReg  = NULL;																									// размер читаемого регистра
		u16 idx 		 = NULL;																									// индекс карты модбас

		bufTx[0] = bufRx[0];
		bufTx[1] = bufRx[1]; 
		bufTx[2] = nBytes;
		
		for(u16 i = 0; i < ASCII_Size_Map; i++)
			if(ASCII_Map[i].num_reg == startAdr) 													
			{
				if(!nRegs)																
				{
					ASCII_Transmit_Answer(n_port, 0x03);									  				// кол-во регистров в запросе 0
					return;
				}		
				for(u16 k = 0, m = 0; cntRegs < nRegs; m++)											
				{
					idx = m+i;
					
					if(ASCII_Get_Stat_Allow_Port(idx))															// Если порту разрешено обращатся к этому регистру
					{	
						sizeReg = ASCII_Map[idx].size;
						
						if((cntRegs += sizeReg) > nRegs)														
						{
							ASCII_Transmit_Answer(n_port, 0x03);									  		// не верное кол-во регситров в запросе
							return;
						}
						if(ASCII_Map[idx].fl_byte == _1_BYTE_)
						{
							bufTx[3+k] = 0;
							bufTx[4+k] = ASCII_Map[idx].point_val[0];
						}
						else						
							for(u8 b = 0, r = 0, ro = ASCII_Map[idx].size-1; r < sizeReg; b+=2, r++, ro--)
							{
								bufTx[3+k+b] = ASCII_Map[idx].point_val[ro]>>8;
								bufTx[4+k+b] = ASCII_Map[idx].point_val[ro];
							}
						k += ASCII_Map[idx].size*2;
					}
					else
					{
						ASCII_Transmit_Answer(n_port, 0x02);									  		
						return;
					}
				}
				ASCII_Port[n_port].sizeTx = 4 + nBytes;   												
				bufTx[ASCII_Port[n_port].sizeTx-1] = Lrc_8(bufTx, ASCII_Port[n_port].sizeTx-1);			
				ASCII_Transmit_Answer(n_port, NULL); 														
				return;
			}
		ASCII_Transmit_Answer(n_port, 0x02);																	// нет такого регистра
	}
	else
		ASCII_Transmit_Answer(n_port, 0x03);																	// не верный размер пакета
}

/**
  * @brief      		 Записать в 1 регистр карты модбас данные и сформировать ответ о записи
	* @param	 *bufRx: Буфер для приема данных	
	* @param	 *bufTx: Буфер для передачи данных	
	* @param  n_bytes: Кол-во принятых байт в буфере Rx	
	* @param   n_port: Номер порта
  */
void ASCII_Write_Reg_06(u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port)
{
	if(n_bytes == 7)																										
	{
		u16 startAdr = (u16)((bufRx[2]<<8) | bufRx[3]);    									

		for(u16 i = 0; i < ASCII_Size_Map; i++)										
			if(ASCII_Map[i].num_reg == startAdr) 										
			{
				if(ASCII_Map[i].size > 1)																																						
				{
					ASCII_Transmit_Answer(n_port, 0x03);									  				
					return;
				}
				if(ASCII_Get_Stat_Allow_Port(i))															
				{
					if(ASCII_Map[i].flagRW == ASCII_WRITE)												
					{
						u16 valW = 0;
						
						if(ASCII_Map[i].fl_byte == _1_BYTE_)
							valW = bufRx[5];									
						else
							valW = (u16)(bufRx[4]<<8 | bufRx[5]);	
						
						if(valW >= ASCII_Map[i].valOf && valW <= ASCII_Map[i].valTo)	
						{
							ASCII_Set_Flags(i, valW); 																	
							
							if(!ASCII_Set_Map_Prm_Kostil_Wr_Reg(i, valW)) 							// Если надо сделать костыль для определенного регистра
							{																												
								if(ASCII_Map[i].fl_byte == _1_BYTE_)
									ASCII_Map[i].point_val[0] = 0x00ff&valW; 														
								else
									ASCII_Map[i].point_val[0] = valW;
							}							
							for(u16 n = 0; n < 6; n++) 		                   		
								bufTx[n] = bufRx[n];
							
							ASCII_Port[n_port].sizeTx = 7;   												
							bufTx[6] = Lrc_8(bufTx, 6);		
							
							if(bufRx[0] != 0)																						// Если не широковещательный пакет
								ASCII_Transmit_Answer(n_port, NULL); 								
							
							return;																									
						}
						else{
							ASCII_Transmit_Answer(n_port, 0x03);											
							return;}
					}
					else{
						ASCII_Transmit_Answer(n_port, 0x03);												
						return;}
				}
				else{
					ASCII_Transmit_Answer(n_port, 0x02);													
					return;}
			}																	
		ASCII_Transmit_Answer(n_port, 0x02);																	
	}	
	else
		ASCII_Transmit_Answer(n_port, 0x03);																	
}

/**
  * @brief      		 Записать в несколько регистров карты модбас данные и сформировать ответ о записи
	* @param	 *bufRx: Буфер для приема данных	
	* @param	 *bufTx: Буфер для передачи данных	
	* @param  n_bytes: Кол-во принятых байт в буфере Rx		
	* @param   n_port: Номер порта
  */
void ASCII_Write_Regs_16(u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port) 
{
	if(n_bytes < 255) 																											// Пакет не превышает размер буфера
	{
		u16 nRegs    = (u16)((bufRx[4]<<8) | bufRx[5]); 											// сколько регистров читать
		u16 startAdr = (u16)((bufRx[2]<<8) | bufRx[3]);    	 									// начальный регистр для чтения
		u16 cntRegs  = 0;																											// счетчик запрошенных регистров
		u16 sizeReg  = 0;																											// размер читаемого регистра
		u16 idx 		 = 0;																											// индекс карты модбас
		
		for(u16 i = 0; i < ASCII_Size_Map; i++)
			if(ASCII_Map[i].num_reg == startAdr)																
			{				
				if(!nRegs)																																									
				{
					ASCII_Transmit_Answer(n_port, 0x03);									  			
					return;
				}
				for(u16 k = 0, m = 0; cntRegs < nRegs; m++)											
				{
					idx = m+i;
					sizeReg = ASCII_Map[idx].size;
					
					if(ASCII_Check_Map_Prm_Val(idx, bufRx, 7+k))										// Или задано значение выходящее за разрешенный диапазон															
					{
						ASCII_Transmit_Answer(n_port, 0x03);									  	
						return;
					}
					if(!ASCII_Get_Stat_Allow_Port(idx))															// Если порту разрешено обращатся к этому регистру
					{
						ASCII_Transmit_Answer(n_port, 0x02);									
						return;
					}
					if((cntRegs += sizeReg) > nRegs ||															
						  ASCII_Map[idx].flagRW != ASCII_WRITE)												
					{
						ASCII_Transmit_Answer(n_port, 0x03);									  
						return;
					}
					u8  nbytes = ASCII_Map[idx].size*2;													
					u8  link_val[8] = {0,0,0,0,0,0,0,0};
					u16 valW = 0;
					
					for(u8 s = 0; s < nbytes; s++)
						link_val[s] |= bufRx[7+k+s];	
					
					if(ASCII_Map[idx].size == 1)																			 																		
					{																															
						if(ASCII_Map[idx].fl_byte == _1_BYTE_)
							valW = bufRx[8+k];
						else
							valW = (u16)(bufRx[7+k]<<8 | bufRx[8+k]);					

						if(valW < ASCII_Map[idx].valOf || valW > ASCII_Map[idx].valTo)
						{
							ASCII_Transmit_Answer(n_port, 0x03);										
							return;																											
						}
					}						
					ASCII_Set_Flags(idx, valW); 																		// Выставляем необходимые флаги, в соответствии с регистрами
					
					if(!ASCII_Set_Map_Prm_Kostil_Wr_Reg(idx, valW)) 								// Если надо сделать костыль\фильтр для определенного регистра
					{																																
						if(ASCII_Map[idx].fl_byte == _1_BYTE_)
							ASCII_Map[idx].point_val[0] = link_val[1]; 												
						else
							for(u8 b = 0, r = 0, ro = ASCII_Map[idx].size-1; r < ASCII_Map[idx].size; b+=2, r++, ro--)
								ASCII_Map[idx].point_val[ro] = link_val[b]<<8 | link_val[b+1];
					}
					k += nbytes;
				}									
				for(u16 n = 0; n < 6; n++) 		                   								
					bufTx[n] = bufRx[n];
				
				ASCII_Port[n_port].sizeTx = 7;   												
				bufTx[6] = Lrc_8(bufTx, 6);	
				
				if(bufRx[0] != 0)																							
					ASCII_Transmit_Answer(n_port, NULL); 															
				
				return;
			}
		ASCII_Transmit_Answer(n_port, 0x02);																	
	}
	else
		ASCII_Transmit_Answer(n_port, 0x03);																
}

/**
	* @brief      		 Диагностика (эхо)
	* @param	 *bufRx: Буфер для приема данных	
	* @param	 *bufTx: Буфер для передачи данных	
	* @param  n_bytes: Кол-во принятых байт в буфере Rx	
	* @param   n_port: Номер порта
  */
void ASCII_Echo_08(u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port) 																	
{ 
	if(n_bytes == 8) 																									
	{ 
		for(int i = 0; i < 6; i++)    					   												
			bufTx[i] = bufRx[i];		
		
		ASCII_Port[n_port].sizeTx = 7;   												
		bufTx[6] = Lrc_8(bufTx, 6);
		ASCII_Transmit_Answer(n_port, NULL); 																	
	}
	else
		ASCII_Transmit_Answer(n_port, 0x03);												
} 

/**
  * @brief      		 Прочитать информацию об устройстве(ID) из регистров карты модбас
	* @param	 *bufRx: Буфер для приема данных	
	* @param	 *bufTx: Буфер для передачи данных	
	* @param  n_bytes: Кол-во принятых байт в буфере Rx		
	* @param   n_port: Номер порта
  */
void ASCII_Read_Info_17(u8* bufRx, u8* bufTx, u8 n_bytes, u8 n_port) 																
{
	if(n_bytes == 4) 																										
	{  
		bufTx[0] = bufRx[0];
		bufTx[1] = bufRx[1];
		bufTx[2] = sizeof(ASCII_ID);
		bufTx[3] = ASCII_ID>>16;
		bufTx[4] = ASCII_ID>>8;
		bufTx[5] = (u8)ASCII_ID;
		ASCII_Port[n_port].sizeTx = 7;   												
		bufTx[6] = Lrc_8(bufTx, 6);
		ASCII_Transmit_Answer(n_port, NULL); 																
	}
	else
		ASCII_Transmit_Answer(n_port, 0x03);														
}

/**
  * @brief         Если надо сделать фильтр для определенного регистра
	* @param	  idx: Индекс массива, который соответствует регистру карты памяти
	* @param *bufRx: Буфер для приема данных
	* @param     ne: Номер массива буфера, с которого начать
	* @retval 		   Возвращает флаг - ошибка задания значения (true - отправить ответ об ошибке)
  */
bool ASCII_Check_Map_Prm_Val(u16 idx, u8* bufRx, u16 ne)
{
	
	return false;
}
				
/**
  * @brief       Если надо сделать костыль для чтения значения из определенного регистра
	* @param	idx: Индекс массива, который соответствует регистру карты памяти, который был изменен	
	* @param	val: Значение, которое необходимо запистаь в регистр 		
	* @retval 		 Возвращает флаг (true - ошибка; false - изменять реистр)
  */
bool ASCII_Set_Map_Prm_Kostil_Rd_Reg(u16 idx, u16 val)
{
	
	return false;
}

/**
  * @brief       Если надо сделать костыль для записи значения в определенный регистр
	* @param	idx: Индекс массива, который соответствует регистру карты памяти, который был изменен	
	* @param	val: Значение, которое необходимо запистаь в регистр 		
	* @retval 		 Возвращает флаг (false - не изменять регистр; true - изменять)
  */
bool ASCII_Set_Map_Prm_Kostil_Wr_Reg(u16 idx, u16 val)
{
	static bool fl_kostil; fl_kostil = false;	
	static u16  reg_val; 
	
	if(ASCII_Map[idx].fl_byte == _2_BYTE_)
		reg_val = ASCII_Map[idx].point_val[0];
	else
		reg_val = (u16)(0x00ff&(ASCII_Map[idx].point_val[0]>>8));
	
	if(reg_val != val)
	{
		
	}
	return fl_kostil;
}

/**
	* @brief       Выставляем необходимые флаги, в соответствии с регистрами	
	* @param	idx: Индекс массива, который соответствует регистру карты памяти, который был изменен	
	* @param	val: Значение регистра
  */
void ASCII_Set_Flags(u16 idx, u16 val) 
{	
	static u16 reg_val; 
	
	if(ASCII_Map[idx].flagRW != ASCII_WRITE)																			// Если регистр для записи
		return; 
	
	if(ASCII_Map[idx].fl_byte == _2_BYTE_)
		reg_val = ASCII_Map[idx].point_val[0];
	else
		reg_val = (u16)(0x00ff&(ASCII_Map[idx].point_val[0]>>8));
	
	if(reg_val != val)
	{
		if(ASCII_Map[idx].num_reg == 0x0003)																					// Если был запрос на выход из режима конф.
		{
			if(CFG_setup.re_cfg_code == 0x80C4 && val != 0x80C4)
				CFG_setup.fl_re_cfg_ok = true;
		}
		// Сюда добавить требуемые регистры
	}
}  
//----------------------------------------------------------------------------------------
