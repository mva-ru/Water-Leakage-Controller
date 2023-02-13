#include "hal_my_spi_callbacks.h"
//#include "hal_nrf24l01_drv.h"

/**********************************************************************
									��������� ���� ������� � ������� ������
***********************************************************************/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);										// ������� ����������� ����� ���������� �������� ������ �� SPI
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);										// ������� ����������� ����� ���������� ������ ������ �� SPI
//---------------------------------------------------------------------	

//*********************************************************************
//								�������� ���� ������� � ������� ������
//*********************************************************************
///**
//  * @brief 				������� ����������� ����� ���������� �������� ������ �� SPI
//	* @param  hspi: ��������� ��� ������ � SPI
//  */
//void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
//{																							
//	if(D1322_drv.hspi->State == HAL_SPI_STATE_READY)
//		if(D1322_drv.spi_mode == SSD1322_TYPE_RX_TX_IT || 
//			 D1322_drv.spi_mode == SSD1322_TYPE_RX_TX_DMA)	
//			if(D1322_drv.hspi == hspi)
//			{
//				if(D1322_drv.cnt_hz > D1322_drv.cnt_hz_max)
//				{
//					if(D1322_drv.link_func[0].fl_tx_cmd)															// �� �����, ���� �������� ������� ������� �� DMA\IT
//					{
//						SSD1322_PIN_CS_ON();
//						D1322_drv.link_func[0].fl_tx_cmd = false;
//						
//						if(D1322_drv.link_func == &D1322_drv.write_ram_0x5C)
//							D1322_Tx_Cmd_Write_Ram(SSD1322_SIZE_RAM, D1322_drv.spi_mode);	// ��������� ������ � RAM ������� (�������� ������ �� ������)
//						else
//							D1322_drv.err = D1322_ERR_NOT_LINK_FUNC_CALLBACK;
//					}
//					else
//					{
//					SSD1322_PIN_CS_ON();
//					D1322_drv.link_func[0].fl_tx_data = false;
//					D1322_drv.link_func = NULL;
//					D1322_drv.cnt_hz = 0;
//					//-------------------------------------------			
//					}
//				}
//				//!!! ������� 74HC4094D ���������� � ������ � ���� �� SPI
//				// �������, �� ��������� �������� ������ �� �������, ���������� ��������� �� 74HC4094D
//				//********** ���������� ���������� ***********
//				if(LD_obj.fl_ref)																										// !!! �� ������������ ����  if(D1322_drv.hspi == hspi)
//				{
//					if(LD_obj.stat == LD_STAT_TX_IDLE)
//					{
//						LD_PIN_STR_OFF();																								// ������� "��������" (������ ������ � ������� �� SPI)
//						__nop();
//						LD_PIN_STR_ON();
//						__nop();
//						LD_PIN_STR_OFF();	// !!! �������� !!! STR ������ ���� � ������ ������ �� ��������� ��������, ����� ����� ���� �� �������						
////						LD_PIN_EO_ON();		
//						LD_obj.stat = LD_STAT_TX_OK;				
//						LD_obj.fl_ref = false;
//					}
//				}																																	 	
//				//-------------------------------------------				
//			}
//}// void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)

///**
//  * @brief 				������� ����������� ����� ���������� ������ ������ �� SPI
//	* @param  hspi: ��������� ��� ������ � SPI
//  */
//void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//	
//}

///**
//  * @brief 				������� ����������� ����� ���������� � �������� � ������ ������ �� SPI
//	* @param  hspi: ��������� ��� ������ � SPI
//  */
//void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//	if(DQT_dev.hspi == hspi)																									// ��� �������� ��������� ����������
//	if(DQT_dev.hspi->State == HAL_SPI_STATE_READY)
//		if(DQT_dev.spi_mode == DQT_TYPE_RX_TX_IT || 
//			 DQT_dev.spi_mode == DQT_TYPE_RX_TX_DMA)	
//		{	
//			DQT_dev.stat_rx = DQT_STAT_TXRX_RX_OK;								
//			DQT_dev.fl_cnt_tmo_rx = false;
//			DQT_dev.cnt_tmo_rx = 0;
//			
//			DQT_dev.stat_tx = DQT_STAT_TXRX_TX_OK;
//			DQT_dev.fl_cnt_tmo_tx = false;									
//			DQT_dev.cnt_tmo_tx = 0;
//			
//			DQT_PIN_SS_ON();
//			DQT_dev.alg_cmd = DQT_ALG_DRDU_IDLE;
//			DQT_dev.fl_cnt_tmo_drdu = true;
//		}
//}
