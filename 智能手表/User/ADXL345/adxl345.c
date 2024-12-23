#include "adxl345.h"
#include "delay.h"
#include "math.h"
#include <stdlib.h>	
#include "stm32f10x.h"

// ADXL345引脚输出模式控制
void ADXL345_IIC_SDA_OUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = ADXL345_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(ADXL345_SDA_PORT, &GPIO_InitStructure);
}

void ADXL345_IIC_SDA_IN(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = ADXL345_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(ADXL345_SDA_PORT, &GPIO_InitStructure);
}

// 模拟IIC总线函数
void ADXL345_IIC_Init() {
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(ADXL345_SCL_GPIO_CLK, ENABLE);    // 使能SCL端口时钟
    GPIO_InitStructure.GPIO_Pin = ADXL345_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADXL345_SCL_PORT, &GPIO_InitStructure);
    GPIO_SetBits(ADXL345_SCL_PORT, ADXL345_SCL_PIN);

    RCC_APB2PeriphClockCmd(ADXL345_SDA_GPIO_CLK, ENABLE);    // 使能SDA端口时钟
    GPIO_InitStructure.GPIO_Pin = ADXL345_SDA_PIN;
    GPIO_SetBits(ADXL345_SDA_PORT, ADXL345_SDA_PIN);
}

void ADXL345_IIC_Start() {
    ADXL345_IIC_SDA_OUT();
    ADXL345_IIC_SDA = 1;
    ADXL345_IIC_SCL = 1;
    delay_us(5);
    ADXL345_IIC_SDA = 0;
    delay_us(5);
    ADXL345_IIC_SCL = 0;
}

void ADXL345_IIC_Stop() {
    ADXL345_IIC_SDA_OUT();
    ADXL345_IIC_SCL = 0;
    ADXL345_IIC_SDA = 0;
    delay_us(5);
    ADXL345_IIC_SCL = 1;
    ADXL345_IIC_SDA = 1;
    delay_us(5);
}

// 主机产生一个应答信号
void ADXL345_IIC_Ack() {
    ADXL345_IIC_SCL = 0;
    ADXL345_IIC_SDA_OUT();
    ADXL345_IIC_SDA = 0;

    delay_us(2);
    ADXL345_IIC_SCL = 1;
    delay_us(5);
    ADXL345_IIC_SCL = 0;
}

// 主机不产生应答信号
void ADXL345_IIC_NAck() {
    ADXL345_IIC_SCL = 0;
    ADXL345_IIC_SDA_OUT();
    ADXL345_IIC_SDA = 1;
    delay_us(2);
    ADXL345_IIC_SCL = 1;
    delay_us(2);
    ADXL345_IIC_SCL = 0;
}

// 等待从机应答信号
// 返回值：1 接收应答失败
//          0 接收应答成功
u8 ADXL345_IIC_Wait_Ack() {
    u8 tempTime = 0;
    ADXL345_IIC_SDA_IN();
    ADXL345_IIC_SDA = 1;
    delay_us(1);
    ADXL345_IIC_SCL = 1;
    delay_us(1);

    while (ADXL345_READ_SDA) {
        tempTime++;
        if (tempTime > 250) {
            ADXL345_IIC_Stop();
            return 1;
        }
    }

    ADXL345_IIC_SCL = 0;
    return 0;
}

void ADXL345_IIC_Send_Byte(u8 txd) {
    u8 i = 0;
    ADXL345_IIC_SDA_OUT();
    ADXL345_IIC_SCL = 0;
    for (i = 0; i < 8; i++) {
        ADXL345_IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;
        ADXL345_IIC_SCL = 1;
        delay_us(2);
        ADXL345_IIC_SCL = 0;
        delay_us(2);
    }
}

// 读取一个字节
u8 ADXL345_IIC_Read_Byte(u8 ack) {
    u8 i = 0, receive = 0;
    ADXL345_IIC_SDA_IN();
    for (i = 0; i < 8; i++) {
        ADXL345_IIC_SCL = 0;
        delay_us(2);
        ADXL345_IIC_SCL = 1;
        receive <<= 1;
        if (ADXL345_READ_SDA)
            receive++;
        delay_us(1);
    }

    if (!ack)
        ADXL345_IIC_NAck();
    else
        ADXL345_IIC_Ack();

    return receive;
}

// 传感器初始化
void ADXL345_Full_Init(void) {
    ADXL345_IIC_Init();
    // 结合两段代码的初始化参数设置合适的值到对应寄存器
    adxl345_write_reg(0X31, 0X0B);    // 低电平中断输出, 13位全分辨率, 输出数据右对齐, 16g量程 
    adxl345_write_reg(0x2C, 0x08);    // 速率设定为50Hz 参考pdf13页（可根据实际需求调整速率）
    adxl345_write_reg(0x2D, 0x08);    // 选择电源模式   参考pdf24页
    adxl345_write_reg(0x2E, 0x80);    // 使能 DATA_READY 中断
    adxl345_write_reg(0X1E, 0x00);    // X 偏移量 根据测试传感器的状态写入pdf29页
    adxl345_write_reg(0X1F, 0x00);    // Y 偏移量 根据测试传感器的状态写入pdf29页
    adxl345_write_reg(0X20, 0x05);    // Z 偏移量 根据测试传感器的状态写入pdf29页
}

// 写寄存器函数
void adxl345_write_reg(u8 addr, u8 val) {
    ADXL345_IIC_Start();
    ADXL345_IIC_Send_Byte(slaveaddress);    // 发送写器件指令
    ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Send_Byte(addr);    // 发送寄存器地址
    ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Send_Byte(val);    // 发送值
    ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Stop();    // 产生一个停止条件
}

// 读寄存器函数
u8 adxl345_read_reg(u8 addr) {
    u8 temp = 0;
    ADXL345_IIC_Start();
    ADXL345_IIC_Send_Byte(slaveaddress);    // 发送写器件指令
    temp = ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Send_Byte(addr);    // 发送寄存器地址
    temp = ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Start();
    ADXL345_IIC_Send_Byte(regaddress);    // 发送读器件指令
    temp = ADXL345_IIC_Wait_Ack();
    temp = ADXL345_IIC_Read_Byte(0);    // 读取一个字节,不继续再读,发送NAK
    ADXL345_IIC_Stop();    // 产生一个停止条件
    return temp;
}

// 读取数据函数
void adxl345_read_data(short *x, short *y, short *z) {
    u8 buf[6];
    u8 i;
    ADXL345_IIC_Start();
    ADXL345_IIC_Send_Byte(slaveaddress);    // 发送写器件指令
    ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Send_Byte(0x32);    // 发送寄存器地址(数据缓存的起始地址为0X32)
    ADXL345_IIC_Wait_Ack();

    ADXL345_IIC_Start();
    ADXL345_IIC_Send_Byte(regaddress);    // 发送读器件指令
    ADXL345_IIC_Wait_Ack();
    for (i = 0; i < 6; i++) {
        if (i == 5)
            buf[i] = ADXL345_IIC_Read_Byte(0);    // 读取一个字节,不继续再读,发送NACK
        else
            buf[i] = ADXL345_IIC_Read_Byte(1);    // 读取一个字节,继续读,发送ACK
    }
    ADXL345_IIC_Stop();    // 产生一个停止条件
    *x = (short) (((u16) buf[1] << 8) + buf[0]);    // 合成数据
    *y = (short) (((u16) buf[3] << 8) + buf[2]);
    *z = (short) (((u16) buf[5] << 8) + buf[4]);
}

// 连读读取几次取平均值函数
// times 取平均值的次数
void adxl345_read_average(float *x, float *y, float *z, u8 times) {
    u8 i;
    short tx, ty, tz;
    *x = 0;
    *y = 0;
    *z = 0;
    if (times) {    // 读取次数不为0
        for (i = 0; i < times; i++) {    // 连续读取times次
            adxl345_read_data(&tx, &ty, &tz);
            *x += tx;
            *y += ty;
            *z += tz;
            delay_ms(5);
        }
        *x /= times;
        *y /= times;
        *z /= times;
    }
}

// 读取xyz数据存入均值滤波器，存满进行计算，滤波后样本存入sample
void filter_calculate(filter_avg_t *filter, axis_info_t *sample) {
    uint8_t i;
    short x_sum = 0, y_sum = 0, z_sum = 0;

    for (i = 0; i < 4; i++) {
        adxl345_read_data(&filter->info[i].x, &filter->info[i].y, &filter->info[i].z);
        x_sum += filter->info[i].x;
        y_sum += filter->info[i].y;
        z_sum += filter->info[i].z;
    }
    sample->x = x_sum / 4;
    sample->y = y_sum / 4;
    sample->z = z_sum / 4;
		//这里测试过了。
//		printf("x值%d\n",sample->x);
//		printf("y值%d\n",sample->y);
//		printf("z值%d\n",sample->z);

}

// 50个样本中找出最大最小值作为动态阈值
// 找出最大值和最小值
void peak_update(peak_value_t *peak, axis_info_t *sample) {
    int8_t i;
    filter_avg_t filter;
    short min_value = -32768;
    short max_value = 32767;
    peak->newmax.x = min_value;
    peak->newmax.y = min_value;
    peak->newmax.z = min_value;

    peak->newmin.x = max_value;
    peak->newmin.y = max_value;
    peak->newmin.z = max_value;
    for (i = 0; i < 50; i++) {
        filter_calculate(&filter, sample);

        peak->newmax.x = (peak->newmax.x > sample->x)? peak->newmax.x : sample->x;
        peak->newmax.y = (peak->newmax.y > sample->y)? peak->newmax.y : sample->y;
        peak->newmax.z = (peak->newmax.z > sample->z)? peak->newmax.z : sample->z;

        peak->newmin.x = (peak->newmin.x < sample->x)? peak->newmin.x : sample->x;
        peak->newmin.y = (peak->newmin.y < sample->y)? peak->newmin.y : sample->y;
        peak->newmin.z = (peak->newmin.z < sample->z)? peak->newmin.z : sample->z;
    }
    peak->oldmax.x = peak->newmax.x;
    peak->oldmax.y = peak->newmax.y;
    peak->oldmax.z = peak->newmax.z;

    peak->oldmin.x = peak->newmin.x;
    peak->oldmin.y = peak->newmin.y;
    peak->oldmin.z = peak->newmin.z;
//		printf("x最小值%d\n",peak->oldmin.x);
//		printf("ymin值%d\n",peak->oldmin.y);
//		printf("zmin值%d\n",peak->oldmin.z);
}

// 一个线性移位寄存器，用于过滤高频噪声
void slid_update(slid_reg_t *slid, axis_info_t *cur_sample) {
    filter_avg_t filter;
    filter_calculate(&filter, cur_sample);
    cur_sample->x = cur_sample->x;
    cur_sample->y = cur_sample->y;
    cur_sample->z = cur_sample->z;

    if (abs((cur_sample->x - slid->new_sample.x)) > 30) {
        slid->old_sample.x = slid->new_sample.x;
        slid->new_sample.x = cur_sample->x;
    } else {
        slid->old_sample.x = slid->new_sample.x;
    }
    if (abs((cur_sample->y - slid->new_sample.y)) > 30) {
        slid->old_sample.y = slid->new_sample.y;
        slid->new_sample.y = cur_sample->y;
    } else {
        slid->old_sample.y = slid->new_sample.y;
    }
    if (abs((cur_sample->z - slid->new_sample.z)) > 30) {
        slid->old_sample.z = slid->new_sample.z;
        slid->new_sample.z = cur_sample->z;
    } else {
        slid->old_sample.z = slid->new_sample.z;
    }
}

// 判断最活跃轴
int is_most_active(peak_value_t *peak) {
    int res;
    short x_change;
    short y_change;
    short z_change;
    
    x_change = abs((peak->newmax.x - peak->newmin.x));
    y_change = abs((peak->newmax.y - peak->newmin.y));
    z_change = abs((peak->newmax.z - peak->newmin.z));
    
//	  printf("x变化zuida值%d\n",peak->newmax.x);
//		printf("y变化值%d\n",y_change);
//		printf("z变化值%d\n",z_change);
    if (x_change > y_change && x_change > z_change && x_change >= 40) {
        res = 1;
    } else if (y_change > x_change && y_change > z_change && y_change >= 40) {
        res = 2;
    } else if (z_change > x_change && z_change > y_change && z_change >= 40) {
        res = 3;
    } else {
        res = 0;
    }
    return res;
}

// 判断是否走步
// 核心的计算步数的函数
int8_t detect_step(peak_value_t *peak, slid_reg_t *slid, axis_info_t *cur_sample) {
    static int8_t step_cnt = 0;
    static uint32_t counter = 0;  // 用于模拟时间流逝的计数器
    int res;
    peak_update(peak,cur_sample);
    res = is_most_active(peak);
    slid_update(slid, cur_sample);
	  //printf("res %d\n",res);

    switch (res) {
        case 0: {
            // 无最活跃轴情况处理，可根据实际需求完善
            break;
        }
        case 1: {
            short threshold_x = (peak->oldmax.x + peak->oldmin.x) / 2;
            if (slid->old_sample.x > threshold_x && slid->new_sample.x < threshold_x) {
                counter++;  // 每次进入满足条件的情况，计数器加1，模拟时间推进
                if (counter >= MIN_STEP_COUNTER_VALUE && counter <= MAX_STEP_COUNTER_VALUE) {
                    step_cnt++;
                    counter = 0;  // 达到合适时间间隔，认为产生一步，重置计数器
                }
            } else {
                // 如果不满足步数判断条件，且计数器超过了最大时间周期范围，重置计数器
                if (counter > COUNTER_MAX_VALUE) {
                    counter = 0;
                }
            }
							 	//printf("1 counter %d\n",counter);
						   // printf("1 step_cnt %d\n",step_cnt);

            break;
        }
        case 2: {
            short threshold_y = (peak->oldmax.y + peak->oldmin.y) / 2;
            if (slid->old_sample.y > threshold_y && slid->new_sample.y < threshold_y) {
                counter++;
                if (counter >= MIN_STEP_COUNTER_VALUE &&  counter <= MAX_STEP_COUNTER_VALUE) {
                    step_cnt++;
                    counter = 0;
                }
            } else {
                if (counter > COUNTER_MAX_VALUE) {
                    counter = 0;
                }
            }
//						printf("2 counter %d\n",counter);
//						printf("2 step_cnt %d\n",step_cnt);

            break;
        }
        case 3: {
            short threshold_z = (peak->oldmax.z + peak->oldmin.z) / 2;
            if (slid->old_sample.z > threshold_z && slid->new_sample.z < threshold_z) {
                counter++;
                if (counter >= MIN_STEP_COUNTER_VALUE && counter <= MAX_STEP_COUNTER_VALUE) {
                    step_cnt++;
                    counter = 0;
                }
            } else {
                if (counter > COUNTER_MAX_VALUE) {
                    counter = 0;
                }
            }
						//printf("3 counter %d\n",counter);
						//printf("3 step_cnt %d\n",step_cnt);

            break;
        }
        default:
					 						//printf("default counter %d\n",counter);
            break;
    }
    return step_cnt;
}

