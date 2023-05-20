#ifndef __IIC_H
#define __IIC_H
#include "sys.h"

/****************
ע�⣺ģ��IICʱ������IO�ڣ���JTAG���⣬��������SDA��SCL��
****************/
#define OLED_SCL_Clr() GPIO_ResetBits(GPIOA,GPIO_Pin_1)//SCL
#define OLED_SCL_Set() GPIO_SetBits(GPIOA,GPIO_Pin_1)

#define OLED_SDA_Clr() GPIO_ResetBits(GPIOA,GPIO_Pin_2)//AIN
#define OLED_SDA_Set() GPIO_SetBits(GPIOA,GPIO_Pin_2)

void IIC_delay(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_WaitAck(void);
void Send_Byte(u8 dat);

#endif
