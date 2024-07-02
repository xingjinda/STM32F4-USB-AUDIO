#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

#define USBD_AUDIO_FREQ				48000 

#define DEFAULT_VOLUME					100 

#define AUDIO_TOTAL_IF_NUM				0x02
#define USBD_CFG_MAX_NUM				1
#define USBD_ITF_MAX_NUM				2
#define USB_MAX_STR_DESC_SIZ			200

#define USBD_SELF_POWERED


#define	AUDIO_OUT_EP					0x01
#define	HID_IN_EP						0x81
#define	HID_OUT_PACKET					0x05
#endif
