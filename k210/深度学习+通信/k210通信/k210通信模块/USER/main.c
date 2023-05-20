/******************************
SDA:PA2
SCL:PA1
******************************/

#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "bmp.h"
#include "uart.h"
#include "stdio.h"


int main(void)
 {		

		OLED_Init();
		delay_init();	    	                              //��ʱ������ʼ��	  
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);   //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
		USART1_Init();
		OLED_ColorTurn(0);          //0������ʾ��1 ��ɫ��ʾ
		OLED_DisplayTurn(0);        //0������ʾ 1 ��Ļ��ת��ʾ
		while(1) 
		{
			OLED_ShowString(0,16,"flag",16,1);
			OLED_ShowChar (32,16,':',16,1);
			OLED_ShowChinese (0,0,0,16,1);
			OLED_ShowChinese (16,0,1,16,1);
			OLED_ShowChinese (32,0,2,16,1);
			OLED_ShowChinese (48,0,3,16,1);
			OLED_ShowChinese (64,0,4,16,1);
			OLED_ShowChinese (80,0,5,16,1);
			OLED_ShowChar (96,0,':',16,1);
			OLED_Refresh();
 
	
		}
	
 }
