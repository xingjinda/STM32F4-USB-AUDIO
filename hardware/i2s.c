#include "i2s.h"

//采样率计算公式:Fs=I2SxCLK/[256*(2*I2SDIV+ODD)]
//I2SxCLK=(HSE/pllm)*PLLI2SN/PLLI2SR
//一般HSE=8Mhz 
//pllm:在Sys_Clock_Set设置的时候确定，一般是8
//PLLI2SN:一般是192~432 
//PLLI2SR:2~7
//I2SDIV:2~255
//ODD:0/1
//I2S分频系数表@pllm=8,HSE=8Mhz,即vco输入频率为1Mhz
//表格式:采样率/10,PLLI2SN,PLLI2SR,I2SDIV,ODD
const uint16_t I2S_PSC_TBL[][5]=
{
	{800 ,256,5,12,1},		//8Khz采样率
	{1102,429,4,19,0},		//11.025Khz采样率 
	{1600,213,2,13,0},		//16Khz采样率
	{2205,429,4, 9,1},		//22.05Khz采样率
	{3200,213,2, 6,1},		//32Khz采样率
	{4410,271,2, 6,0},		//44.1Khz采样率
	{4800,258,3, 3,1},		//48Khz采样率
	{8820,316,2, 3,1},		//88.2Khz采样率
	{9600,344,2, 3,1},		//96Khz采样率
	{17640,361,2,2,0},		//176.4Khz采样率 
	{19200,393,2,2,0},		//192Khz采样率
};

#if defined(I2S2)
//I2S2初始化
//std:I2S标准,00,飞利浦标准;01,MSB对齐标准(右对齐);10,LSB对齐标准(左对齐);11,PCM标准
//mode:I2S工作模式,00,从机发送;01,从机接收;10,主机发送;11,主机接收
//cpol:0,时钟低电平有效;1,时钟高电平有效
//datalen:数据长度,0,16位标准;1,16位扩展(frame=32bit);2,24位;3,32位.
void I2S_Init(uint8_t std,uint8_t mode,uint8_t cpol,uint8_t datalen)
{ 
	RCC->APB1ENR|=1<<14;			//使能SPI2时钟

	SPI2->I2SCFGR=0;			//全部设置为0
	SPI2->I2SPR=0X02;			//分频寄存器为默认值
	SPI2->I2SCFGR|=1<<11;		//选择:I2S模式
	SPI2->I2SCFGR|=mode<<8;		//I2S工作模式设置
	SPI2->I2SCFGR|=std<<4;		//I2S标准设置 
	SPI2->I2SCFGR|=cpol<<3;		//空闲时钟电平设置
	if(datalen)					//非标准16位长度
	{
		SPI2->I2SCFGR|=1<<0;	//Channel长度为32位 
		datalen-=1;		
	}else SPI2->I2SCFGR|=0<<0;	//Channel长度为16位  
	SPI2->I2SCFGR|=datalen<<1;	//I2S标准设置 
	SPI2->CR2|=1<<1;			//SPI2 TX DMA请求使能.
	SPI2->I2SCFGR|=1<<10;		//SPI2 I2S EN使能.	
} 
  
//设置IIS的采样率(@MCKEN)
//samplerate:采样率,单位:Hz
//返回值:0,设置成功;1,无法设置.
uint8_t I2S_SampleRate_Set(uint32_t samplerate)
{ 
	uint8_t i=0; 
	uint32_t tempreg=0;
	samplerate/=10;//缩小10倍   
	for(i=0;i<(sizeof(I2S_PSC_TBL)/10);i++)//看看改采样率是否可以支持
	{
		if(samplerate==I2S_PSC_TBL[i][0])break;
	}
	RCC->CR&=~(1<<26);						//先关闭PLLI2S
	if(i==(sizeof(I2S_PSC_TBL)/10))return 1;//搜遍了也找不到
	tempreg|=12;		//设置PLLI2SM
	tempreg|=(uint32_t)I2S_PSC_TBL[i][1]<<6;		//设置PLLI2SN
	tempreg|=(uint32_t)I2S_PSC_TBL[i][2]<<28;	//设置PLLI2SR
	RCC->PLLI2SCFGR=tempreg;		//设置I2SxCLK的频率(x=2) 
	RCC->CR|=1<<26;					//开启I2S时钟
	while((RCC->CR&1<<27)==0);		//等待I2S时钟开启成功. 
	tempreg=I2S_PSC_TBL[i][3]<<0;	//设置I2SDIV
	tempreg|=I2S_PSC_TBL[i][4]<<8;	//设置ODD位
	tempreg|=1<<9;					//使能MCKOE位,输出MCK
	SPI2->I2SPR=tempreg;			//设置I2SPR寄存器 
	return 0;
}  
//I2S2 TX DMA配置
//设置为双缓冲模式,并开启DMA传输完成中断
//buf0:M0AR地址.
//buf1:M1AR地址.
//num:每次传输数据量
void I2S_TX_DMA_Init(uint8_t* buf0,uint8_t *buf1,uint16_t num)
{  
	RCC->AHB1ENR|=1<<21;		//DMA1时钟使能   
	while(DMA1_Stream4->CR&0X01);//等待DMA1_Stream4可配置 
	DMA1->HIFCR|=0X3D<<6*0;		//清空通道4上所有中断标志
	DMA1_Stream4->FCR=0X0000021;//设置为默认值	
	
	DMA1_Stream4->PAR=(uint32_t)&SPI2->DR;		//外设地址为:SPI2->DR
	DMA1_Stream4->M0AR=(uint32_t)buf0;//暂不设置
	DMA1_Stream4->M1AR=(uint32_t)buf1;//暂不设置
	DMA1_Stream4->NDTR=num;		//暂时设置长度为1
	DMA1_Stream4->CR=0;			//先全部复位CR寄存器值  
	DMA1_Stream4->CR|=1<<6;		//存储器到外设模式 
	DMA1_Stream4->CR|=1<<8;		//循环模式
	DMA1_Stream4->CR|=0<<9;		//外设非增量模式
	DMA1_Stream4->CR|=1<<10;	//存储器增量模式
	DMA1_Stream4->CR|=1<<11;	//外设数据长度:16位
	DMA1_Stream4->CR|=1<<13;	//存储器数据长度:16位
	DMA1_Stream4->CR|=2<<16;	//高优先级
	DMA1_Stream4->CR|=1<<18;	//双缓冲模式
	DMA1_Stream4->CR|=0<<21;	//外设突发单次传输
	DMA1_Stream4->CR|=0<<23;	//存储器突发单次传输
	DMA1_Stream4->CR|=0<<25;	//选择通道0 SPI2_TX通道 

	DMA1_Stream4->FCR&=~(1<<2);	//不使用FIFO模式
	DMA1_Stream4->FCR&=~(3<<0);	//无FIFO 设置
	
	DMA1_Stream4->CR|=1<<4;		//开启传输完成中断
	nvic_init(0,0,DMA1_Stream4_IRQn,2);	//抢占1，子优先级0，组2  
} 

extern void audio_i2s_dma_callback(void);

//DMA1_Stream4中断服务函数
void DMA1_Stream4_IRQHandler(void)
{      
	if(DMA1->HISR&(1<<5))	//DMA1_Steam4,传输完成标志
	{ 
		DMA1->HIFCR|=1<<5;	//清除传输完成中断
      	audio_i2s_dma_callback();	//执行回调函数,读取数据等操作在这里面处理  
	}   											 
}
#elif defined(I2S3)
//I2S3初始化
//std:I2S标准,00,飞利浦标准;01,MSB对齐标准(右对齐);10,LSB对齐标准(左对齐);11,PCM标准
//mode:I2S工作模式,00,从机发送;01,从机接收;10,主机发送;11,主机接收
//cpol:0,时钟低电平有效;1,时钟高电平有效
//datalen:数据长度,0,16位标准;1,16位扩展(frame=32bit);2,24位;3,32位.
void I2S_Init(unsigned char std,unsigned char mode,unsigned char cpol,unsigned char datalen)
{ 
	RCC->APB1ENR|=1<<15;	//使能SPI3时钟
	RCC->APB1RSTR|=1<<15;	//复位SPI3
	RCC->APB1RSTR&=~(1<<15);//结束复位  
	SPI3->I2SCFGR=0;		//全部设置为0
	SPI3->I2SPR=0X02;		//分频寄存器为默认值
	SPI3->I2SCFGR|=1<<11;	//选择:I2S模式
	SPI3->I2SCFGR|=mode<<8;//I2S工作模式设置
	SPI3->I2SCFGR|=std<<4;		//I2S标准设置 
	SPI3->I2SCFGR|=cpol<<3;		//空闲时钟电平设置
	if(datalen)					//非标准16位长度
	{
		SPI3->I2SCFGR|=1<<0;	//Channel长度为32位 
		datalen-=1;		
	}else SPI3->I2SCFGR|=0<<0;	//Channel长度为16位  
	SPI3->I2SCFGR|=datalen<<1;	//I2S标准设置 
	SPI3->CR2|=1<<1;			//SPI3 TX DMA请求使能.
	SPI3->I2SCFGR|=1<<10;		//SPI3 I2S EN使能.	
}

//设置IIS的采样率(@MCKEN)
//samplerate:采样率,单位:Hz
//返回值:0,设置成功;1,无法设置.
unsigned char I2S_SampleRate_Set(unsigned int samplerate)
{ 
	unsigned char i=0; 
	unsigned int tempreg=0;
	samplerate/=10;//缩小10倍   
	for(i=0;i<(sizeof(I2S_PSC_TBL)/10);i++)//看看改采样率是否可以支持
	{
		if(samplerate==I2S_PSC_TBL[i][0])break;                                                                                                          
	}
	RCC->CR&=~(1<<26);						//先关闭PLLI2S
	if(i==(sizeof(I2S_PSC_TBL)/10))return 1;//搜遍了也找不到
	tempreg|=(unsigned int)I2S_PSC_TBL[i][1]<<6;		//设置PLLI2SN
	tempreg|=(unsigned int)I2S_PSC_TBL[i][2]<<28;	//设置PLLI2SR
	tempreg|=12<<0;////设置PLLI2SM
	RCC->PLLI2SCFGR=tempreg;		//设置I2SxCLK的频率(x=2) 
	RCC->CR|=1<<26;					//开启I2S时钟
	while((RCC->CR&1<<27)==0);		//等待I2S时钟开启成功. 
	tempreg=I2S_PSC_TBL[i][3]<<0;	//设置I2SDIV
	tempreg|=I2S_PSC_TBL[i][4]<<8;	//设置ODD位
	tempreg|=1<<9;					//使能MCKOE位,输出MCK
	SPI3->I2SPR=tempreg;			//设置I2SPR寄存器 
	return 0;
}  

//I2S3 TX DMA配置
//设置为双缓冲模式,并开启DMA传输完成中断
//buf0:M0AR地址.
//buf1:M1AR地址.
//num:每次传输数据量
void I2S_TX_DMA_Init(unsigned char* buf0,unsigned char *buf1,unsigned short num)
{  
	RCC->AHB1ENR|=1<<21;		//DMA1时钟使能   
	while(DMA1_Stream5->CR&0X01);//等待DMA1_Stream5可配置 
	DMA1->HIFCR|=0X3D<<6;		//清空流5上所有中断标志
	DMA1_Stream5->FCR=0X0000021;//设置为默认值	
	
	DMA1_Stream5->PAR=(unsigned int)&SPI3->DR;		//外设地址为:SPI3->DR
	DMA1_Stream5->M0AR=(unsigned int)buf0;//暂不设置
	DMA1_Stream5->M1AR=(unsigned int)buf1;//暂不设置
	DMA1_Stream5->NDTR=num;		//暂时设置长度为1
	DMA1_Stream5->CR=0;			//先全部复位CR寄存器值  
	DMA1_Stream5->CR|=1<<6;		//存储器到外设模式 
	DMA1_Stream5->CR|=1<<8;		//循环模式
	DMA1_Stream5->CR|=0<<9;		//外设非增量模式
	DMA1_Stream5->CR|=1<<10;	//存储器增量模式
	DMA1_Stream5->CR|=1<<11;	//外设数据长度:16位
	DMA1_Stream5->CR|=1<<13;	//存储器数据长度:16位
	DMA1_Stream5->CR|=2<<16;	//高优先级
	DMA1_Stream5->CR|=1<<18;	//双缓冲模式
	DMA1_Stream5->CR|=0<<21;	//外设突发单次传输
	DMA1_Stream5->CR|=0<<23;	//存储器突发单次传输
	DMA1_Stream5->CR|=0<<25;	//选择通道0 SPI3_TX通道 

	DMA1_Stream5->FCR&=~(1<<2);	//不使用FIFO模式
	DMA1_Stream5->FCR&=~(3<<0);	//无FIFO 设置
	
	DMA1_Stream5->CR|=1<<4;		//开启传输完成中断
	nvic_init(0,0,DMA1_Stream5_IRQn,2);	//抢占1，子优先级0，组2  
} 

//DMA1_Stream5中断服务函数
void DMA1_Stream5_IRQHandler(void)
{      
	if(DMA1->HISR&(1<<11))	//DMA1_Steam5,传输完成标志
	{ 
		DMA1->HIFCR|=1<<11;	//清除传输完成中断
		audio_i2s_dma_callback();	//执行回调函数,读取数据等操作在这里面处理  
	}   											 
} 
#endif
