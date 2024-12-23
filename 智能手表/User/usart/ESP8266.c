#include "ESP8266.h"
#include "bsp_usart.h"	
//#include "SysTick.h"
#include "delay.h"
#include <stdlib.h>
#include <string.h>

/*用于保存小时，分钟，秒数的变量*/
int hour_return;//小时
int min_return; //分钟
int sec_return; //秒数

//WIFI和密码·
#define ESP8266_WIFI_INFO		"AT+CWJAP=\"mumu\",\"152701293\"\r\n"

/*
//苏宁后台获取时间的API
define Time_TCP		"AT+CIPSTART=\"TCP\",\"quan.suning.com\",80\r\n"
//苏宁后台获取时间GET报文
#define Time_GET		"GET http://quan.suning.com/getSysTime.do\r\n"
var response_taobao = http.get("http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp");

*/

//#define Time_TCP "GET http://api.k780.com:88/?app=life.time&appkey=10003&sign=b59bc3ef6191eb9f747dd4e83c99f2a4&format=json\r\n"

#define Time_TCP		"AT+CIPSTART=\"TCP\",\"quan.suning.com\",80\r\n"
#define Time_GET "GET http://quan.suning.com/getSysTime.do HTTP/1.1\r\nHost: quan.suning.com\r\n\r\n"

//ESP8266数据存放
unsigned char esp8266_buf[2000] = {0};
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
//存放时间数据
unsigned char Time_buff[1024];   //位数是随机确定的



//串口3 
void USART3_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开串口GPIO的时钟 
	WIFI_USART_GPIO_APBxClkCmd(WIFI_USART_GPIO_CLK , ENABLE);
	
	// 打开串口外设的时钟
	WIFI_USART_APBxClkCmd(WIFI_USART_CLK, ENABLE);

	// 将USART Tx的GPIO配置为推挽复用模式	
	GPIO_InitStructure.GPIO_Pin = WIFI_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(WIFI_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  // 将USART Rx的GPIO配置为浮空输入模式		
	GPIO_InitStructure.GPIO_Pin = WIFI_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(WIFI_USART_RX_GPIO_PORT, &GPIO_InitStructure);
	
	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = WIFI_USART_BAUDRATE;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(WIFI_USARTx, &USART_InitStructure);
	
	 NVIC_InitTypeDef NVIC_InitStructure;
  
//  /* 嵌套向量中断控制器组选择 */
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* 配置USART为中断源 */
  NVIC_InitStructure.NVIC_IRQChannel = WIFI_USART_IRQ;
  /* 抢断优先级*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  /* 子优先级 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  /* 使能中断 */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* 初始化配置NVIC */
  NVIC_Init(&NVIC_InitStructure);
	
	// 使能串口接收中断
	USART_ITConfig(WIFI_USARTx, USART_IT_RXNE , ENABLE);	
	
	// 使能发送中断
	//USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	
	// 使能串口
	USART_Cmd(WIFI_USARTx, ENABLE);	   
	
//	//拉高PB8
//	GPIO_SetBits(OUT_GPIO_PORT, OUT_GPIO_PIN);
//	LED_G(ON);
//    //GPIO_ResetBits(OUT_GPIO_PORT, OUT_GPIO_PIN);
	
}

void ESP8266_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;	//声明一个结构体变量，用来初始化GPIO

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);   /* 开启GPIO时钟 */

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    //GPIO_InitStructure.GPIO_Pull = GPIO_NOPULL; // 不使用上拉/下拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // 设置为低速
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/*  配置GPIO的模式和IO口 */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;		//选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;		  //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOB,&GPIO_InitStructure); 	 /* 初始化GPIO */
    //PBout(8)=1;
	//拉高PB8
	GPIO_SetBits(GPIOB, GPIO_Pin_8);
	
//	LED_G(ON);
    //GPIO_ResetBits(OUT_GPIO_PORT, OUT_GPIO_PIN);
}


void ESP8266_Test()
{ 
    Usart_SendString(USART3,"AT\r\n");
    delay_us(500000);
//    Usart_SendString(USART3,"AT+RST\r\n");
//    delay_ms(500);
    Usart_SendString(USART3,"AT+CWMODE=3\r\n");
    delay_us(500000);
    Usart_SendString(USART3,"AT+CWSAP=\"ESP8266_001\",\"1234567890\",1,3\r\n");
    delay_us(500000);
}
void WIFI_Config(void)
{
 USART3_Config();
 ESP8266_init();

}

void ESP8266_Init(void)
{
    ESP8266_Clear();//清除缓冲

    /*让WIFI退出透传模式 要发两次*/
    while(ESP8266_SendCmd("+++", ""));//这是一个死循环，目的结束透传模式

    /*让WIFI清除Flah*/
    printf("0.恢复出厂设置成功\r\n");

while(ESP8266_SendCmd("AT+RESTORE\r\n", "OK"));//恢复出厂设置

    //初始AT应答，看wifi接线是否成功
    printf("1.AT应答成功\r\n");

    while(ESP8266_SendCmd("AT\r\n", "OK"));

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //加一步ESP8266复位操作
    printf("2.RST复位成功\r\n");
  while(  ESP8266_SendCmd("AT+RST\r\n", ""));
    delay_ms(500);
  while(  ESP8266_SendCmd("AT+CIPCLOSE\r\n", ""));//断开与服务器的连接
    delay_ms(500);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    printf("3.CWMODE设置工作模式，保存到Flash\r\n");

while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"));//没有CUR就是保存到Flash，AT+CWMODE_CUR设置模块工作模式为station，不保存到Flash

    printf("4.AT+CIPMUX单连接模式设置成功\r\n");

  while(  ESP8266_SendCmd("AT+CIPMUX=0\r\n", "OK"));//AT+CIPMUX=0 设置为单连接模式

    printf("5.寻找对应的WIFI名称和密码\r\n");

    while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "OK"));

    printf("6.ESP8266_Init连接WIFI成功\r\n");
}



/**************************************************************************/
//函数作用：获取苏宁后台时间
//函数名称：Get_current_time();
//内部参数：
//修改日期：2022年4月18日  晚上20：32
//作者：    大屁桃
/**************************************************************************/
void Get_current_time(void)
{
    ESP8266_Init();	     //连接Wifi的ESP8266初始化
    ESP8266_Clear();

    while(ESP8266_SendCmd(Time_TCP, "CONNECT"));

    printf("6.访问苏宁服务器成功 OK\r\n");

    while(ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK"));//开启透传模式

    printf("7.开启透传模式成功 OK\r\n");

    /*sizeof(Time_GET)，必须用sizeof函数，用strlen没有用*/
    ESP8266_SendData((u8 *)Time_GET, sizeof(Time_GET)); //发送AT+CIPSEND  以及 Time_GET
    printf("8.9 数据获取成功\r\n");
    ESP8266_GetIPD_GET(300, Time_buff); //将串口数据取出来
	  printf("8.99 提取数据成功\r\n");
    ESP8266_Clear();//清除缓存数据
    while(ESP8266_SendCmd("+++", ""));      /*退出透传模式，发送两次*/
    printf("9.退出透传模式成功 OK\r\n");

}


/*******************************************解析时间*************************************/
//代码来源于CSDN博客地址：https://blog.csdn.net/xgy516/article/details/119968124
/****************************************************************************************
年的首地址移动11位；
月份首地址移动15位；
日期首地址移动17位；
小时首地址移动19位；
分钟首地址移动21位；
秒钟首地址移动23位；
*/
#define  YEAR_ADD_DRES 11
#define  MOON_ADD_DRES 15
#define  DAYS_ADD_DRES 17

#define  HOURS_ADD_DRES 19
#define  MINUTES_ADD_DRES 21
#define  SECONDS_ADD_DRES 23

int DAYS, MOONS, YEARS, TIMES;
///**************************************************************************/
////函数作用：解析苏宁时间函数
////函数名称：cJSON_Time_Parse();
////内部参数：
////修改日期：2022年4月18日  下午22：11
///**************************************************************************/
void cJSON_Time_Parse(void)
{
    char *data_pt;
    char *day_string;
    char *moon_string;
    char *year_string;
    char *hour_string;
    char *minute_string;
    char *second_string;


    data_pt = strstr((const char *)Time_buff, (const char *)"sysTime1");  //寻找到时间结果的地址

//    printf("%s\r\n",Time_buff);
    if(data_pt != NULL)
    {
        day_string = data_pt + DAYS_ADD_DRES;          //日期地址
        moon_string = data_pt + MOON_ADD_DRES;         //月份地址
        year_string = data_pt + YEAR_ADD_DRES;         //年份地址
        hour_string = data_pt + HOURS_ADD_DRES;        //小时地址
        minute_string = data_pt + MINUTES_ADD_DRES;    //分钟地址
        second_string = data_pt + SECONDS_ADD_DRES;    //秒中地址

        //将时间信息传递给全局变量
        DAYS = Get_Day(day_string);
        MOONS = Get_Moonth(moon_string);
        YEARS = Get_Year(year_string);
        TIMES = Get_Times(hour_string, minute_string, second_string);
		
		hour_return = TIMES/3600;//小时
		min_return = (TIMES%3600)/60; //分钟
        sec_return = (TIMES%3600)%60; //秒数
		printf("时间获取并处理成功\r\n");

    }
    else
    {
        printf("时间获取失败\r\n");
    }
}

//得到年函数（以年开始的字符串长度过长，因此使用不一样的方法）
//输入值是年位置的地址
//返回值是 整型的10进制四位数
int Get_Year(char *y)
{

    int year_return;
    char *year_temp;
    char year[5] = {0};
    char i;
//年的获取须要提取一次字符串，不然没法读取
    year_temp = y;

    for(i = 0; i < 4; i++)
    {
        year[i] = *year_temp;
        year_temp ++;
    }

    year_return =  atoi(&year[0]);
    return year_return;
}

//得到月份函数
//输入值是月份位置的地址
//返回值是 整型的10进制两位数
int Get_Moonth(char *m)
{
    int moonth_return;
    moonth_return = atoi(m) / 100000000; //取月份
    return moonth_return;
}

//得到日期函数
//输入值是日期位置的地址
//返回值是 整型的10进制两位数
int Get_Day(char *d)
{


    int day_return;
    day_return = atoi(d) / 1000000; //取日期

    return day_return;
}

//得到时间
//输入值是时间的位置的地址
//返回值是 整型的10进制的时间总秒数
int Get_Times(char *h, char *m, char *s)
{
    int time_return;
    int hour_return;
    int min_return;
    int sec_return;

    hour_return = atoi(h) / 10000; //取小时
    min_return = atoi(m) / 100; //取分钟
    sec_return = atoi(s);   //取秒数

    time_return = hour_return * 3600 + min_return * 60 + sec_return; //转换成总秒数

    return time_return;
}

//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_Clear(void)
{

    memset(esp8266_buf, 0, sizeof(esp8266_buf));
    esp8266_cnt = 0;

}
//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	data：数据
//				len：长度
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{
    char cmdBuf[32];

    ESP8266_Clear();								//清空接收缓存
    sprintf(cmdBuf, "AT+CIPSEND\r\n");		//发送命令

    if(!ESP8266_SendCmd(cmdBuf, ">"))				//收到‘>’时可以发送数据
    {
        printf("8.开始处于透传发送状态!\r\n");
        /*发送请求数据*/
        Usart_SendString(USART3, data);		//发送设备连接请求数据
    }
		        printf("8.5.开始处于透传发送完毕!\r\n");

}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	copy天气数据到buff数组里面
//
//	返回参数：	平台返回的原始数据
//
//	说明：		copy天气数据到buff
//==========================================================

unsigned char *ESP8266_GetIPD_GET(unsigned short timeOut, u8 *buff) //这里我用了一个全局变量将esp8266buf储存到这个全局变量里面
{
    do
    {
        delay_ms(5);
    }
    while(timeOut--);

    strcpy((char*)buff, (char*)esp8266_buf);
		printf("提取中%d",timeOut);
    return buff;
}



//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

    if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
        return REV_WAIT;

    if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
    {
        esp8266_cnt = 0;							//清0接收计数

        return REV_OK;								//返回接收完成标志
    }

    esp8266_cntPre = esp8266_cnt;					//置为相同

    return REV_WAIT;								//返回接收未完成标志

}


//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{

    unsigned char timeOut = 250;

    Usart_SendString(USART3, (char *)cmd);

    while(timeOut--)
    {
        if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
        {
            if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
            {
                ESP8266_Clear();									//清空缓存

                return 0;
            }
        }

        delay_ms(10);
    }
    return 1;
}

void WIFI_USART_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //接收中断
    {

        if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆
        Usart_SendByte(USART1,USART_ReceiveData(USART3));
        esp8266_buf[esp8266_cnt++] = USART3->DR;

//		USART_SendData(USART1,USART2->DR);      //让接收到的数据打印在串口一上

        USART_ClearFlag(USART3, USART_FLAG_RXNE);
    }
}






