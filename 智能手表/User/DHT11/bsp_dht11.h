#ifndef __BSP_DHT11_H
#define	__BSP_DHT11_H

#include "stm32f10x.h"

/************************** DHT11 数据类型定义********************************/
typedef struct
{
	uint8_t  humi_int;		//湿度的整数部分
	uint8_t  humi_deci;	 	//湿度的小数部分
	uint8_t  temp_int;	 	//温度的整数部分
	uint8_t  temp_deci;	 	//温度的小数部分
	uint8_t  check_sum;	 	//校验和
		                 
} DHT11_Data_TypeDef;

// DHT11
#define DHT11_GPIO_PORT    	GPIOE			              /* GPIO端口 */
#define DHT11_GPIO_CLK 	    RCC_APB2Periph_GPIOE		/* GPIO端口时钟 */
#define DHT11_GPIO_PIN		GPIO_Pin_6			        /* 连接到SCL时钟线的GPIO */
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
		



















