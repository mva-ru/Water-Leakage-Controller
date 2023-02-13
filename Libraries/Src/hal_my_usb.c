#include "hal_my_usb.h"

/***************************************************************************
							Объявляем глобальные переменные внутри модуля
****************************************************************************/
USB_Struct USB_port;
//--------------------------------------------------------------------------

/***************************************************************************
											Прототипы всех функций модуля
****************************************************************************/
void USB_Init(void);																												// Инициализация модуля
void USB_Handler(void);		 																									// Обработчик модуля (положить в Main)
void USB_Handler_Tm(void);																									// Обработчик интервалов для модуля (положить в таймер 1мс)

void EP3_OUT_Callback(void);																								// Прерывание для обработки принятых данных
//--------------------------------------------------------------------------

/***************************************************************************
									  Описание функций и методов модуля
****************************************************************************/
/**
	* @brief Инициализация модуля
  */
void USB_Init(void)
{
	//	uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
	//	uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];
	
	USB_port.hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	
	//	USBD_CDC_ReceivePacket(&hUsbDeviceFS); 	 // принять пакет с данными   (по умолчанию размер APP_RX_DATA_SIZE)
	//	USBD_CDC_TransmitPacket(&hUsbDeviceFS);  // отправить пакет с данными (по умолчанию размер APP_TX_DATA_SIZE)
	
	//			CDC_Receive_FS((u8*)ASCII_Port[n].bfRx, ASCII_Port[n].sizeRx);
//			CDC_Transmit_FS((u8*)ASCII_Port[n].bfTx, ASCII_Port[n].sizeTx);
	
//	uint32_t HAL_PCD_EP_GetRxCount(PCD_HandleTypeDef *hpcd, uint8_t ep_addr)
//{
//  return hpcd->OUT_ep[ep_addr & EP_ADDR_MSK].xfer_count;
//}
}

/** 
	* @brief Обработчик модуля (положить в Main)
  */
void USB_Handler(void)
{
	
}

/**
	* @brief Обработчик интервалов для модуля (положить в таймер 1мс)
  */
void USB_Handler_Tm(void)
{
	
}

/**
	* @brief Прерывание для обработки принятых данных
  */
void EP3_OUT_Callback(void)
{
  
}

//--------------------------------------------------------------------------
