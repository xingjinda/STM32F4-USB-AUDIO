#include "sys.h"
#include "delay.h"  
#include "usart.h"      
#include "malloc.h" 
#include "usbd_audio_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "IIC.h"
#include "es9018.h"	
 
USB_OTG_CORE_HANDLE USB_OTG_dev;
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
	IIC_Init();
	ES9018_Init();
	mem_init();
	USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&AUDIO_cb,&USR_cb);
	while(1)
	{
	}
}






