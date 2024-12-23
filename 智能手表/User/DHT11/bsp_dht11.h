#ifndef __BSP_DHT11_H
#define	__BSP_DHT11_H

#include "stm32f10x.h"

/************************** DHT11 �������Ͷ���********************************/
typedef struct
{
	uint8_t  humi_int;		//ʪ�ȵ���������
	uint8_t  humi_deci;	 	//ʪ�ȵ�С������
	uint8_t  temp_int;	 	//�¶ȵ���������
	uint8_t  temp_deci;	 	//�¶ȵ�С������
	uint8_t  check_sum;	 	//У���
		                 
} DHT11_Data_TypeDef;

// DHT11
#define DHT11_GPIO_PORT    	GPIOE			              /* GPIO�˿� */
#define DHT11_GPIO_CLK 	    RCC_APB2Periph_GPIOE		/* GPIO�˿�ʱ�� */
#define DHT11_GPIO_PIN		GPIO_Pin_6			        /* ���ӵ�SCLʱ���ߵ�GPIO */
#define DHT11_Dout_IN()	  GPIO_ReadInputDataBit ( DHT11_GPIO_PORT, DHT11_GPIO_PIN ) 


#define ON  0
#define OFF 1
#define DHT11(a)	if (a)	\
					GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN);\
					else		\
					GPIO_ResetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN)
					
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data);
void DHT11_Init ( void );
void DHT11_GPIO_Config(void);
					
#endif /* __BSP_DHT11_H */
		



















