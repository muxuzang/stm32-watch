#ifndef ESP8266_H
#define ESP8266_H
#include "bsp_usart.h"
#include "stm32f10x.h"

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

//串口3-USART3
#define  WIFI_USARTx                   USART3
#define  WIFI_USART_CLK                RCC_APB1Periph_USART3
#define  WIFI_USART_APBxClkCmd         RCC_APB1PeriphClockCmd
#define  WIFI_USART_BAUDRATE           115200

// USART GPIO 引脚宏定义
#define  WIFI_USART_GPIO_CLK           (RCC_APB2Periph_GPIOB)
#define  WIFI_USART_GPIO_APBxClkCmd    RCC_APB2PeriphClockCmd
    
#define  WIFI_USART_TX_GPIO_PORT       GPIOB   
#define  WIFI_USART_TX_GPIO_PIN        GPIO_Pin_10
#define  WIFI_USART_RX_GPIO_PORT       GPIOB
 #define  WIFI_USART_RX_GPIO_PIN        GPIO_Pin_11

#define  WIFI_USART_IRQ                USART3_IRQn
#define  WIFI_USART_IRQHandler         USART3_IRQHandler


void ESP8266_init(void);
void ESP8266_Test(void);
void WIFI_Config(void);


unsigned char *ESP8266_GetIPD_GET(unsigned short timeOut,u8 *buff);

void ESP8266_Clear(void);
_Bool ESP8266_WaitRecive(void);

_Bool ESP8266_SendCmd(char *cmd, char *res);
void ESP8266_SendData(unsigned char *data, unsigned short len);

void ESP8266_Init(void);
void Get_current_time(void);





//解析苏宁返回数据
void cJSON_Time_Parse(void);
int Get_Year(char *y);
int Get_Moonth(char *m);
int Get_Day(char *d);
int Get_Times(char *h, char *m, char *s);
#endif

