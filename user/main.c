#include "sys.h"
#include "delay.h"  
#include "usart.h"      
#include "malloc.h" 
#include "usbd_audio_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "IIC.h"
#include "es9018.h"
#include "gpio.h"
#include "24cxx.h"


unsigned char needsavedata;
unsigned char volvalue;

USB_OTG_CORE_HANDLE USB_OTG_dev;

void readvolkey()
{
	unsigned char temp;

	if(VOL_UP)
	{
		temp = volvalue;
		if ( volvalue <= 3 )
		volvalue = 0;
		else
		volvalue -= 3;
		if ( temp != volvalue )
		{
			ES9018_SetVol(volvalue);
			needsavedata = 1;
		}
	}
	if (VOL_DOWN)
	{
		temp = volvalue;
		if ( volvalue >= 0xfd )
		volvalue =0xff;
		else
		volvalue += 3;
		if ( temp != volvalue )
		{
			ES9018_SetVol(volvalue);
			needsavedata = 1;
		}
	}

}

void TIM3_Init()
{
	RCC->APB1ENR|=1<<1;	//开启TIM3时钟
 	TIM3->ARR=1000-1;	//计数器装载值
	TIM3->PSC=8400-1;	//分频器
	TIM3->DIER|=1<<0;	//开启中断
	TIM3->CR1|=1<<0;	//开启定时器
  	nvic_init(1,0,TIM3_IRQn,2);
}

void TIM3_IRQHandler()
{
	TIM3->SR=0xfffe;
	readvolkey();
	if (needsavedata)
	{
		if (!--needsavedata)
		AT24CXX_WriteOneByte(2,volvalue);
	}
}

int main(void)
{
	#if defined(STM32F412Rx)
		stm32_clock_init(192,12,2,4);
		delay_init(96);
		#ifdef DEBUG
			usart_init(96,115200);
		#endif
	#elif defined(STM32F401xC)
		stm32_clock_init(336,12,4,7);
		delay_init(84);
		#ifdef DEBUG
			usart_init(84,115200);
		#endif
	#endif
	gpio_init();
	IIC_Init();
	volvalue=AT24CXX_ReadOneByte(2);
	TIM3_Init();
	mem_init();
	if(ES9018_Init()!=4)
	{
		#ifdef DEBUG
			print("ES9018k2m Init False\n");
		#endif
	}
	else
	{
		ES9018_SetVol(volvalue);
		USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&AUDIO_cb,&USR_cb);
	}
	while(1)
	{
	}
}






