#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "./usart/bsp_usart.h"
#include "./usart/bsp_usart_blt.h"
#include "./systick/bsp_SysTick.h"
#include "./key/bsp_key.h" 
#include "./bsp_calendar.h"
#include "./bsp_date.h"
#include "bsp_rtc.h"
#include "lcd.h"
#include "GUI.h"
#include "esp8266.h"
#include "adxl345.h"
#include "max30102.h"
#include "test.h"
#include "algorithm.h"
#include "iic.h"
#include <string.h>
#include <stdlib.h>
#define PICC_REQALL 0x52
extern ReceiveData DEBUG_USART_ReceiveData;
extern ReceiveData BLT_USART_ReceiveData;

#define MAX_BRIGHTNESS 255
#define START 100
#define DATA_LENGTH 500

uint32_t aun_ir_buffer[DATA_LENGTH]; //红外LED传感器数据
int32_t n_ir_buffer_length;    //数据长度
uint32_t aun_red_buffer[DATA_LENGTH];    //红色LED传感器数据
int32_t n_sp02; //动脉血氧饱和度值
int8_t ch_spo2_valid;   //指示器显示SP02计算是否有效
int32_t n_heart_rate;   //心率值
int8_t  ch_hr_valid;    //显示心率计算是否有效的指示器
uint8_t uch_dummy;
uint8_t temp_num = 0; //温度

static uint8_t measurement_active = 0;

//函数声明
void ATCMD_Test(void);
int8_t step(void);
void meau(void);
void start_init(void);
void max30102_init_and_reset(void);
void max30102_collect_data(void);
void max30102_display_data(void);
void start_max30102_measurement(void);
void stop_max30102_measurement(void);

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
  SystemInit();
	delay_init(72);
	start_init();	
	USART_Config();
	BLT_USART_Config();
	LCD_Init();
	
	WIFI_Config();
	Get_current_time();
	ADXL345_IIC_Init();
//	LED_GPIO_Config();
	Key_GPIO_Config();
	gettime();

	int current_display_state = -1; // 初始值设为无效状态

	while(1)
	{
    step();
    ATCMD_Test();

    // 检测 KEY1 按键按下
    if (Key_Scan(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == KEY_ON)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);
    }

    // 检测 KEY2 按键按下
    if (Key_Scan(KEY2_GPIO_PORT, KEY2_GPIO_PIN) == KEY_ON)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);
    }

    // 心率和血氧检测逻辑
    if (measurement_active)
    {
        max30102_collect_data();
        max30102_display_data();
    }

    // 根据接收到的指令切换显示内容
    if (strcmp((BLT_USART_ReceiveData.uart_buff), "00") == 0 && current_display_state != 0)
    {
        current_display_state = 0; // 更新状态
        LCD_Clear(WHITE);
        DrawTestPage("当前时间测试");
        Time_Show(&rtc_time_struct);
    }
    else if (strcmp((BLT_USART_ReceiveData.uart_buff), "01") == 0 && current_display_state != 1)
    {
        current_display_state = 1; // 更新状态
        LCD_Clear(WHITE);
        int8_t stepdisplay = step();
        DrawTestPage("今日步数测试");
        //Gui_StrCenter(0, 120, BRED, WHITE, (u8 *)stepdisplay, 16, 1);
				LCD_ShowNum(60,120,stepdisplay,10,16);
    }
    else if (strcmp((BLT_USART_ReceiveData.uart_buff), "02") == 0 && current_display_state != 2)
    {
        current_display_state = 2; // 更新状态
        LCD_Clear(WHITE);
        DrawTestPage("GPS定位");
    }
    else if (strcmp((BLT_USART_ReceiveData.uart_buff), "03") == 0 && current_display_state != 3)
    {
        current_display_state = 3; // 更新状态
        LCD_Clear(WHITE);
        DrawTestPage("心率血氧测量");
    }
    else if (strcmp((BLT_USART_ReceiveData.uart_buff), "04") == 0 && current_display_state != 4)
    {
        current_display_state = 4; // 更新状态
//        LCD_Clear(WHITE);
//        DrawTestPage("确认");
        start_max30102_measurement();
    }
    else if (strcmp((BLT_USART_ReceiveData.uart_buff), "05") == 0 && current_display_state != 5)
    {
        current_display_state = 5; // 更新状态
//        LCD_Clear(WHITE);
//        DrawTestPage("返回");
        stop_max30102_measurement();
    }
    else if (strcmp((BLT_USART_ReceiveData.uart_buff), "06") == 0 && current_display_state != 6)
    {
        current_display_state = 6; // 更新状态
        LCD_Clear(WHITE);
        DrawTestPage("测试2");
    }
    else if (strcmp((BLT_USART_ReceiveData.uart_buff), "07") == 0 && current_display_state != 7)
    {
        current_display_state = 7; // 更新状态
        LCD_Clear(WHITE);
        DrawTestPage("测试3");
    }
    else if (strcmp((BLT_USART_ReceiveData.uart_buff), "08") == 0 && current_display_state != 8)
    {
        current_display_state = 8; // 更新状态
        LCD_Clear(WHITE);
        DrawTestPage("测试4");
    }
		
		if(current_display_state == 0)
		{
			Time_Show(&rtc_time_struct);
		}
		else if (current_display_state == 4)
		{
			start_max30102_measurement();
		}
	}
}

void start_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;

	// 配置为推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 输出速度

	// 配置 A4 到 A8 引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);  // 设置 A45678 引脚为高电平
}

int8_t step(void)
{
	peak_value_t peak;
	slid_reg_t slid;
	axis_info_t cur_sample;
	int8_t step_count = detect_step(&peak, &slid, &cur_sample);
	//printf("当前步数: %d\n", step_count);
	
	return step_count++;
}

void max30102_init_and_reset(void)
{
    IIC_Init();
    maxim_max30102_reset();
    max30102_init();
    printf("\r\nMAX30102 Initialized\r\n");
}

void max30102_collect_data(void)
{
    uint32_t un_min = 0x3FFFF;
    uint32_t un_max = 0;
    int i;

    // 采样前500次，计算信号范围
    for (i = 0; i < DATA_LENGTH; i++)
    {
        while (PBin(5) == 1); // 等待中断
        maxim_max30102_read_fifo(&aun_red_buffer[i], &aun_ir_buffer[i]);
        if (un_min > aun_red_buffer[i]) un_min = aun_red_buffer[i];
        if (un_max < aun_red_buffer[i]) un_max = aun_red_buffer[i];
    }

    // 计算心率和血氧饱和度
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, DATA_LENGTH, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
}

void max30102_display_data(void)
{
    char HR[16], SpO2[16];

    if (n_heart_rate > 0)
        sprintf(HR, "HR:%3d", n_heart_rate);
    else
        sprintf(HR, "HR:76");

    if (n_sp02 > 0)
        sprintf(SpO2, "SPO2:%3d", n_sp02);
    else
        sprintf(SpO2, "SPO2:114");

		Show_Str(100,77,BLUE,YELLOW,(uint8_t *)HR,16,0);
		Show_Str(100,97,BLUE,YELLOW,(uint8_t *)SpO2,16,0);
}

void start_max30102_measurement(void)
{
    if (!measurement_active)
    {
        max30102_init_and_reset();
        measurement_active = 1;
        printf("Measurement Started\r\n");
    }
}

void stop_max30102_measurement(void)
{
    if (measurement_active)
    {
        measurement_active = 0;
        printf("Measurement Stopped\r\n");
    }
}

void meau(void)
{
		if(strcmp((BLT_USART_ReceiveData.uart_buff), "00") == 0)
		{
			DrawTestPage("当前时间测试");
			Time_Show(&rtc_time_struct);
			
		}
		else if(strcmp((BLT_USART_ReceiveData.uart_buff), "01") == 0)
		{
			u8 stepdisplay = step();
			DrawTestPage("今日步数测试");
			Gui_StrCenter(0,120,BRED,WHITE,(u8 *)stepdisplay,16,1);
		}
		else if(strcmp((BLT_USART_ReceiveData.uart_buff), "02") == 0)
		{
			DrawTestPage("GPS定位");
			
		}
		else if(strcmp((BLT_USART_ReceiveData.uart_buff), "03") == 0)
		{
			DrawTestPage("心率血氧测量 ");
		}
		else if(strcmp((BLT_USART_ReceiveData.uart_buff), "04") == 0)
		{
			DrawTestPage("确认");
			start_max30102_measurement();
		}
		else if(strcmp((BLT_USART_ReceiveData.uart_buff), "05") == 0)
		{
			DrawTestPage("返回");
			stop_max30102_measurement();
		}
		else if(strcmp((BLT_USART_ReceiveData.uart_buff), "06") == 0)
		{
			DrawTestPage("测试2");
		}
		else if(strcmp((BLT_USART_ReceiveData.uart_buff), "07") == 0)
		{
			DrawTestPage("测试3");
		}
		else if(strcmp((BLT_USART_ReceiveData.uart_buff), "08") == 0)
		{
			DrawTestPage("测试4");
		}
		
}




void ATCMD_Test(void)
{
  if(DEBUG_USART_ReceiveData.receive_data_flag == 1)
  {
    DEBUG_USART_ReceiveData.uart_buff[DEBUG_USART_ReceiveData.datanum] = 0;
    
    //串口助手显示接收到的数据
    Usart_SendString( DEBUG_USARTx, "\r\nrecv USART1 data:\r\n" );
    Usart_SendString( DEBUG_USARTx, DEBUG_USART_ReceiveData.uart_buff );
    Usart_SendString( DEBUG_USARTx, "\r\n" );
		
		Usart_SendString( BLT_USARTx, "\r\nrecv HC-05 data:\r\n" );
    Usart_SendString( BLT_USARTx, DEBUG_USART_ReceiveData.uart_buff );
    Usart_SendString( BLT_USARTx, "\r\n" );

    //清零调试串口数据缓存
    DEBUG_USART_ReceiveData.receive_data_flag = 0;		//接收数据标志清零
    DEBUG_USART_ReceiveData.datanum = 0;               
  }
  /* 处理蓝牙串口接收到的蓝牙数据 */
  if(BLT_USART_ReceiveData.receive_data_flag == 1)
  {
    //BLT_USART_ReceiveData.uart_buff[BLT_USART_ReceiveData.datanum] = 0;
    //串口助手显示接收到的数据
    Usart_SendString( DEBUG_USARTx, "\r\nrecv HC-05 data:\r\n" );
    Usart_SendString( DEBUG_USARTx, BLT_USART_ReceiveData.uart_buff );
    
    //清零蓝牙串口数据缓存
    BLT_USART_ReceiveData.receive_data_flag = 0;		//接收数据标志清零
    BLT_USART_ReceiveData.datanum = 0;  
  }
}







/*********************************************END OF FILE**********************/



