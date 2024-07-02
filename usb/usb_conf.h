#ifndef __USB_CONF__H__
#define __USB_CONF__H__

#include "stm32f4xx.h"


#ifdef USE_USB_OTG_FS 
 #define USB_OTG_FS_CORE
#endif


/****************** USB OTG FS CONFIGURATION **********************************/
#ifdef USB_OTG_FS_CORE
 #define RX_FIFO_FS_SIZE		0x80
 #define TX0_FIFO_FS_SIZE		0x40
 #define TX1_FIFO_FS_SIZE		0x80
 #define TX2_FIFO_FS_SIZE		0
 #define TX3_FIFO_FS_SIZE		0

// #define USB_OTG_FS_LOW_PWR_MGMT_SUPPORT
// #define USB_OTG_FS_SOF_OUTPUT_ENABLED
#endif

#define USE_DEVICE_MODE	

#ifndef USB_OTG_FS_CORE
 #ifndef USB_OTG_HS_CORE
		#error  "USB_OTG_HS_CORE or USB_OTG_FS_CORE should be defined"
 #endif
#endif

#ifndef USE_DEVICE_MODE
 #ifndef USE_HOST_MODE
		#error  "USE_DEVICE_MODE or USE_HOST_MODE should be defined"
 #endif
#endif


#define __ALIGN_BEGIN
#define __ALIGN_END   

/* __packed keyword used to decrease the data type alignment to 1-byte */
#if defined (__CC_ARM)         /* ARM Compiler */
	#define __packed    __packed
#elif defined (__ICCARM__)     /* IAR Compiler */
	#define __packed    __packed
#elif defined   ( __GNUC__ )   /* GNU Compiler */                        
	#define __packed    __attribute__ ((__packed__))
#elif defined   (__TASKING__)  /* TASKING Compiler */
	#define __packed    __unaligned
#endif /* __CC_ARM */

#endif 

