#ifndef _IIC_H
#define _IIC_H

#include "sys.h"

#define I2C_WR	0		/* 写控制bit */
#define I2C_RD	1		/* 读控制bit */

//IO方向设置
//0X0FFFFFFF(管脚)
#define SDA_IN() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)8<<28;}
#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)3<<28;}


//IO操作函数
#define IIC_SCL    PBout(6) //SCL
#define IIC_SDA    PBout(7) //SDA	 
#define READ_SDA   PBin(7)  //输入SDA 

//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

void IIC_Write_One_Byte(u8 daddr, u8 addr, u8 data);
void IIC_Read_One_Byte(u8 daddr, u8 addr, u8 *data);

void IIC_WriteBytes(u8 WriteAddr, u8 *data, u8 dataLength);
void IIC_ReadBytes(u8 deviceAddr, u8 writeAddr, u8 *data, u8 dataLength);
#endif








