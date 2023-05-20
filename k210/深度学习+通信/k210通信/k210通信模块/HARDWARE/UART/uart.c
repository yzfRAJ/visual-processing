#include "uart.h"
#include "oled.h"
#include "stdio.h"

static u8 Cx=0,Cy=0;

void USART1_Init(void)
{
	  //USART1_TX:PA 9   
	  //USART1_RX:PA10
	GPIO_InitTypeDef GPIO_InitStructure;     //串口端口配置结构体变量
	USART_InitTypeDef USART_InitStructure;   //串口参数配置结构体变量
	NVIC_InitTypeDef NVIC_InitStructure;     //串口中断配置结构体变量

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   //打开PA端口时钟

    //USART1_TX   PA9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;          		 //PA9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  		 //设定IO口的输出速度为50MHz
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   		 //复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);             	 	 //初始化PA9
    //USART1_RX	  PA10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;             //PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 //初始化PA10 

    //USART1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;  //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		    //子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			      //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	                          //根据指定的参数初始化VIC寄存器

    //USART 初始化设置
	USART_InitStructure.USART_BaudRate = 115200;                  //串口波特率为115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;   //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;        //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;           //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	                  //收发模式
    USART_Init(USART1, &USART_InitStructure);                     //初始化串口1

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //使能中断
    USART_Cmd(USART1, ENABLE);                     //使能串口1

  	//如下语句解决第1个字节无法正确发送出去的问题
	  USART_ClearFlag(USART1, USART_FLAG_TC);        //清串口1发送标志
		
}

//USART1 全局中断服务函数
void USART1_IRQHandler(void)			 
{
		u8 com_data; 
		u8 i;
		static u8 RxCounter1=0;
		static u16 RxBuffer1[10]={0};
		static u8 RxState = 0;	
		static u8 RxFlag1 = 0;


		if( USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)  	   //接收中断  
		{
				USART_ClearITPendingBit(USART1,USART_IT_RXNE);   //清除中断标志
				com_data = USART_ReceiveData(USART1);
			
				if(RxState==0&&com_data==0x2C)  //0x2c帧头
				{
					RxState=1;
					RxBuffer1[RxCounter1++]=com_data;OLED_Refresh();
				}
		
				else if(RxState==1&&com_data==0x12)  //0x12帧头
				{
					RxState=2;
					RxBuffer1[RxCounter1++]=com_data;
				}
		
				else if(RxState==2)
				{
					RxBuffer1[RxCounter1++]=com_data;

					if(RxCounter1>=7||com_data == 0x0A)       //RxBuffer1接受满了,接收数据结束
					{
						RxState=3;
						RxFlag1=1;
						Cx=RxBuffer1[RxCounter1-3]-48;
						
						Cy=RxBuffer1[RxCounter1-2]-48;
					}
				}
				else if(RxState==3)		//检测是否接受到结束标志
				{
						if(RxBuffer1[RxCounter1-1] == 0x0A)
						{
									USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);//关闭DTSABLE中断
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
						else   //接收错误
						{
									RxState = 0;
									RxCounter1=0;
									for(i=0;i<10;i++)
									{
											RxBuffer1[i]=0x00;      //将存放数据数组清零
									}
						}
				} 
	
				else   //接收异常
				{
						RxState = 0;
						RxCounter1=0;
						for(i=0;i<10;i++)
						{
								RxBuffer1[i]=0x00;      //将存放数据数组清零
						}
				}

		}
		
}
