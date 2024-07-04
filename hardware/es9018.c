#include "es9018.h"
#include "delay.h"
#include "IIC.h"
#include "i2s.h"


unsigned char ES9018_ByteRead(unsigned char DataAddr)
{
	unsigned char temp=0;
	IIC_Start();
	IIC_Send(ES9018_ADDR+((DataAddr/256)<<1));   //发送器件地址ES9018_ADDR,写数据
	IIC_Wait_Ack();
	IIC_Send(DataAddr%256);			//发送低地址
	IIC_Wait_Ack();
	IIC_Start();
	IIC_Send(ES9018_ADDR|0x01);		//进入接收模式
	IIC_Wait_Ack();
	temp=IIC_Read(0);
	IIC_Stop();						//产生一个停止条件
	return temp;
}
void ES9018_ByteWrite(unsigned char DataAddr,unsigned char Data)
{
	IIC_Start();
	IIC_Send(ES9018_ADDR+((DataAddr/256)<<1));   //发送器件地址ES9018_ADDR,写数据
	IIC_Wait_Ack();
	IIC_Send(DataAddr%256);   //发送低地址
	IIC_Wait_Ack();
	IIC_Send(Data);     //发送字节
	IIC_Wait_Ack();
	IIC_Stop();//产生一个停止条件
	delay_ms(10);
}

unsigned char ES9018_Init(void)
{
	RCC->AHB1ENR|=1<<0|1<<1|1<<2;
#if defined(RCC_MCO_1)

	GPIOA->AFR[1] &=0xFFFFFFF0;			//复用PA8
	GPIOA->MODER&=0xFFFCFFFF;			//PA8
	GPIOA->MODER|=0x00020000;
	GPIOA->OTYPER&=0x0000FEFF;
	GPIOA->OSPEEDR&=0xFFFCFFFF;
	GPIOA->OSPEEDR|=0x00030000;
	GPIOA->PUPDR&=0xFFFCFFFF;
#elif defined(RCC_MCO_2)
	GPIOC->AFR[1] &=0xFFFFFF0F;			//复用PC9
	GPIOC->MODER&=0xFFF3FFFF;			//PC9
	GPIOC->MODER|=0x00080000;
	GPIOC->OTYPER&=0x0000FDFF;
	GPIOC->OSPEEDR&=0xFFF3FFFF;  
	GPIOC->OSPEEDR|=0x000C0000;
	GPIOC->PUPDR&=0xFFF3FFFF;
#endif

#if defined(I2S2)
	GPIOB->AFR[1] &=0x0F00FFFF;			//复用PB12 PB13 PB15
	GPIOB->AFR[1] |=0x50550000;
	GPIOB->MODER&=0x30FFFFFF;			//PB12 PB13 PB15
	GPIOB->MODER|=0x8A000000;
	GPIOB->OTYPER&=0x00004FFF;
	GPIOB->OSPEEDR&=0x30FFFFFF;
	GPIOB->OSPEEDR|=0xCF000000;
	GPIOB->PUPDR&=0x30FFFFFF;
#elif defined(I2S3)
	// GPIOA->AFR[0] &=0xFFF0FFFF;			//复用PA4
	// GPIOA->AFR[0] |=0x00060000;

	// GPIOB->AFR[0] &=0xFF0F0FFF;			//复用PB3	PB5
	// GPIOB->AFR[0] |=0x00606000;

	// GPIOA->MODER&=0xFFFFFCFF;			//PA4
	// GPIOA->MODER|=0x00000200;
	// GPIOA->OTYPER&=0x0000FFEF;
	// GPIOA->OSPEEDR&=0xFFFFFCFF;
	// GPIOA->OSPEEDR|=0x00000300;
	// GPIOA->PUPDR&=0xFFFFFCFF;

	// GPIOB->MODER&=0xFFFFF33F;			//PB3 PB5
	// GPIOB->MODER|=0x00000880;
	// GPIOB->OTYPER&=0x0000FFD7;
	// GPIOB->OSPEEDR&=0xFFFFF33F;
	// GPIOB->OSPEEDR|=0x00000CC0;
	// GPIOB->PUPDR&=0xFFFFF33F;
#endif

	ES9018_GPIO->MODER&=~((unsigned int)3<<(2*ES9018_PIN));
	ES9018_GPIO->MODER|=(unsigned int)1<<(2*ES9018_PIN);
	ES9018_GPIO->OTYPER&=~(1<<ES9018_PIN);
	ES9018_GPIO->OTYPER|=0<<ES9018_PIN;
	ES9018_GPIO->OSPEEDR&=~((unsigned int)3<<(2*ES9018_PIN));
	ES9018_GPIO->OSPEEDR|=(unsigned int)2<<(2*ES9018_PIN);
	ES9018_GPIO->PUPDR&=~((unsigned int)3<<(2*ES9018_PIN));
	ES9018_GPIO->PUPDR|=(unsigned int)2<<(2*ES9018_PIN);
	
	ES9018_OFF();
	delay_ms(10);
	ES9018_ON();
	delay_ms(10);
	ES9018_ByteWrite(0x01,0x8C);
	delay_ms(10);
	ES9018_ByteWrite(0x0B,0x02);
	ES9018_SetVol(200);

	return (ES9018_ByteRead(0x40)&0x1C)>>2;
}

void ES9018_SetVol(unsigned char index)
{

	unsigned char vol; 

	if ( index >= 2 )
		vol = 95 - 95 * index / 0xFF;
	else
		vol= 0xff;
	ES9018_ByteWrite(0x0F,vol);
	delay_ms(10);
	ES9018_ByteWrite(0x10,vol);
	delay_ms(10);
}

void ES9018_SetMute(unsigned char mute)
{
	if(mute)
	ES9018_ByteWrite(0x07,0x83);
	else
	ES9018_ByteWrite(0x07,0x80);
}
