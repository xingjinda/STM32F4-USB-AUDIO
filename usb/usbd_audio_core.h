#ifndef __USB_AUDIO_CORE_H_
#define __USB_AUDIO_CORE_H_

#include "usbd_ioreq.h"
#include "usbd_req.h"
#include "usbd_desc.h"



// 音频采样率 x 数据大小(2 bytes) x 通道数量(Stereo: 2)
#define AUDIO_OUT_PACKET                              (uint32_t)(((USBD_AUDIO_FREQ/1000) * 2 * 2) ) 

//音频传输缓冲区中的子包数。您可以修改这个值，但要确保它是偶数且大于3
#define OUT_PACKET_NUM                                   4
// 音频传输缓冲区的总大小 
#define TOTAL_OUT_BUF_SIZE                           ((uint32_t)(AUDIO_OUT_PACKET * OUT_PACKET_NUM))

#define AUDIO_CONFIG_DESC_SIZE                          109
#define AUDIO_INTERFACE_DESC_SIZE                       9
#define USB_AUDIO_DESC_SIZ                              0x09
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE               0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE              0x07

#define AUDIO_DESCRIPTOR_TYPE                           0x21
#define USB_DEVICE_CLASS_AUDIO                          0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                     0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                   0x02
#define AUDIO_PROTOCOL_UNDEFINED                        0x00
#define AUDIO_STREAMING_GENERAL                         0x01
#define AUDIO_STREAMING_FORMAT_TYPE                     0x02

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE                 0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                  0x25

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                            0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                    0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                   0x03
#define AUDIO_CONTROL_FEATURE_UNIT                      0x06

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                  0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE                 0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE             0x07

#define AUDIO_CONTROL_MUTE                              0x0001
#define AUDIO_CONTROL_VOLUME                            0x0002

#define AUDIO_FORMAT_TYPE_I                             0x01
#define AUDIO_FORMAT_TYPE_III                           0x03

#define USB_ENDPOINT_TYPE_ISOCHRONOUS                   0x01
#define AUDIO_ENDPOINT_GENERAL                          0x01

#define AUDIO_REQ_GET_CUR                               0x81
#define AUDIO_REQ_SET_CUR                               0x01
#define AUDIO_REQ_GET_MIN                               0x82
#define AUDIO_REQ_GET_MAX                               0x83
#define AUDIO_REQ_GET_RES                               0x84


#define AUDIO_OUT_STREAMING_CTRL                        0x02


typedef struct _Audio_Fops
{
    uint8_t  (*Init)         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
    uint8_t  (*DeInit)       (uint32_t options);
    uint8_t  (*AudioCmd)     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
    uint8_t  (*VolumeCtl)    (uint8_t vol);
    uint8_t  (*MuteCtl)      (uint8_t cmd);
    uint8_t  (*PeriodicTC)   (uint8_t cmd);
    uint8_t  (*GetState)     (void);
}AUDIO_FOPS_TypeDef;

#define DESC_SIZE(x)                   ((uint8_t)((x) & 0x00FF)),(uint8_t)(((x) & 0xFF00) >>8)

 //最大数据包大小: (音频采样率/ 1000) * channels * bytes_per_sample
 // e.g. (48000 / 1000) * 2(stereo) * 2(16bit) = 192

#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq/1000) * 2 * 2) & 0xFF),(uint8_t)((((frq/1000) * 2 * 2) >> 8) & 0xFF)

#define SAMPLE_FREQ(frq)               (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))


extern USBD_Class_cb_TypeDef  AUDIO_cb;

#endif

