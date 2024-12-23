#include "bsp_usart.h"
#include "bsp_rtc.h"
#include "esp8266.h"
#include "lcd.h"
#include "delay.h"
#include "gui.h"
#include "test.h"


/* 秒中断标志，进入秒中断时置1，当时间被刷新之后清0 */
__IO uint32_t TimeDisplay = 0;

/*星期，生肖用文字ASCII码*/
char const *WEEK_STR[] = {"日", "一", "二", "三", "四", "五", "六"};
char const *zodiac_sign[] = {"猪", "鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗"};

/*英文，星期，生肖用文字ASCII码*/
char const *en_WEEK_STR[] = { "Sunday","Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char const *en_zodiac_sign[] = {"Pig", "Rat", "Ox", "Tiger", "Rabbit", "Dragon", "Snake", "Horse", "Goat", "Monkey", "Rooster", "Dog"};

extern uint8_t dat[5];
//时间数据
extern int DAYS,MOONS,YEARS,TIMES;
extern int hour_return;//小时
extern int min_return; //分钟
extern int sec_return; //秒数
struct rtc_time rtc_time_struct = {0};
// 菜单选项枚举

struct rtc_time systmtime=
{
0,0,0,1,1,2000,0
};

extern __IO uint32_t TimeDisplay ;



/*
 * 函数名：NVIC_Configuration
 * 描述  ：配置RTC秒中断的主中断优先级为1，次优先级为0
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */
void RTC_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/*
 * 函数名：RTC_CheckAndConfig
 * 描述  ：检查并配置RTC
 * 输入  ：用于读取RTC时间的结构体指针
 * 输出  ：无
 * 调用  ：外部调用
 */
void RTC_CheckAndConfig(struct rtc_time *tm)
{
   	/*在启动时检查备份寄存器BKP_DR1，如果内容不是0xA5A5,
	  则需重新配置时间并询问用户调整时间*/
	if (BKP_ReadBackupRegister(RTC_BKP_DRX) != RTC_BKP_DATA)
	{
		printf("\r\n\r\n RTC not yet configured....");
		printf("\r\n\r\n RTC configured....");

		/* 使用tm的时间配置RTC寄存器 */
		Time_Adjust(tm);
		
		/*向BKP_DR1寄存器写入标志，说明RTC已在运行*/
		BKP_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);
	}
	else
	{
		
		/* 使能 PWR 和 Backup 时钟 */
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
		/* 允许访问 Backup 区域 */
	  PWR_BackupAccessCmd(ENABLE);
		
	  /*LSE启动无需设置新时钟*/
		
#ifdef RTC_CLOCK_SOURCE_LSI		
			/* 使能 LSI */
			RCC_LSICmd(ENABLE);

			/* 等待 LSI 准备好 */
			while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
			{}
#endif

		/*检查是否掉电重启*/
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
		{
		    printf("\r\n\r\n Power On Reset occurred....");
		}
		/*检查是否Reset复位*/
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
		{
			printf("\r\n\r\n External Reset occurred....");
		}
	
		printf("\r\n No need to configure RTC....");
		
		/*等待寄存器同步*/
		RTC_WaitForSynchro();
		
		/*允许RTC秒中断*/
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		
		/*等待上次RTC寄存器写操作完成*/
		RTC_WaitForLastTask();
	}
	   /*定义了时钟输出宏，则配置校正时钟输出到PC13*/
	#ifdef RTCClockOutput_Enable
	/* 使能 PWR 和 Backup 时钟 */
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* 允许访问 Backup 区域 */
	  PWR_BackupAccessCmd(ENABLE);
	
	  /* 禁止 Tamper 引脚 */
	  /* 要输出 RTCCLK/64 到 Tamper 引脚,  tamper 功能必须禁止 */	
	  BKP_TamperPinCmd(DISABLE); 
	
	  /* 使能 RTC 时钟输出到 Tamper 引脚 */
	  BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
	#endif
	
	  /* 清除复位标志 flags */
	  RCC_ClearFlag();

}



/*
 * 函数名：RTC_Configuration
 * 描述  ：配置RTC
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */
void RTC_Configuration(void)
{
	/* 使能 PWR 和 Backup 时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* 允许访问 Backup 区域 */
	PWR_BackupAccessCmd(ENABLE);
	
	/* 复位 Backup 区域 */
	BKP_DeInit();
	
//使用外部时钟还是内部时钟（在bsp_rtc.h文件定义）	
//使用外部时钟时，在有些情况下晶振不起振
//批量产品的时候，很容易出现外部晶振不起振的情况，不太可靠	
#ifdef 	RTC_CLOCK_SOURCE_LSE
	/* 使能 LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	
	/* 等待 LSE 准备好 */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{}
	
	/* 选择 LSE 作为 RTC 时钟源 */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	/* 使能 RTC 时钟 */
	RCC_RTCCLKCmd(ENABLE);
	
	/* 等待 RTC 寄存器 同步
	 * 因为RTC时钟是低速的，内环时钟是高速的，所以要同步
	 */
	RTC_WaitForSynchro();
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
	
	/* 使能 RTC 秒中断 */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
	
	/* 设置 RTC 分频: 使 RTC 周期为1s  */
	/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) = 1HZ */
	RTC_SetPrescaler(32767); 
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
	
#else

	/* 使能 LSI */
	RCC_LSICmd(ENABLE);

	/* 等待 LSI 准备好 */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{}
	
	/* 选择 LSI 作为 RTC 时钟源 */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	
	/* 使能 RTC 时钟 */
	RCC_RTCCLKCmd(ENABLE);
	
	/* 等待 RTC 寄存器 同步
	 * 因为RTC时钟是低速的，内环时钟是高速的，所以要同步
	 */
	RTC_WaitForSynchro();
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
	
	/* 使能 RTC 秒中断 */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
	
	/* 设置 RTC 分频: 使 RTC 周期为1s ,LSI约为40KHz */
	/* RTC period = RTCCLK/RTC_PR = (40 KHz)/(40000-1+1) = 1HZ */	
	RTC_SetPrescaler(40000-1); 
	
	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();
#endif
	
}



/*
 * 函数名：Time_Regulate_Get
 * 描述  ：保存用户使用串口设置的时间，
 *         以便后面转化成时间戳存储到RTC 计数寄存器中。
 * 输入  ：用于读取RTC时间的结构体指针
 * 注意  ：在串口调试助手输入时，输入完数字要加回车
 */
void Time_Regulate_Get(struct rtc_time *tm)
{
	  uint32_t temp_num = 0;
		uint8_t day_max=0 ;

	  printf("\r\n=========================设置时间==================");
		
	  do 
	  {
			printf("\r\n  请输入年份(Please Set Years),范围[1970~2038]，输入字符后请加回车:");
			scanf("%d",&temp_num);
			if(temp_num <1970 || temp_num >2038)
			{
				printf("\r\n 您输入的数字是:%d，不符合要求",temp_num);
					  
			}
			else
			{	  
				printf("\n\r  年份被设置为: %d\n\r", temp_num);

				tm->tm_year = temp_num;
				break;
			}
	  }while(1);


	 do 
	  {
			printf("\r\n  请输入月份(Please Set Months):范围[1~12]，输入字符后请加回车:");
			scanf("%d",&temp_num);
			if(temp_num <1 || temp_num >12)
			{
				printf("\r\n 您输入的数字是:%d，不符合要求",temp_num);
					  
			}
			else
			{	  
				printf("\n\r  月份被设置为: %d\n\r", temp_num);

				tm->tm_mon = temp_num;
				break;
			}
	  }while(1);
		
		/*根据月份计算最大日期*/
		switch(tm->tm_mon)
			{
				case 1:
				case 3:
				case 5:
				case 7:
				case 8:
				case 10:
				case 12:					
						day_max = 31;
					break;
				
				case 4:
				case 6:
				case 9:
				case 11:
						day_max = 30;
					break;
				
				case 2:					
				     /*计算闰年*/
						if((tm->tm_year%4==0) &&
							 ((tm->tm_year%100!=0) || (tm->tm_year%400==0)) &&
							 (tm->tm_mon>2)) 
								{
									day_max = 29;
								} else 
								{
									day_max = 28;
								}
					break;			
			}

		do 
	  {				
			printf("\r\n  请输入日期(Please Set Months),范围[1~%d]，输入字符后请加回车:",day_max);
			scanf("%d",&temp_num);
			
			if(temp_num <1 || temp_num >day_max)
			{
				printf("\r\n 您输入的数字是:%d，不符合要求",temp_num);
			}
			else
			{
				printf("\n\r  日期被设置为: %d\n\r", temp_num);

				tm->tm_mday = temp_num;
				break;
			}
	  }while(1);
		
		do 
	  {				
			printf("\r\n  请输入时钟(Please Set Hours),范围[0~23]，输入字符后请加回车:");
			scanf("%d",&temp_num);
			
			if( temp_num >23)
			{
				printf("\r\n 您输入的数字是:%d，不符合要求",temp_num);
			}
			else
			{
				printf("\n\r  时钟被设置为: %d\n\r", temp_num);

				tm->tm_hour = temp_num;
				break;
			}
	  }while(1);

		do 
	  {				
			printf("\r\n  请输入分钟(Please Set Minutes),范围[0~59]，输入字符后请加回车:");
			scanf("%d",&temp_num);
			
			if( temp_num >59)
			{
				printf("\r\n 您输入的数字是:%d，不符合要求",temp_num);
			}
			else
			{
				printf("\n\r  分钟被设置为: %d\n\r", temp_num);

				tm->tm_min = temp_num;
				break;
			}
	  }while(1);

		do 
	  {				
			printf("\r\n  请输入秒钟(Please Set Seconds),范围[0~59]，输入字符后请加回车:");
			scanf("%d",&temp_num);
			
			if( temp_num >59)
			{
				printf("\r\n 您输入的数字是:%d，不符合要求",temp_num);
			}
			else
			{
				printf("\n\r  秒钟被设置为: %d\n\r", temp_num);

				tm->tm_sec = temp_num;
				break;
			}
	  }while(1);

}

/*
 * 函数名：Time_Show
 * 描述  ：显示当前时间值
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */ 
void Time_Show(struct rtc_time *tm)
{	 
		/* 每过1s */
		if (TimeDisplay == 1)
		{
			/* Display current time */
			//Time_Display( RTC_GetCounter(),tm);
			const char* time_str = Time_Display(RTC_GetCounter(),tm);
			//Gui_StrCenter(0,67,BRED,BLUE,(u8 *)time_str,16,1);
			//Show_Str(100,87,BLUE,YELLOW,(u8 *)time_str,16,1);
			TimeDisplay = 0;
		}
}


/*
 * 函数名：Time_Adjust
 * 描述  ：时间调节
 * 输入  ：用于读取RTC时间的结构体指针（北京时间）
 * 输出  ：无
 * 调用  ：外部调用
 */
void Time_Adjust(struct rtc_time *tm)
{
	
			/* RTC 配置 */
		RTC_Configuration();

	  /* 等待确保上一次操作完成 */
	  RTC_WaitForLastTask();
		  
	  /* 计算星期 */
	  GregorianDay(tm);

	  /* 由日期计算时间戳并写入到RTC计数寄存器 */
	  RTC_SetCounter(mktimev(tm)-TIME_ZOOM);

	  /* 等待确保上一次操作完成 */
	  RTC_WaitForLastTask();
}

/*
 * 函数名：Time_Display
 * 描述  ：显示当前时间值
 * 输入  ：-TimeVar RTC计数值，单位为 s
 * 输出  ：无
 * 调用  ：内部调用
 */	

const char* Time_Display(uint32_t TimeVar, struct rtc_time *tm)
{
    static uint32_t FirstDisplay = 1;
		static char display_buffer[256];
    uint32_t BJ_TimeVar;
    uint8_t str[200]; // 字符串暂存

    /*  把标准时间转换为北京时间*/
    BJ_TimeVar = TimeVar + TIME_ZOOM;

    to_tm(BJ_TimeVar, tm); /*把定时器的值转换为北京时间*/

    if ((!tm->tm_hour &&!tm->tm_min &&!tm->tm_sec) || (FirstDisplay))
    {
        GetChinaCalendar((u16)tm->tm_year, (u8)tm->tm_mon, (u8)tm->tm_mday, str);

        // 发送"今天新历："字符串到串口三
        Usart_SendString(USART3, "\r\n 今天新历：");
        // 逐个发送新历日期的数字部分到串口三，格式化为两位宽度
        for (int i = 0; i < 4; i++)
        {
            char num_str[3];
            sprintf(num_str, "%0.2d", str[i]);
            Usart_SendString(USART1, num_str);
            if (i < 3)
            {
                Usart_SendByte(USART1, ',');
            }
        }
        Usart_SendByte(USART1, '\r');
        Usart_SendByte(USART1, '\n');

        GetChinaCalendarStr((u16)tm->tm_year, (u8)tm->tm_mon, (u8)tm->tm_mday, str);
        // 发送"今天农历："字符串及农历日期字符串到串口三
        Usart_SendString(USART1, "\r\n 今天农历：");
        Usart_SendString(USART1, str);
        Usart_SendByte(USART1, '\r');
        Usart_SendByte(USART1, '\n');

        if (GetJieQiStr((u16)tm->tm_year, (u8)tm->tm_mon, (u8)tm->tm_mday, str))
        {
            // 发送包含节气信息的农历日期字符串到串口三
            Usart_SendString(USART1, "\r\n 今天农历：");
            Usart_SendString(USART1, str);
            Usart_SendByte(USART1, '\r');
            Usart_SendByte(USART1, '\n');
        }

        FirstDisplay = 0;
    }

    // 发送时间戳及公历时间信息到串口三
    char time_data[50],time_time[50];
    sprintf(display_buffer, " UNIX时间戳 = %d 当前时间为: %d年(%s年) %d月 %d日 (星期%s)  %0.2d:%0.2d:%0.2d\r",
            TimeVar,
            tm->tm_year, zodiac_sign[(tm->tm_year - 3) % 12], tm->tm_mon, tm->tm_mday,
            WEEK_STR[tm->tm_wday], tm->tm_hour,
            tm->tm_min, tm->tm_sec);
		
		sprintf(time_data, " %d-%d-%d\r",tm->tm_year, tm->tm_mon, tm->tm_mday);
		sprintf(time_time, " %0.2d:%0.2d:%0.2d\r",tm->tm_hour, tm->tm_min, tm->tm_sec);
    Usart_SendString(USART1, display_buffer);
		//LCD_Clear(WHITE);
		Gui_StrCenter(0,90,BRED,WHITE,time_data,16,1);
		Gui_StrCenter(0,120,BRED,WHITE,time_time,16,1);

		return display_buffer;

}
void RTC_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_SEC)!= RESET)
    {
        // 清除秒中断标志，避免重复进入中断
        RTC_ClearITPendingBit(RTC_IT_SEC);

        // 设置时间更新标志为1，以便主程序中的Time_Show函数能检测到并更新显示时间
        TimeDisplay = 1;

        // 可以在这里添加其他需要在秒中断中处理的逻辑，比如定时执行某些任务等
    }
}

void gettime(void)
{
	cJSON_Time_Parse();
	RTC_NVIC_Config();
	RTC_CheckAndConfig(&systmtime);	
	
	printf("获取到的时间信息：%04d-%02d-%02d %02d:%02d:%02d\n", YEARS, MOONS, DAYS, hour_return, min_return, sec_return);

	struct rtc_time set_time;
	set_time.tm_year = YEARS;
	set_time.tm_mon = MOONS;
	set_time.tm_mday = DAYS;
	set_time.tm_hour = hour_return;
	set_time.tm_min = min_return;
	set_time.tm_sec = sec_return;
	
	Time_Adjust(&set_time);
	BKP_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);
	RTC_WaitForLastTask();
}

/************************END OF FILE***************************************/
