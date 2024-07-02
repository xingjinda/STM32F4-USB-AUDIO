#include "sys.h"
#include "usart.h"
#include "stdarg.h"
#include "stdio.h"


#if SYS_SUPPORT_OS
#include "os.h"
#endif


void print(char* format, ...) 
{
	char sz[100]={0};
	unsigned int i=0;
	va_list args;
	va_start(args, format);
	vsprintf(sz, format, args);
	for (; i < 100; i++)
	{
		if(sz[i])
		{
			while ((USART1->SR & 0X40) == 0);
			USART1->DR = sz[i];
		}
	}
	va_end(args);
}

#if USART_EN_RX


unsigned char rx_buf[USART_REC_LEN];

//  接收状态
//  bit15，      接收完成标志
//  bit14，      接收到0x0d
//  bit13~0，    接收到的有效字节数目
//
unsigned short rx_sta = 0;


void USART1_IRQHandler(void)
{
	unsigned char rxdata;
#if SYS_SUPPORT_OS  /* 如果SYS_SUPPORT_OS为真，则需要支持OS. */
	OSIntEnter();
#endif

	if (USART1->SR & (1 << 5))
	{
		rxdata = USART1->DR;

		if ((rx_sta & 0x8000) == 0)
		{
			if (rx_sta & 0x4000)
			{
				if (rxdata != 0x0a)	rx_sta = 0;
				else rx_sta |= 0x8000;
			}
			else
			{
				if (rxdata == 0x0d)
					rx_sta |= 0x4000;
				else
				{
					rx_buf[rx_sta & 0X3FFF] = rxdata;
					rx_sta++;

					if (rx_sta > (USART_REC_LEN - 1))rx_sta = 0;
				}
			}
		}
	}

#if SYS_SUPPORT_OS
	OSIntExit();
#endif
}
#endif


void usart_init(unsigned int sclk, unsigned int baudrate)
{
	unsigned int temp;

	RCC->AHB1ENR |= 1 << 0;	//开启PA时钟
	RCC->APB2ENR |= 1 << 4;	//开启串口时钟

	GPIOA->AFR[1] &=0xFFFFF00F;//复用PA9 PA10
	GPIOA->AFR[1] |=0x00000770;

	GPIOA->MODER&=0xFFC3FFFF;//PA9 PA10
	GPIOA->MODER|=0x00280000;
	GPIOA->OTYPER&=0x0000F6FF;
	GPIOA->OSPEEDR&=0xFFC3FFFF;
	GPIOA->OSPEEDR|=0x00280000;
	GPIOA->PUPDR&=0xFFC3FFFF;
	
	temp = (sclk * 1000000 + baudrate / 2) / baudrate;

	USART1->BRR = temp;
	USART1->CR1 = 0;
	USART1->CR1 |= 0 << 12;
	USART1->CR1 |= 0 << 15;
	USART1->CR1 |= 1 << 3;	//开启串口发送
#if USART_EN_RX
	USART1->CR1 |= 1 << 2;	//开启串口接收
	USART1->CR1 |= 1 << 5;	// 接收缓冲区非空中断
	sys_nvic_init(3, 3, USART1_IRQn, 2);
#endif
	USART1->CR1 |= 1 << 13;	//开启串口使能

}
