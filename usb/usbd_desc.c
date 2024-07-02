#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_conf.h"
#include "usb_regs.h"


#define USBD_VID                        0x3210
#define USBD_PID                        0x1230



#define USBD_LANGID_STRING              0x0409
#define USBD_MANUFACTURER_STRING        "XJD"
#define USBD_PRODUCT_FS_STRING          "XJD GamePad"

#define USBD_SERIALNUMBER_FS_STRING     "000000000123"


#define USBD_CONFIGURATION_FS_STRING    "HID Config"
#define USBD_INTERFACE_FS_STRING        "HID Interface"


USBD_DEVICE USR_desc =
{
	get_Device,
	get_LangID, 
	get_Manufacturer,
	get_Product,
	get_Serial,
	get_Config,
	get_Interface,
	
};


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined   (__CC_ARM)        /* !< ARM Compiler */
__align(4)
#elif defined ( __ICCARM__ )    /* !< IAR Compiler */
#pragma data_alignment=4
#elif defined (__GNUC__)        /* !< GNU Compiler */
#pragma pack(4)
#endif                          /* __CC_ARM */
#endif

uint8_t USBD_DeviceDesc[USB_SIZ_DEVICE_DESC] = {
	0x12,                         /* bLength */
	USB_DEVICE_DESCRIPTOR_TYPE,   /* bDescriptorType */
	0x00,                         /* bcdUSB */
	0x02,
	0x00,                         /* bDeviceClass */
	0x00,                         /* bDeviceSubClass */
	0x00,                         /* bDeviceProtocol */
	USB_OTG_MAX_EP0_SIZE,         /* bMaxPacketSize */
	LOBYTE(USBD_VID),             /* idVendor */
	HIBYTE(USBD_VID),             /* idVendor */
	LOBYTE(USBD_PID),             /* idVendor */
	HIBYTE(USBD_PID),             /* idVendor */
	0x00,                         /* bcdDevice rel. 2.00 */
	0x02,
	USBD_IDX_MFC_STR,             /* Index of manufacturer string */
	USBD_IDX_PRODUCT_STR,         /* Index of product string */
	USBD_IDX_SERIAL_STR,          /* Index of serial number string */
	USBD_CFG_MAX_NUM              /* bNumConfigurations */
}

;                               /* USB_DeviceDescriptor */



uint8_t USBD_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] = {
	USB_LEN_DEV_QUALIFIER_DESC,
	USB_DESC_TYPE_DEVICE_QUALIFIER,
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	0x40,
	0x01,
	0x00,
};

uint8_t USBD_LangIDDesc[USB_SIZ_STRING_LANGID] =
{
	USB_SIZ_STRING_LANGID,         
	USB_DESC_TYPE_STRING,       
	LOBYTE(USBD_LANGID_STRING),
	HIBYTE(USBD_LANGID_STRING), 
};

uint8_t *  get_Device( uint8_t speed , uint16_t *length)
{
	*length = sizeof(USBD_DeviceDesc);
	return USBD_DeviceDesc;
}

uint8_t *  get_LangID( uint8_t speed , uint16_t *length)
{
	*length =  sizeof(USBD_LangIDDesc);  
	return USBD_LangIDDesc;
}

uint8_t *  get_Product( uint8_t speed , uint16_t *length)
{
	USBD_GetString (USBD_PRODUCT_FS_STRING, USBD_StrDesc, length);    
	return USBD_StrDesc;
}

uint8_t *  get_Manufacturer( uint8_t speed , uint16_t *length)
{
	USBD_GetString (USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

uint8_t *  get_Serial( uint8_t speed , uint16_t *length)
{
	USBD_GetString (USBD_SERIALNUMBER_FS_STRING, USBD_StrDesc, length);    
	return USBD_StrDesc;
}

uint8_t *  get_Config( uint8_t speed , uint16_t *length)
{
	USBD_GetString (USBD_CONFIGURATION_FS_STRING, USBD_StrDesc, length); 
	return USBD_StrDesc;  
}

uint8_t *  get_Interface( uint8_t speed , uint16_t *length)
{
	USBD_GetString (USBD_INTERFACE_FS_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;  
} 
