#include "stm324xg_usb_audio_codec.h"
#include "usbd_audio_core.h"
#include "i2s.h"
#include "malloc.h"
#include "usart.h"

uint8_t volume=0;							//当前音量  

volatile uint8_t audiostatus=0;				//bit0:0,暂停播放;1,继续播放   
volatile uint16_t i2splaybuf=0;				//即将播放的音频帧缓冲编号
volatile uint16_t i2ssavebuf=0;				//当前保存到的音频缓冲编号 
#define AUDIO_BUF_NUM		100				//由于采用的是USB同步传输数据播放
											//而STM32 IIS的速度和USB传送过来数据的速度存在差异,比如在48Khz下,实
											//际IIS是低于48Khz(47.991Khz)的,所以电脑送过来的数据流,会比STM32播放
											//速度快,缓冲区写位置追上播放位置(i2ssavebuf==i2splaybuf)时,就会出现
											//混叠.设置尽量大的AUDIO_BUF_NUM值,可以尽量减少混叠次数. 
								
uint8_t *i2sbuf[AUDIO_BUF_NUM];				//音频缓冲帧,占用内存数=AUDIO_BUF_NUM*AUDIO_OUT_PACKET 字节
  
//音频数据I2S DMA传输回调函数
void audio_i2s_dma_callback(void) 
{      
	if((i2splaybuf==i2ssavebuf)&&audiostatus==0) 
		DMA_Stream->CR&=~(1<<0);
	else
	{
		i2splaybuf++;
		if(i2splaybuf>(AUDIO_BUF_NUM-1))i2splaybuf=0;
		if(DMA_Stream->CR&(1<<19)) 
			DMA_Stream->M0AR=(uint32_t)i2sbuf[i2splaybuf];
		else  		
			DMA_Stream->M1AR=(uint32_t)i2sbuf[i2splaybuf];
	}
}  

//配置音频接口
//OutputDevice:输出设备选择,未用到.
//Volume:音量大小,0~100
//AudioFreq:音频采样率
uint32_t EVAL_AUDIO_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{   
	uint16_t t=0;
	for(t=0;t<AUDIO_BUF_NUM;t++)		//内存申请 
		i2sbuf[t]=malloc(AUDIO_OUT_PACKET);
	if(i2sbuf[AUDIO_BUF_NUM-1]==NULL)	//内存申请失败
	{
		for(t=0;t<AUDIO_BUF_NUM;t++)free(i2sbuf[t]); 
		return 1;
	} 
	I2S_Init(0,2,0,1);					//飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度
	I2S_SampleRate_Set(AudioFreq);		//设置采样率
	EVAL_AUDIO_VolumeCtl(Volume);		//设置音量
	I2S_TX_DMA_Init(i2sbuf[0],i2sbuf[1],AUDIO_OUT_PACKET/2); 
	I2S_Play_Start();							//开启DMA  
	return 0; 
}
 
//开始播放音频数据
//pBuffer:音频数据流首地址指针
//Size:数据流大小(单位:字节)
uint32_t EVAL_AUDIO_Play(uint16_t* pBuffer, uint32_t Size)
{  
	return 0;
}
 
//暂停/恢复音频流播放
//Cmd:0,暂停播放;1,恢复播放
//Addr:音频数据流缓存首地址
//Size:音频数据流大小(单位:harf word,也就是2个字节 
//返回值:0,成功
//    其他,设置失败
uint32_t EVAL_AUDIO_PauseResume(uint32_t Cmd, uint32_t Addr, uint32_t Size)
{    
	uint16_t i;
	uint8_t *p=(uint8_t*)Addr;
	if(Cmd==AUDIO_PAUSE)
	{
 		audiostatus=0; 
	}else
	{
		audiostatus=1;
		i2ssavebuf++;
		if(i2ssavebuf>(AUDIO_BUF_NUM-1))i2ssavebuf=0;
		for(i=0;i<AUDIO_OUT_PACKET;i++)
		{
			i2sbuf[i2ssavebuf][i]=p[i];
		}
		I2S_Play_Start();				//开启DMA  
	} 
	return 0;
}
 
//停止播放
//Option:控制参数,1/2,详见:CODEC_PDWN_HW定义
//返回值:0,成功
//    其他,设置失败
uint32_t EVAL_AUDIO_Stop(uint32_t Option)
{ 
	audiostatus=0;
	return 0;
} 
//音量设置 
//Volume:0~100
//返回值:0,成功
//    其他,设置失败
uint32_t EVAL_AUDIO_VolumeCtl(uint8_t Volume)
{ 
	volume=Volume; 
	return 0;
} 
//静音控制
//Cmd:0,正常
//    1,静音
//返回值:0,正常
//    其他,错误代码
uint32_t EVAL_AUDIO_Mute(uint32_t Cmd)
{  
	if(Cmd==AUDIO_MUTE_ON)
	{
	}else
	{

	}
	return 0;
}  
//播放音频数据流
//Addr:音频数据流缓存首地址
//Size:音频数据流大小(单位:harf word,也就是2个字节)
void Audio_MAL_Play(uint32_t Addr, uint32_t Size)
{  
	uint16_t i;
	uint16_t t=i2ssavebuf;
	uint8_t *p=(uint8_t*)Addr;
	uint16_t curplay=i2splaybuf;	//当前正在播放的缓存帧编号
	if(curplay)
	curplay--;
	else 
	curplay=AUDIO_BUF_NUM-1; 
	audiostatus=1;
	t++;
	if(t>(AUDIO_BUF_NUM-1))t=0; 
	if(t==curplay)			//写缓存碰上了当前正在播放的帧,跳到下一帧
	{
		t++;
		if(t>(AUDIO_BUF_NUM-1))t=0;
		#ifdef DEBUG
			print("bad position:%d\r\n",t);
		#endif
		
	}
	i2ssavebuf=t;
	for(i=0;i<Size*2;i++)
		i2sbuf[i2ssavebuf][i]=p[i];
	I2S_Play_Start();		//开启DMA  
}






















