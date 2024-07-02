#ifndef __IIC_H
#define __IIC_H

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  

#define EE_TYPE AT24C04

#define IIC_CLOCK()		RCC->AHB1ENR|=1<<2;//开启GPIOC时钟

#define	SCL_GPIO	GPIOC
#define	SCL_PIN		4

#define SDA_GPIO	GPIOC
#define SDA_PIN		5


#define SDA_IN()  {SDA_GPIO->MODER&=~(3<<(SDA_PIN*2));SDA_GPIO->MODER|=0<<(SDA_PIN*2);}
#define SDA_OUT() {SDA_GPIO->MODER&=~(3<<(SDA_PIN*2));SDA_GPIO->MODER|=1<<(SDA_PIN*2);}

	 
#define READ_SDA   SDA_GPIO->IDR&1<<SDA_PIN 



void IIC_Init(void);		 
void IIC_Start(void);	
void IIC_Stop(void);
void IIC_Send(unsigned char txd);
unsigned char IIC_Read(unsigned char ack);
unsigned char IIC_Wait_Ack(void); 
void IIC_Ack(void);	
void IIC_NAck(void);

unsigned char IIC_ByteRead(unsigned char DeviceAddr,unsigned char DataAddr);
void IIC_ByteWrite(unsigned char DeviceAddr,unsigned char DataAddr,unsigned char Data);
unsigned char AT24CXX_Check(void);
#endif
































