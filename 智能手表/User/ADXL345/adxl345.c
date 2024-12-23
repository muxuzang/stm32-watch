#include "adxl345.h"
#include "delay.h"
#include "math.h"
#include <stdlib.h>	
#include "stm32f10x.h"

// ADXL345�������ģʽ����
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

// ģ��IIC���ߺ���
void ADXL345_IIC_Init() {
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(ADXL345_SCL_GPIO_CLK, ENABLE);    // ʹ��SCL�˿�ʱ��
    GPIO_InitStructure.GPIO_Pin = ADXL345_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADXL345_SCL_PORT, &GPIO_InitStructure);
    GPIO_SetBits(ADXL345_SCL_PORT, ADXL345_SCL_PIN);

    RCC_APB2PeriphClockCmd(ADXL345_SDA_GPIO_CLK, ENABLE);    // ʹ��SDA�˿�ʱ��
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

// ��������һ��Ӧ���ź�
void ADXL345_IIC_Ack() {
    ADXL345_IIC_SCL = 0;
    ADXL345_IIC_SDA_OUT();
    ADXL345_IIC_SDA = 0;

    delay_us(2);
    ADXL345_IIC_SCL = 1;
    delay_us(5);
    ADXL345_IIC_SCL = 0;
}

// ����������Ӧ���ź�
void ADXL345_IIC_NAck() {
    ADXL345_IIC_SCL = 0;
    ADXL345_IIC_SDA_OUT();
    ADXL345_IIC_SDA = 1;
    delay_us(2);
    ADXL345_IIC_SCL = 1;
    delay_us(2);
    ADXL345_IIC_SCL = 0;
}

// �ȴ��ӻ�Ӧ���ź�
// ����ֵ��1 ����Ӧ��ʧ��
//          0 ����Ӧ��ɹ�
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

// ��ȡһ���ֽ�
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

// ��������ʼ��
void ADXL345_Full_Init(void) {
    ADXL345_IIC_Init();
    // ������δ���ĳ�ʼ���������ú��ʵ�ֵ����Ӧ�Ĵ���
    adxl345_write_reg(0X31, 0X0B);    // �͵�ƽ�ж����, 13λȫ�ֱ���, ��������Ҷ���, 16g���� 
    adxl345_write_reg(0x2C, 0x08);    // �����趨Ϊ50Hz �ο�pdf13ҳ���ɸ���ʵ������������ʣ�
    adxl345_write_reg(0x2D, 0x08);    // ѡ���Դģʽ   �ο�pdf24ҳ
    adxl345_write_reg(0x2E, 0x80);    // ʹ�� DATA_READY �ж�
    adxl345_write_reg(0X1E, 0x00);    // X ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
    adxl345_write_reg(0X1F, 0x00);    // Y ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
    adxl345_write_reg(0X20, 0x05);    // Z ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
}

// д�Ĵ�������
void adxl345_write_reg(u8 addr, u8 val) {
    ADXL345_IIC_Start();
    ADXL345_IIC_Send_Byte(slaveaddress);    // ����д����ָ��
    ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Send_Byte(addr);    // ���ͼĴ�����ַ
    ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Send_Byte(val);    // ����ֵ
    ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Stop();    // ����һ��ֹͣ����
}

// ���Ĵ�������
u8 adxl345_read_reg(u8 addr) {
    u8 temp = 0;
    ADXL345_IIC_Start();
    ADXL345_IIC_Send_Byte(slaveaddress);    // ����д����ָ��
    temp = ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Send_Byte(addr);    // ���ͼĴ�����ַ
    temp = ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Start();
    ADXL345_IIC_Send_Byte(regaddress);    // ���Ͷ�����ָ��
    temp = ADXL345_IIC_Wait_Ack();
    temp = ADXL345_IIC_Read_Byte(0);    // ��ȡһ���ֽ�,�������ٶ�,����NAK
    ADXL345_IIC_Stop();    // ����һ��ֹͣ����
    return temp;
}

// ��ȡ���ݺ���
void adxl345_read_data(short *x, short *y, short *z) {
    u8 buf[6];
    u8 i;
    ADXL345_IIC_Start();
    ADXL345_IIC_Send_Byte(slaveaddress);    // ����д����ָ��
    ADXL345_IIC_Wait_Ack();
    ADXL345_IIC_Send_Byte(0x32);    // ���ͼĴ�����ַ(���ݻ������ʼ��ַΪ0X32)
    ADXL345_IIC_Wait_Ack();

    ADXL345_IIC_Start();
    ADXL345_IIC_Send_Byte(regaddress);    // ���Ͷ�����ָ��
    ADXL345_IIC_Wait_Ack();
    for (i = 0; i < 6; i++) {
        if (i == 5)
            buf[i] = ADXL345_IIC_Read_Byte(0);    // ��ȡһ���ֽ�,�������ٶ�,����NACK
        else
            buf[i] = ADXL345_IIC_Read_Byte(1);    // ��ȡһ���ֽ�,������,����ACK
    }
    ADXL345_IIC_Stop();    // ����һ��ֹͣ����
    *x = (short) (((u16) buf[1] << 8) + buf[0]);    // �ϳ�����
    *y = (short) (((u16) buf[3] << 8) + buf[2]);
    *z = (short) (((u16) buf[5] << 8) + buf[4]);
}

// ������ȡ����ȡƽ��ֵ����
// times ȡƽ��ֵ�Ĵ���
void adxl345_read_average(float *x, float *y, float *z, u8 times) {
    u8 i;
    short tx, ty, tz;
    *x = 0;
    *y = 0;
    *z = 0;
    if (times) {    // ��ȡ������Ϊ0
        for (i = 0; i < times; i++) {    // ������ȡtimes��
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

// ��ȡxyz���ݴ����ֵ�˲������������м��㣬�˲�����������sample
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
		//������Թ��ˡ�
//		printf("xֵ%d\n",sample->x);
//		printf("yֵ%d\n",sample->y);
//		printf("zֵ%d\n",sample->z);

}

// 50���������ҳ������Сֵ��Ϊ��̬��ֵ
// �ҳ����ֵ����Сֵ
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
//		printf("x��Сֵ%d\n",peak->oldmin.x);
//		printf("yminֵ%d\n",peak->oldmin.y);
//		printf("zminֵ%d\n",peak->oldmin.z);
}

// һ��������λ�Ĵ��������ڹ��˸�Ƶ����
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

// �ж����Ծ��
int is_most_active(peak_value_t *peak) {
    int res;
    short x_change;
    short y_change;
    short z_change;
    
    x_change = abs((peak->newmax.x - peak->newmin.x));
    y_change = abs((peak->newmax.y - peak->newmin.y));
    z_change = abs((peak->newmax.z - peak->newmin.z));
    
//	  printf("x�仯zuidaֵ%d\n",peak->newmax.x);
//		printf("y�仯ֵ%d\n",y_change);
//		printf("z�仯ֵ%d\n",z_change);
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

// �ж��Ƿ��߲�
// ���ĵļ��㲽���ĺ���
int8_t detect_step(peak_value_t *peak, slid_reg_t *slid, axis_info_t *cur_sample) {
    static int8_t step_cnt = 0;
    static uint32_t counter = 0;  // ����ģ��ʱ�����ŵļ�����
    int res;
    peak_update(peak,cur_sample);
    res = is_most_active(peak);
    slid_update(slid, cur_sample);
	  //printf("res %d\n",res);

    switch (res) {
        case 0: {
            // �����Ծ����������ɸ���ʵ����������
            break;
        }
        case 1: {
            short threshold_x = (peak->oldmax.x + peak->oldmin.x) / 2;
            if (slid->old_sample.x > threshold_x && slid->new_sample.x < threshold_x) {
                counter++;  // ÿ�ν��������������������������1��ģ��ʱ���ƽ�
                if (counter >= MIN_STEP_COUNTER_VALUE && counter <= MAX_STEP_COUNTER_VALUE) {
                    step_cnt++;
                    counter = 0;  // �ﵽ����ʱ��������Ϊ����һ�������ü�����
                }
            } else {
                // ��������㲽���ж��������Ҽ��������������ʱ�����ڷ�Χ�����ü�����
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

