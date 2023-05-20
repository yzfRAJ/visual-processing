#include "uart.h"
#include "oled.h"
#include "stdio.h"

static u8 Cx=0,Cy=0;

void USART1_Init(void)
{
	  //USART1_TX:PA 9   
	  //USART1_RX:PA10
	GPIO_InitTypeDef GPIO_InitStructure;     //���ڶ˿����ýṹ�����
	USART_InitTypeDef USART_InitStructure;   //���ڲ������ýṹ�����
	NVIC_InitTypeDef NVIC_InitStructure;     //�����ж����ýṹ�����

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   //��PA�˿�ʱ��

    //USART1_TX   PA9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;          		 //PA9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  		 //�趨IO�ڵ�����ٶ�Ϊ50MHz
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   		 //�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure);             	 	 //��ʼ��PA9
    //USART1_RX	  PA10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;             //PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 //��ʼ��PA10 

    //USART1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;  //��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		    //�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			      //IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	                          //����ָ���Ĳ�����ʼ��VIC�Ĵ���

    //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = 115200;                  //���ڲ�����Ϊ115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;   //�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;        //һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;           //����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	                  //�շ�ģʽ
    USART_Init(USART1, &USART_InitStructure);                     //��ʼ������1

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //ʹ���ж�
    USART_Cmd(USART1, ENABLE);                     //ʹ�ܴ���1

  	//�����������1���ֽ��޷���ȷ���ͳ�ȥ������
	  USART_ClearFlag(USART1, USART_FLAG_TC);        //�崮��1���ͱ�־
		
}

//USART1 ȫ���жϷ�����
void USART1_IRQHandler(void)			 
{
		u8 com_data; 
		u8 i;
		static u8 RxCounter1=0;
		static u16 RxBuffer1[10]={0};
		static u8 RxState = 0;	
		static u8 RxFlag1 = 0;


		if( USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)  	   //�����ж�  
		{
				USART_ClearITPendingBit(USART1,USART_IT_RXNE);   //����жϱ�־
				com_data = USART_ReceiveData(USART1);
			
				if(RxState==0&&com_data==0x2C)  //0x2c֡ͷ
				{
					RxState=1;
					RxBuffer1[RxCounter1++]=com_data;OLED_Refresh();
				}
		
				else if(RxState==1&&com_data==0x12)  //0x12֡ͷ
				{
					RxState=2;
					RxBuffer1[RxCounter1++]=com_data;
				}
		
				else if(RxState==2)
				{
					RxBuffer1[RxCounter1++]=com_data;

					if(RxCounter1>=7||com_data == 0x0A)       //RxBuffer1��������,�������ݽ���
					{
						RxState=3;
						RxFlag1=1;
						Cx=RxBuffer1[RxCounter1-3]-48;
						
						Cy=RxBuffer1[RxCounter1-2]-48;
					}
				}
				else if(RxState==3)		//����Ƿ���ܵ�������־
				{
						if(RxBuffer1[RxCounter1-1] == 0x0A)
						{
									USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);//�ر�DTSABLE�ж�
									if(RxFlag1)
									{
									OLED_Refresh();
									OLED_ShowNum(110,0,Cx,1,16,1);
									OLED_ShowNum(40,16,Cy,1,16,1);
									}
									RxFlag1 = 0;
									RxCounter1 = 0;
									RxState = 0;
									USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
						}
						else   //���մ���
						{
									RxState = 0;
									RxCounter1=0;
									for(i=0;i<10;i++)
									{
											RxBuffer1[i]=0x00;      //�����������������
									}
						}
				} 
	
				else   //�����쳣
				{
						RxState = 0;
						RxCounter1=0;
						for(i=0;i<10;i++)
						{
								RxBuffer1[i]=0x00;      //�����������������
						}
				}

		}
		
}
