#ifndef __RTC_H
#define	__RTC_H


#include "stm32f10x.h"
#include "./bsp_calendar.h"
#include "./bsp_date.h"

//�Ƿ�ʹ��LCD��ʾ����
#define USE_LCD_DISPLAY


//ʹ��LSE�ⲿʱ�� �� LSI�ڲ�ʱ��
//#define RTC_CLOCK_SOURCE_LSE      
#define RTC_CLOCK_SOURCE_LSI

#define RTC_BKP_DRX          BKP_DR1
// д�뵽���ݼĴ��������ݺ궨��
#define RTC_BKP_DATA         0xA5A5

//����ʱ���ʱ��������
#define TIME_ZOOM						(8*60*60)
extern uint8_t dat[5]; 
extern int DAYS, MOONS, YEARS, TIMES;
extern int hour_return, min_return, sec_return;


extern struct rtc_time rtc_time_struct;
extern struct rtc_time systmtime;
extern __IO uint32_t TimeDisplay;
extern struct rtc_time set_time;
/* ������������������Ļ�,PC13�ͻ����Ƶ��ΪRTC Clock/64��ʱ�� */   
//#define RTCClockOutput_Enable  /* RTC Clock/64 is output on tamper pin(PC.13) */

void RTC_NVIC_Config(void);
void RTC_Configuration(void);
void Time_Regulate_Get(struct rtc_time *tm);
void Time_Adjust(struct rtc_time *tm);
const char* Time_Display(uint32_t TimeVar,struct rtc_time *tm);
void Time_Show(struct rtc_time *tm);
static uint8_t USART_Scanf(uint32_t value);
void RTC_CheckAndConfig(struct rtc_time *tm);
void gettime(void);

#endif /* __XXX_H */
