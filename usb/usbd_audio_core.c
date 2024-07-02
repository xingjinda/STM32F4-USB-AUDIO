#include "usbd_audio_core.h"
#include "usbd_audio_out_if.h"

//音频设备库回调
static uint8_t  usbd_audio_Init       (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_audio_DeInit     (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_audio_Setup      (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_audio_EP0_RxReady(void *pdev);
static uint8_t  usbd_audio_DataIn     (void *pdev, uint8_t epnum);
static uint8_t  usbd_audio_DataOut    (void *pdev, uint8_t epnum);
static uint8_t  usbd_audio_SOF        (void *pdev);
static uint8_t  usbd_audio_OUT_Incplt (void  *pdev);

//音频请求管理功能
static uint8_t  usbd_audio_hid_Setup      (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_hid_Setup      (void  *pdev, USB_SETUP_REQ *req);
static void AUDIO_Req_GetCurrent		(void *pdev, USB_SETUP_REQ *req);
static void AUDIO_Req_SetCurrent		(void *pdev, USB_SETUP_REQ *req);
static uint8_t  *usbd_audio_GetCfgDesc (uint8_t speed, uint16_t *length);




static uint32_t  USBD_HID_Protocol = 0;
static uint32_t  USBD_HID_IdleState= 0;
static uint32_t  USBD_HID_AltSet= 0;

//用于音频数据输出传输的主缓冲区及其相关指针
uint8_t  IsocOutBuff [TOTAL_OUT_BUF_SIZE * 2];
uint8_t* IsocOutWrPtr = IsocOutBuff;
uint8_t* IsocOutRdPtr = IsocOutBuff;

//音频控制请求传输及其相关变量的主缓冲区
uint8_t  AudioCtl[64];
uint8_t  AudioCtlCmd = 0;
uint32_t AudioCtlLen = 0;
uint8_t  AudioCtlUnit = 0;

static volatile unsigned int VOL_MIN=0;
static volatile unsigned int VOL_MAX=100;
static volatile unsigned int VOL_RES=1;

static uint32_t PlayFlag = 0;

static __IO uint32_t  usbd_audio_AltSet = 0;
static uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE];

//音频接口类回调结构
USBD_Class_cb_TypeDef  AUDIO_cb = 
{
	usbd_audio_Init,
	usbd_audio_DeInit,
	usbd_audio_hid_Setup,
	NULL,
	usbd_audio_EP0_RxReady,
	usbd_audio_DataIn,
	usbd_audio_DataOut,
	usbd_audio_SOF,
	NULL,
	usbd_audio_OUT_Incplt,   
	usbd_audio_GetCfgDesc,    
};

static uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE] =
{
	// Configuration 1
	0x09,									// bLength
	USB_CONFIGURATION_DESCRIPTOR_TYPE,		// bDescriptorType 
	DESC_SIZE(AUDIO_CONFIG_DESC_SIZE),		// 总大小   
	0x03,                                	// bNumInterfaces 
	0x01,                                	// bConfigurationValue 
	0x00,                                 	// iConfiguration 
	0xC0,                                 	// bmAttributes  BUS Powred
	0x16,                                 	// 最大电流
	// 09 byte

	// USB Speaker Standard interface descriptor 
	AUDIO_INTERFACE_DESC_SIZE,            // bLength 
	USB_INTERFACE_DESCRIPTOR_TYPE,        // bDescriptorType 
	0x00,                                 // bInterfaceNumber 
	0x00,                                 // bAlternateSetting 
	0x00,                                 // bNumEndpoints 
	USB_DEVICE_CLASS_AUDIO,               // bInterfaceClass 
	AUDIO_SUBCLASS_AUDIOCONTROL,          // bInterfaceSubClass 
	AUDIO_PROTOCOL_UNDEFINED,             // bInterfaceProtocol 
	0x00,                                 // iInterface 
	// 09 byte

	// USB Speaker Class-specific AC Interface Descriptor 
	AUDIO_INTERFACE_DESC_SIZE,            	// bLength 
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      	// bDescriptorType 
	AUDIO_CONTROL_HEADER,                 	// bDescriptorSubtype 
	0x00,									// bcdADC 
	0x01,									// 1.00 
	0x27,									// wTotalLength = 39
	0x00,
	0x01,									// bInCollection 
	0x01,									// baInterfaceNr 
	// 09 byte

	// USB Speaker Input Terminal Descriptor 
	AUDIO_INPUT_TERMINAL_DESC_SIZE,       // bLength 
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType 
	AUDIO_CONTROL_INPUT_TERMINAL,         // bDescriptorSubtype 
	0x01,                                 // bTerminalID 
	0x01,                                 // wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 
	0x01,
	0x00,                                 // bAssocTerminal 
	0x01,                                 // bNrChannels 
	0x00,                                 // wChannelConfig 0x0000  Mono 
	0x00,
	0x00,                                 // iChannelNames 
	0x00,                                 // iTerminal 
	// 12 byte

	// USB Speaker Audio Feature Unit Descriptor 
	0x09,                                 // bLength 
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType 
	AUDIO_CONTROL_FEATURE_UNIT,           // bDescriptorSubtype 
	AUDIO_OUT_STREAMING_CTRL,             // bUnitID 
	0x01,                                 // bSourceID 
	0x01,                                 // bControlSize 
	AUDIO_CONTROL_MUTE|AUDIO_CONTROL_VOLUME,                   // bmaControls(0) 
	0x00,                                 // bmaControls(1) 
	0x00,                                 // iTerminal 
	// 09 byte

	//USB Speaker Output Terminal Descriptor 
	0x09,									// bLength 
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      	// bDescriptorType 
	AUDIO_CONTROL_OUTPUT_TERMINAL,        	// bDescriptorSubtype 
	0x03,                                 	// bTerminalID
	0x01,                                 	// wTerminalType  0x0301
	0x03,
	0x00,                                 // bAssocTerminal 
	0x02,                                 // bSourceID 
	0x00,                                 // iTerminal 
	// 09 byte

	// USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith 
	// Interface 1, Alternate Setting 0                                             
	AUDIO_INTERFACE_DESC_SIZE,				// bLength 
	USB_INTERFACE_DESCRIPTOR_TYPE,        	// bDescriptorType 
	0x01,                                 	// bInterfaceNumber 
	0x00,                                 	// bAlternateSetting 
	0x00,                                 	// bNumEndpoints 
	USB_DEVICE_CLASS_AUDIO,               	// bInterfaceClass 
	AUDIO_SUBCLASS_AUDIOSTREAMING,        	// bInterfaceSubClass 
	AUDIO_PROTOCOL_UNDEFINED,             	// bInterfaceProtocol 
	0x00,                                 	// iInterface 
	// 09 byte

	// USB Speaker Standard AS Interface Descriptor - Audio Streaming Operational 
	// Interface 1, Alternate Setting 1                                           
	AUDIO_INTERFACE_DESC_SIZE,				// bLength
	USB_INTERFACE_DESCRIPTOR_TYPE,			// bDescriptorType 
	0x01,                                 	// bInterfaceNumber 
	0x01,                                 	// bAlternateSetting 
	0x01,                                 	// bNumEndpoints 
	USB_DEVICE_CLASS_AUDIO,               	// bInterfaceClass 
	AUDIO_SUBCLASS_AUDIOSTREAMING,        	// bInterfaceSubClass 
	AUDIO_PROTOCOL_UNDEFINED,             	// bInterfaceProtocol 
	0x00,									// iInterface
	/* 09 byte*/

	// USB Speaker Audio Streaming Interface Descriptor 
	AUDIO_STREAMING_INTERFACE_DESC_SIZE,  // bLength 
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType 
	AUDIO_STREAMING_GENERAL,              // bDescriptorSubtype 
	0x01,                                 // bTerminalLink 
	0x01,                                 // bDelay 
	0x01,                                 // wFormatTag AUDIO_FORMAT_PCM  0x0001
	0x00,
	// 07 byte

	// USB Speaker Audio Type III Format Interface Descriptor
	0x0B,									// bLength
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,		// bDescriptorType
	AUDIO_STREAMING_FORMAT_TYPE,			// bDescriptorSubtype
	AUDIO_FORMAT_TYPE_III,                	// bFormatType 
	0x02,									// bNrChannels
	0x02,									// bSubFrameSize :  2 Bytes per frame (16bits)
	16,										// bBitResolution (16-bits per sample)
	0x01,									// bSamFreqType only one frequency supported
	SAMPLE_FREQ(USBD_AUDIO_FREQ),			// Audio sampling frequency coded on 3 bytes
	// 11 byte

	// Endpoint 1 - Standard Descriptor
	AUDIO_STANDARD_ENDPOINT_DESC_SIZE,		// bLength
	USB_ENDPOINT_DESCRIPTOR_TYPE,			// bDescriptorType
	AUDIO_OUT_EP,							// bEndpointAddress 1 out endpoint
	USB_ENDPOINT_TYPE_ISOCHRONOUS,			// bmAttributes
	AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),		// wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord))
	0x01,									// bInterval
	0x00,									// bRefresh
	0x00,									// bSynchAddress
	// 09 byte

	// Endpoint - Audio Streaming Descriptor
	AUDIO_STREAMING_ENDPOINT_DESC_SIZE,		// bLength
	AUDIO_ENDPOINT_DESCRIPTOR_TYPE,			// bDescriptorType
	AUDIO_ENDPOINT_GENERAL,					// bDescriptor
	0x00,									// bmAttributes
	0x00,									// bLockDelayUnits
	0x00,									// wLockDelay
	0x00,
	// 07 byte

	//						操纵杆界面描述符
	0x09,							//bLength: Interface Descriptor size
	USB_INTERFACE_DESCRIPTOR_TYPE,	//bDescriptorType: Interface descriptor type
	0x02,							//bInterfaceNumber: Number of Interface
	0x00,							//bAlternateSetting: Alternate setting
	0x01,							//bNumEndpoints
	0x03,							//bInterfaceClass: HID
	0x01,							//bInterfaceSubClass : 1=BOOT, 0=no boot
	0x02,							//nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse
	0,								//iInterface: Index of string descriptor
	//						操纵杆HID描述符
	0x09,							//bLength: HID Descriptor size
	HID_DESCRIPTOR_TYPE,			//bDescriptorType: HID
	0x11,							//bcdHID: HID Class Spec release number
	0x01,
	0x00,							//bCountryCode: Hardware target country
	0x01,							//bNumDescriptors: Number of HID class descriptors to follow
	0x22,							//bDescriptorType
	JOYSTICK_REPORT_DESC_SIZE,		//wItemLength: Total length of Report descriptor`
	0x00,
	//						操纵杆端点描述符
	0x07,							//端点描述符大小
	USB_ENDPOINT_DESCRIPTOR_TYPE,	//bDescriptorType:
	HID_IN_EP,						//端点地址
	0x03,          					//端点号
	HID_OUT_PACKET, 				//端点字节大小
	0x00,
	0x0A,							//bInterval: Polling Interval (10 ms)
} ;

static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ] =
{
	/* 18 */
	0x09,					//bLength: HID Descriptor size
	HID_DESCRIPTOR_TYPE,	//bDescriptorType: HID
	0x11,					//bcdHID: HID Class Spec release number
	0x01,
	0x00,					//bCountryCode: Hardware target country
	0x01,					//bNumDescriptors: Number of HID class descriptors to follow
	0x22,					//bDescriptorType
	JOYSTICK_REPORT_DESC_SIZE,//wItemLength: Total length of Report descriptor
	0x00,
};

static uint8_t joystick_reportdesc[JOYSTICK_REPORT_DESC_SIZE]=
{
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,                    // USAGE (Game Pad)
	0xa1, 0x01,                    // COLLECTION (Application)

	0x09, 0x01,                    //   USAGE (Pointer)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x09, 0x30,                    //     USAGE (X)
	0x09, 0x31,                    //     USAGE (Y)
	0x09, 0x32,
	0x09, 0x33,
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)	
	0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
	0x46, 0xff, 0x7f,              //     PHYSICAL_MAXIMUM (32767)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x02,                    //     REPORT_COUNT (2)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0xc0,                          //     END_COLLECTION

	0x09, 0x39,                    //   USAGE (Hat switch)
	0x15, 0x01,                    //   LOGICAL_MINIMUM (1)
	0x25, 0x08,                    //   LOGICAL_MAXIMUM (8)
	0x75, 0x04,                    //   REPORT_SIZE (4)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x81, 0x42,                    //   INPUT (Data,Var,Abs,Null)

	0x05, 0x09,                    //   USAGE_PAGE (Button)
	0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
	0x29, 0x14,                    //   USAGE_MAXIMUM (Button 12)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,                    //   REPORT_SIZE (1)
	0x95, 0x14,                    //   REPORT_COUNT (12)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)
	0xc0                           // END_COLLECTION
};

//初始化AUDIO接口
static uint8_t  usbd_audio_Init (void  *pdev, uint8_t cfgidx)
{  
	//打开音频端点输出  
	DCD_EP_Open(pdev,AUDIO_OUT_EP,AUDIO_OUT_PACKET,USB_OTG_EP_ISOC);
	//打开键盘端点输出
	DCD_EP_Open(pdev,HID_IN_EP,HID_OUT_PACKET,USB_OTG_EP_INT);
	//初始化音频输出硬件层
	if (AUDIO_OUT_fops.Init(USBD_AUDIO_FREQ, DEFAULT_VOLUME, 0) != USBD_OK)
	{
	return USBD_FAIL;
	}
	//准备输出端点接收音频数据
	DCD_EP_PrepareRx(pdev,AUDIO_OUT_EP,IsocOutBuff,AUDIO_OUT_PACKET);  

	return USBD_OK;
}

//取消初始化AUDIO接口
static uint8_t  usbd_audio_DeInit (void  *pdev,uint8_t cfgidx)
{ 
	DCD_EP_Close (pdev ,AUDIO_OUT_EP);
	DCD_EP_Close (pdev ,HID_IN_EP);
	//取消初始化音频输出硬件层
	if (AUDIO_OUT_fops.DeInit(0) != USBD_OK)
	{
	return USBD_FAIL;
	}
	return USBD_OK;
}

//处理音频和键盘控制请求解析
static uint8_t  usbd_audio_hid_Setup (void  *pdev,USB_SETUP_REQ *req)
{
	switch (req->bmRequest & 0x03)
	{
		case 0x01:
			if(req->wIndex!=2)
				return usbd_audio_Setup(pdev,req);
			return usbd_hid_Setup(pdev,req);
		case 0x02:
		{
			if(req->wIndex!=0x81)
				return usbd_audio_Setup(pdev,req);
			return usbd_hid_Setup(pdev,req);
		}
					
	}
	return USBD_OK;
}

//处理键盘控制请求解析
static uint8_t  usbd_hid_Setup (void  *pdev,USB_SETUP_REQ *req)
{
  uint16_t len = USB_HID_DESC_SIZ;
  uint8_t  *pbuf = usbd_audio_CfgDesc + 18;
  
	switch (req->bmRequest & USB_REQ_TYPE_MASK)
	{
		case USB_REQ_TYPE_CLASS :  
		switch (req->bRequest)
		{
			case HID_REQ_GET_IDLE:
				USBD_CtlSendData (pdev,(uint8_t *)&USBD_HID_IdleState,1);        
				break;
			case HID_REQ_GET_PROTOCOL:
				USBD_CtlSendData (pdev,(uint8_t *)&USBD_HID_Protocol,1);    
				break;
			case HID_REQ_SET_IDLE:
				USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
				break;
			case HID_REQ_SET_PROTOCOL:
				USBD_HID_Protocol = (uint8_t)(req->wValue);
				break;
			default:
				USBD_CtlError (pdev, req);
			return USBD_FAIL; 
		}
    	break;
    
	case USB_REQ_TYPE_STANDARD:
	switch (req->bRequest)
	{
		case 1:
			
			break;
		case USB_REQ_GET_DESCRIPTOR: 
			if( req->wValue >> 8 == HID_REPORT_DESC)
			{

				len = MIN(JOYSTICK_REPORT_DESC_SIZE , req->wLength);
				pbuf = joystick_reportdesc;
				USBD_CtlSendData (pdev, pbuf,len);

			}
			else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
			{
				pbuf = USBD_HID_Desc;
				len = MIN(USB_HID_DESC_SIZ , req->wLength);
				USBD_CtlSendData (pdev, pbuf,len);
			}
			break;
		case USB_REQ_GET_INTERFACE :
			USBD_CtlSendData (pdev,(uint8_t *)&USBD_HID_AltSet,1);
		break;

		case USB_REQ_SET_INTERFACE :
			USBD_HID_AltSet = (uint8_t)(req->wValue);
		break;
	}
  }
  return USBD_OK;
}

//处理音频控制请求解析
static uint8_t  usbd_audio_Setup (void  *pdev,USB_SETUP_REQ *req)
{
  uint16_t len=USB_AUDIO_DESC_SIZ;
  uint8_t  *pbuf=usbd_audio_CfgDesc + 18;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :   
    switch (req->bRequest)
    {
		case AUDIO_REQ_SET_CUR:
			AUDIO_Req_SetCurrent(pdev, req);   
			break;
		case AUDIO_REQ_GET_CUR:
			AUDIO_Req_GetCurrent(pdev, req);
			break;
		case AUDIO_REQ_GET_MIN:
        	USBD_CtlSendData(pdev, (uint8_t *)&VOL_MIN, req->wLength);
        	break;
      	case AUDIO_REQ_GET_MAX:
        	USBD_CtlSendData(pdev, (uint8_t *)&VOL_MAX, req->wLength);
			break;
      	case AUDIO_REQ_GET_RES:
        	USBD_CtlSendData(pdev, (uint8_t *)&VOL_RES, req->wLength);
			break; 
		default:
      	USBD_CtlError (pdev, req);
      	return USBD_FAIL;
    }
    break;
    
	case USB_REQ_TYPE_STANDARD:
		switch (req->bRequest)
		{
		case USB_REQ_GET_DESCRIPTOR:
			if( (req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
			{

				pbuf = usbd_audio_CfgDesc + 18;
				len = MIN(USB_AUDIO_DESC_SIZ,req->wLength);
				USBD_CtlSendData (pdev, pbuf,len);
			}
			break;
		case USB_REQ_GET_INTERFACE :
			USBD_CtlSendData (pdev,(uint8_t *)&usbd_audio_AltSet,1);
			break;
		case USB_REQ_SET_INTERFACE :
			if ((uint8_t)(req->wValue) < AUDIO_TOTAL_IF_NUM)
				usbd_audio_AltSet = (uint8_t)(req->wValue);
			else
				USBD_CtlError (pdev, req);
		break;
		}
	}
	return USBD_OK;
}

//处理音频控制请求数据
static uint8_t  usbd_audio_EP0_RxReady (void  *pdev)
{ 
	//检查是否发出了AudioControl请求
	if (AudioCtlCmd == AUDIO_REQ_SET_CUR)
	{// 在这个驱动程序中，为了简化代码，只管理SET CUR请求
	// 检查AudioControl请求已发出的地址单元 
		if (AudioCtlUnit == AUDIO_OUT_STREAMING_CTRL)
		{// 在这个驱动程序中，为了简化代码，只管理一个单元
			//调用音频接口静音功能
			AUDIO_OUT_fops.MuteCtl(AudioCtl[0]);
			
			//重置AudioCtlCmd变量以防止重新进入该函数
			AudioCtlCmd = 0;
			AudioCtlLen = 0;
		}
	} 

	return USBD_OK;
}

//处理音频输入数据
static uint8_t  usbd_audio_DataIn (void *pdev, uint8_t epnum)
{
	DCD_EP_Flush(pdev,HID_IN_EP);
	return USBD_OK;
}

//处理音频输出数据
static uint8_t  usbd_audio_DataOut (void *pdev, uint8_t epnum)
{ 
	if (epnum == AUDIO_OUT_EP)
	{    
	// 当所有缓冲区都已满时，递增缓冲区指针或将其回滚
		if (IsocOutWrPtr >= (IsocOutBuff + (AUDIO_OUT_PACKET * OUT_PACKET_NUM)))
		{//所有缓冲区都已满:回滚
			IsocOutWrPtr = IsocOutBuff;
		}
		else
		{// 增加缓冲区指针
			IsocOutWrPtr += AUDIO_OUT_PACKET;
		}

		// 切换帧索引 
		((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame = (((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame)? 0:1;
			
		//准备Out端点接收下一个音频包
		DCD_EP_PrepareRx(pdev,AUDIO_OUT_EP,(uint8_t*)(IsocOutWrPtr),AUDIO_OUT_PACKET);
			
		//只有当一半缓冲区已满时才触发流开始
		if ((PlayFlag == 0) && (IsocOutWrPtr >= (IsocOutBuff + ((AUDIO_OUT_PACKET * OUT_PACKET_NUM) / 2))))
		{
			//启用流开始
			PlayFlag = 1;
		}
	}
	return USBD_OK;
}

//处理SOF事件(数据缓冲区更新和同步)
static uint8_t  usbd_audio_SOF (void *pdev)
{     
	//检查流缓冲区中是否有可用数据。在此函数中，使用单个变量(PlayFlag)来避免软件延迟。检测到软信号后，应尽快执行播放操作。
	if (PlayFlag)
	{      
		//开始播放接收到的数据包
		AUDIO_OUT_fops.AudioCmd((uint8_t*)(IsocOutRdPtr),AUDIO_OUT_PACKET,AUDIO_CMD_PLAY);           

		//增加缓冲区指针或回滚，当所有缓冲区全部满
		if (IsocOutRdPtr >= (IsocOutBuff + (AUDIO_OUT_PACKET * OUT_PACKET_NUM)))
		{
			// 回滚到缓冲区的开始
			IsocOutRdPtr = IsocOutBuff;
		}
		else
		{
			//递增到下一个子缓冲区
			IsocOutRdPtr += AUDIO_OUT_PACKET;
		}

		//如果所有可用缓冲区已被消耗，则停止播放
		if (IsocOutRdPtr == IsocOutWrPtr)
		{    
			//暂停播放
			AUDIO_OUT_fops.AudioCmd((uint8_t*)(IsocOutBuff),AUDIO_OUT_PACKET,AUDIO_CMD_PAUSE);
			
			//停止进入播放循环
			PlayFlag = 0;
			
			//重置缓冲区指针
			IsocOutRdPtr = IsocOutBuff;
			IsocOutWrPtr = IsocOutBuff;
		}
	}
	return USBD_OK;
}

static uint8_t  usbd_audio_OUT_Incplt (void  *pdev)
{
  return USBD_OK;
}

static void AUDIO_Req_GetCurrent(void *pdev, USB_SETUP_REQ *req)
{
 unsigned char *haudio=AudioCtl;
	switch(req->wValue>>8)
	{
		case	1:
		{
			if(req->bRequest==0x81)
			{
				*haudio=0;
			}
			else if(req->bRequest==0x82)
			{
				*haudio=0;
			}
			else if(req->bRequest==0x83)
			{
				*haudio=1;
			}
			else
			{
				*haudio=1;
			}
		}
		case	2:
		{
			if(req->bRequest==0x81)
			{
				*haudio=0x14;
			}
			else if(req->bRequest==0x82)
			{
				*haudio=0x00;
			}
			else if(req->bRequest==0x83)
			{
				*haudio=0x64;
			}
			else
			{
				*haudio=1;
			}
		}
	}
			
  USBD_CtlSendData (pdev,AudioCtl,req->wLength);
}

static void AUDIO_Req_SetCurrent(void *pdev, USB_SETUP_REQ *req)
{ 
	if (req->wLength)
	{
		//准备EPO缓冲液的接收
		USBD_CtlPrepareRx (pdev, AudioCtl,req->wLength);

		AudioCtlCmd = AUDIO_REQ_SET_CUR;
		AudioCtlLen = req->wLength; 
		AudioCtlUnit = HIBYTE(req->wIndex);
	}
}

static uint8_t  *usbd_audio_GetCfgDesc(uint8_t speed, uint16_t *length)
{
	*length = sizeof (usbd_audio_CfgDesc);
	return usbd_audio_CfgDesc;
}

unsigned char usbd_sendreport(USB_OTG_CORE_HANDLE  *pdev, unsigned char *report,unsigned short len)
{
	if (pdev->dev.device_status == USB_OTG_CONFIGURED )
	{
		DCD_EP_Tx (pdev, HID_IN_EP, report, len);
	}
	return USBD_OK;
}
