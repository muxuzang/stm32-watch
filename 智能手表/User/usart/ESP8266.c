#include "ESP8266.h"
#include "bsp_usart.h"	
//#include "SysTick.h"
#include "delay.h"
#include <stdlib.h>
#include <string.h>

/*���ڱ���Сʱ�����ӣ������ı���*/
int hour_return;//Сʱ
int min_return; //����
int sec_return; //����

//WIFI�����롤
#define ESP8266_WIFI_INFO		"AT+CWJAP=\"mumu\",\"152701293\"\r\n"

/*
//������̨��ȡʱ���API
define Time_TCP		"AT+CIPSTART=\"TCP\",\"quan.suning.com\",80\r\n"
//������̨��ȡʱ��GET����
#define Time_GET		"GET http://quan.suning.com/getSysTime.do\r\n"
var response_taobao = http.get("http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp");

*/

//#define Time_TCP "GET http://api.k780.com:88/?app=life.time&appkey=10003&sign=b59bc3ef6191eb9f747dd4e83c99f2a4&format=json\r\n"

#define Time_TCP		"AT+CIPSTART=\"TCP\",\"quan.suning.com\",80\r\n"
#define Time_GET "GET http://quan.suning.com/getSysTime.do HTTP/1.1\r\nHost: quan.suning.com\r\n\r\n"

//ESP8266���ݴ��
unsigned char esp8266_buf[2000] = {0};
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
//���ʱ������
unsigned char Time_buff[1024];   //λ�������ȷ����



//����3 
void USART3_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// �򿪴���GPIO��ʱ�� 
	WIFI_USART_GPIO_APBxClkCmd(WIFI_USART_GPIO_CLK , ENABLE);
	
	// �򿪴��������ʱ��
	WIFI_USART_APBxClkCmd(WIFI_USART_CLK, ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ	
	GPIO_InitStructure.GPIO_Pin = WIFI_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(WIFI_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  // ��USART Rx��GPIO����Ϊ��������ģʽ		
	GPIO_InitStructure.GPIO_Pin = WIFI_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(WIFI_USART_RX_GPIO_PORT, &GPIO_InitStructure);
	
	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = WIFI_USART_BAUDRATE;
	// ���� �������ֳ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// ����ֹͣλ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// ����У��λ
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// ����Ӳ��������
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// ���ù���ģʽ���շ�һ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// ��ɴ��ڵĳ�ʼ������
	USART_Init(WIFI_USARTx, &USART_InitStructure);
	
	 NVIC_InitTypeDef NVIC_InitStructure;
  
//  /* Ƕ�������жϿ�������ѡ�� */
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* ����USARTΪ�ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = WIFI_USART_IRQ;
  /* �������ȼ�*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  /* �����ȼ� */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  /* ʹ���ж� */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* ��ʼ������NVIC */
  NVIC_Init(&NVIC_InitStructure);
	
	// ʹ�ܴ��ڽ����ж�
	USART_ITConfig(WIFI_USARTx, USART_IT_RXNE , ENABLE);	
	
	// ʹ�ܷ����ж�
	//USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	
	// ʹ�ܴ���
	USART_Cmd(WIFI_USARTx, ENABLE);	   
	
//	//����PB8
//	GPIO_SetBits(OUT_GPIO_PORT, OUT_GPIO_PIN);
//	LED_G(ON);
//    //GPIO_ResetBits(OUT_GPIO_PORT, OUT_GPIO_PIN);
	
}

void ESP8266_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;	//����һ���ṹ�������������ʼ��GPIO

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);   /* ����GPIOʱ�� */

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // ��������
    //GPIO_InitStructure.GPIO_Pull = GPIO_NOPULL; // ��ʹ������/����
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // ����Ϊ����
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/*  ����GPIO��ģʽ��IO�� */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;		//ѡ����Ҫ���õ�IO��
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;		  //�����������ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //���ô�������
	GPIO_Init(GPIOB,&GPIO_InitStructure); 	 /* ��ʼ��GPIO */
    //PBout(8)=1;
	//����PB8
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
    ESP8266_Clear();//�������

    /*��WIFI�˳�͸��ģʽ Ҫ������*/
    while(ESP8266_SendCmd("+++", ""));//����һ����ѭ����Ŀ�Ľ���͸��ģʽ

    /*��WIFI���Flah*/
    printf("0.�ָ��������óɹ�\r\n");

while(ESP8266_SendCmd("AT+RESTORE\r\n", "OK"));//�ָ���������

    //��ʼATӦ�𣬿�wifi�����Ƿ�ɹ�
    printf("1.ATӦ��ɹ�\r\n");

    while(ESP8266_SendCmd("AT\r\n", "OK"));

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //��һ��ESP8266��λ����
    printf("2.RST��λ�ɹ�\r\n");
  while(  ESP8266_SendCmd("AT+RST\r\n", ""));
    delay_ms(500);
  while(  ESP8266_SendCmd("AT+CIPCLOSE\r\n", ""));//�Ͽ��������������
    delay_ms(500);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    printf("3.CWMODE���ù���ģʽ�����浽Flash\r\n");

while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"));//û��CUR���Ǳ��浽Flash��AT+CWMODE_CUR����ģ�鹤��ģʽΪstation�������浽Flash

    printf("4.AT+CIPMUX������ģʽ���óɹ�\r\n");

  while(  ESP8266_SendCmd("AT+CIPMUX=0\r\n", "OK"));//AT+CIPMUX=0 ����Ϊ������ģʽ

    printf("5.Ѱ�Ҷ�Ӧ��WIFI���ƺ�����\r\n");

    while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "OK"));

    printf("6.ESP8266_Init����WIFI�ɹ�\r\n");
}



/**************************************************************************/
//�������ã���ȡ������̨ʱ��
//�������ƣ�Get_current_time();
//�ڲ�������
//�޸����ڣ�2022��4��18��  ����20��32
//���ߣ�    ��ƨ��
/**************************************************************************/
void Get_current_time(void)
{
    ESP8266_Init();	     //����Wifi��ESP8266��ʼ��
    ESP8266_Clear();

    while(ESP8266_SendCmd(Time_TCP, "CONNECT"));

    printf("6.���������������ɹ� OK\r\n");

    while(ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK"));//����͸��ģʽ

    printf("7.����͸��ģʽ�ɹ� OK\r\n");

    /*sizeof(Time_GET)��������sizeof��������strlenû����*/
    ESP8266_SendData((u8 *)Time_GET, sizeof(Time_GET)); //����AT+CIPSEND  �Լ� Time_GET
    printf("8.9 ���ݻ�ȡ�ɹ�\r\n");
    ESP8266_GetIPD_GET(300, Time_buff); //����������ȡ����
	  printf("8.99 ��ȡ���ݳɹ�\r\n");
    ESP8266_Clear();//�����������
    while(ESP8266_SendCmd("+++", ""));      /*�˳�͸��ģʽ����������*/
    printf("9.�˳�͸��ģʽ�ɹ� OK\r\n");

}


/*******************************************����ʱ��*************************************/
//������Դ��CSDN���͵�ַ��https://blog.csdn.net/xgy516/article/details/119968124
/****************************************************************************************
����׵�ַ�ƶ�11λ��
�·��׵�ַ�ƶ�15λ��
�����׵�ַ�ƶ�17λ��
Сʱ�׵�ַ�ƶ�19λ��
�����׵�ַ�ƶ�21λ��
�����׵�ַ�ƶ�23λ��
*/
#define  YEAR_ADD_DRES 11
#define  MOON_ADD_DRES 15
#define  DAYS_ADD_DRES 17

#define  HOURS_ADD_DRES 19
#define  MINUTES_ADD_DRES 21
#define  SECONDS_ADD_DRES 23

int DAYS, MOONS, YEARS, TIMES;
///**************************************************************************/
////�������ã���������ʱ�亯��
////�������ƣ�cJSON_Time_Parse();
////�ڲ�������
////�޸����ڣ�2022��4��18��  ����22��11
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


    data_pt = strstr((const char *)Time_buff, (const char *)"sysTime1");  //Ѱ�ҵ�ʱ�����ĵ�ַ

//    printf("%s\r\n",Time_buff);
    if(data_pt != NULL)
    {
        day_string = data_pt + DAYS_ADD_DRES;          //���ڵ�ַ
        moon_string = data_pt + MOON_ADD_DRES;         //�·ݵ�ַ
        year_string = data_pt + YEAR_ADD_DRES;         //��ݵ�ַ
        hour_string = data_pt + HOURS_ADD_DRES;        //Сʱ��ַ
        minute_string = data_pt + MINUTES_ADD_DRES;    //���ӵ�ַ
        second_string = data_pt + SECONDS_ADD_DRES;    //���е�ַ

        //��ʱ����Ϣ���ݸ�ȫ�ֱ���
        DAYS = Get_Day(day_string);
        MOONS = Get_Moonth(moon_string);
        YEARS = Get_Year(year_string);
        TIMES = Get_Times(hour_string, minute_string, second_string);
		
		hour_return = TIMES/3600;//Сʱ
		min_return = (TIMES%3600)/60; //����
        sec_return = (TIMES%3600)%60; //����
		printf("ʱ���ȡ������ɹ�\r\n");

    }
    else
    {
        printf("ʱ���ȡʧ��\r\n");
    }
}

//�õ��꺯�������꿪ʼ���ַ������ȹ��������ʹ�ò�һ���ķ�����
//����ֵ����λ�õĵ�ַ
//����ֵ�� ���͵�10������λ��
int Get_Year(char *y)
{

    int year_return;
    char *year_temp;
    char year[5] = {0};
    char i;
//��Ļ�ȡ��Ҫ��ȡһ���ַ�������Ȼû����ȡ
    year_temp = y;

    for(i = 0; i < 4; i++)
    {
        year[i] = *year_temp;
        year_temp ++;
    }

    year_return =  atoi(&year[0]);
    return year_return;
}

//�õ��·ݺ���
//����ֵ���·�λ�õĵ�ַ
//����ֵ�� ���͵�10������λ��
int Get_Moonth(char *m)
{
    int moonth_return;
    moonth_return = atoi(m) / 100000000; //ȡ�·�
    return moonth_return;
}

//�õ����ں���
//����ֵ������λ�õĵ�ַ
//����ֵ�� ���͵�10������λ��
int Get_Day(char *d)
{


    int day_return;
    day_return = atoi(d) / 1000000; //ȡ����

    return day_return;
}

//�õ�ʱ��
//����ֵ��ʱ���λ�õĵ�ַ
//����ֵ�� ���͵�10���Ƶ�ʱ��������
int Get_Times(char *h, char *m, char *s)
{
    int time_return;
    int hour_return;
    int min_return;
    int sec_return;

    hour_return = atoi(h) / 10000; //ȡСʱ
    min_return = atoi(m) / 100; //ȡ����
    sec_return = atoi(s);   //ȡ����

    time_return = hour_return * 3600 + min_return * 60 + sec_return; //ת����������

    return time_return;
}

//	�������ƣ�	ESP8266_Clear
//
//	�������ܣ�	��ջ���
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����
//==========================================================
void ESP8266_Clear(void)
{

    memset(esp8266_buf, 0, sizeof(esp8266_buf));
    esp8266_cnt = 0;

}
//==========================================================
//	�������ƣ�	ESP8266_SendData
//
//	�������ܣ�	��������
//
//	��ڲ�����	data������
//				len������
//
//	���ز�����	��
//
//	˵����
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{
    char cmdBuf[32];

    ESP8266_Clear();								//��ս��ջ���
    sprintf(cmdBuf, "AT+CIPSEND\r\n");		//��������

    if(!ESP8266_SendCmd(cmdBuf, ">"))				//�յ���>��ʱ���Է�������
    {
        printf("8.��ʼ����͸������״̬!\r\n");
        /*������������*/
        Usart_SendString(USART3, data);		//�����豸������������
    }
		        printf("8.5.��ʼ����͸���������!\r\n");

}

//==========================================================
//	�������ƣ�	ESP8266_GetIPD
//
//	�������ܣ�	copy�������ݵ�buff��������
//
//	���ز�����	ƽ̨���ص�ԭʼ����
//
//	˵����		copy�������ݵ�buff
//==========================================================

unsigned char *ESP8266_GetIPD_GET(unsigned short timeOut, u8 *buff) //����������һ��ȫ�ֱ�����esp8266buf���浽���ȫ�ֱ�������
{
    do
    {
        delay_ms(5);
    }
    while(timeOut--);

    strcpy((char*)buff, (char*)esp8266_buf);
		printf("��ȡ��%d",timeOut);
    return buff;
}



//==========================================================
//	�������ƣ�	ESP8266_WaitRecive
//
//	�������ܣ�	�ȴ��������
//
//	��ڲ�����	��
//
//	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
//
//	˵����		ѭ�����ü���Ƿ�������
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

    if(esp8266_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
        return REV_WAIT;

    if(esp8266_cnt == esp8266_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
    {
        esp8266_cnt = 0;							//��0���ռ���

        return REV_OK;								//���ؽ�����ɱ�־
    }

    esp8266_cntPre = esp8266_cnt;					//��Ϊ��ͬ

    return REV_WAIT;								//���ؽ���δ��ɱ�־

}


//==========================================================
//	�������ƣ�	ESP8266_SendCmd
//
//	�������ܣ�	��������
//
//	��ڲ�����	cmd������
//				res����Ҫ���ķ���ָ��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{

    unsigned char timeOut = 250;

    Usart_SendString(USART3, (char *)cmd);

    while(timeOut--)
    {
        if(ESP8266_WaitRecive() == REV_OK)							//����յ�����
        {
            if(strstr((const char *)esp8266_buf, res) != NULL)		//����������ؼ���
            {
                ESP8266_Clear();									//��ջ���

                return 0;
            }
        }

        delay_ms(10);
    }
    return 1;
}

void WIFI_USART_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //�����ж�
    {

        if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //��ֹ���ڱ�ˢ��
        Usart_SendByte(USART1,USART_ReceiveData(USART3));
        esp8266_buf[esp8266_cnt++] = USART3->DR;

//		USART_SendData(USART1,USART2->DR);      //�ý��յ������ݴ�ӡ�ڴ���һ��

        USART_ClearFlag(USART3, USART_FLAG_RXNE);
    }
}






