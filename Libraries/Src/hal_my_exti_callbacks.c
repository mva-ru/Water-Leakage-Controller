#include "hal_my_exti_callbacks.h"
#include "hal_nrf24l01_drv.h"
	
/**********************************************************************
									��������� ���� ������� � ������� ������
***********************************************************************/
void HAL_GPIO_EXTI_Callback(u16 GPIO_Pin);															// Callback ������� - ���������� �� ����� ��� ���������� ������
//---------------------------------------------------------------------	

//*********************************************************************
//								�������� ���� ������� � ������� ������
//*********************************************************************
/**
	* @brief  					Callback ������� - ���������� �� ����� ��� ���������� ������
	* @param  GPIO_Pin: ���� �� ������� ��������� ����������
  */
void HAL_GPIO_EXTI_Callback(u16 GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_1)
		NRF24_dev.alg = NRF24_ALG_IRQ;
}
