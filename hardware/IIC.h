#ifndef __IIC_H
#define __IIC_H
#include "stm32f4xx.h" 

#define IIC_CLOCK()     RCC->AHB1ENR|=1<<2;//开启GPIOC时钟

#define	SCL_GPIO	GPIOC
#define	SCL_PIN		4

#define SDA_GPIO	GPIOC
#define SDA_PIN		5


#define SDA_IN()  {SDA_GPIO->MODER&=~(3<<(SDA_PIN*2));SDA_GPIO->MODER|=0<<(SDA_PIN*2);}
#define SDA_OUT() {SDA_GPIO->MODER&=~(3<<(SDA_PIN*2));SDA_GPIO->MODER|=1<<(SDA_PIN*2);}

	 
#define READ_SDA		SDA_GPIO->IDR&1<<SDA_PIN
#define IIC_SCL(x)		if(x)SCL_GPIO->ODR|=(1<<SCL_PIN); else SCL_GPIO->ODR&=~(1<<SCL_PIN);
#define IIC_SDA(x)		if(x)SDA_GPIO->ODR|=(1<<SDA_PIN); else SDA_GPIO->ODR&=~(1<<SDA_PIN);

void IIC_Init(void);		 
void IIC_Start(void);	
void IIC_Stop(void);
void IIC_Send(unsigned char txd);
unsigned char IIC_Read(unsigned char ack);
unsigned char IIC_Wait_Ack(void); 
void IIC_Ack(void);	
void IIC_NAck(void);

#endif
































