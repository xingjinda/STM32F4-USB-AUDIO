#include "usb_bsp.h"
#include "usbd_conf.h"
#include "delay.h"

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
	RCC->AHB1ENR |= 1 << 0;     // 使能PORTA时钟
	RCC->AHB2ENR |= 1 << 7;     // 使能USB OTG时钟

	RCC->AHB2RSTR|=1<<7;
	RCC->AHB2RSTR&=~(1<<7);

	GPIOA->AFR[1] &=0xFFF00FFF;//复用PA11 PA12
	GPIOA->AFR[1] |=0x000AA000;

	GPIOA->MODER&=0xFC3FFFFF;//PA11 PA12
	GPIOA->MODER|=0x02800000;
	GPIOA->OTYPER&=0x0000E7FF;
	GPIOA->OSPEEDR&=0xFC3FFFFF;
	GPIOA->OSPEEDR|=0x02800000;
	GPIOA->PUPDR&=0xFC3FFFFF;
}

void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
	nvic_init(0,2,OTG_FS_IRQn,2);
}


void USB_OTG_BSP_uDelay (const uint32_t usec)
{ 
	delay_us(usec);
}

void USB_OTG_BSP_mDelay (const uint32_t msec)
{  
	delay_ms(msec);
}
