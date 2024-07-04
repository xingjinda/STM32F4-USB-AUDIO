#include "iic.h"
#include "delay.h"

void IIC_Init(void)
{			
	IIC_CLOCK();

	SCL_GPIO->MODER&=~((unsigned int)3<<(2*SCL_PIN));
	SCL_GPIO->MODER|=(unsigned int)1<<(2*SCL_PIN);
	SCL_GPIO->OTYPER&=~(1<<SCL_PIN);
	SCL_GPIO->OTYPER|=0<<SCL_PIN;
	SCL_GPIO->OSPEEDR&=~((unsigned int)3<<(2*SCL_PIN));
	SCL_GPIO->OSPEEDR|=(unsigned int)2<<(2*SCL_PIN);
	SCL_GPIO->PUPDR&=~((unsigned int)3<<(2*SCL_PIN));
	SCL_GPIO->PUPDR|=(unsigned int)2<<(2*SCL_PIN);

	SDA_GPIO->MODER&=~((unsigned int)3<<(2*SDA_PIN));
	SDA_GPIO->MODER|=(unsigned int)1<<(2*SDA_PIN);
	SDA_GPIO->OTYPER&=~(1<<SDA_PIN);
	SDA_GPIO->OTYPER|=0<<SDA_PIN;
	SDA_GPIO->OSPEEDR&=~((unsigned int)3<<(2*SDA_PIN));
	SDA_GPIO->OSPEEDR|=(unsigned int)2<<(2*SDA_PIN);
	SDA_GPIO->PUPDR&=~((unsigned int)3<<(2*SDA_PIN));
	SDA_GPIO->PUPDR|=(unsigned int)2<<(2*SDA_PIN);

	IIC_SCL(1);
	IIC_SDA(1);
}

void IIC_Start(void)
{
	SDA_OUT();
	IIC_SDA(1);	  	  
	IIC_SCL(1);
	delay_us(4);
 	IIC_SDA(0);
	delay_us(4);
	IIC_SCL(0); 
}	  

void IIC_Stop(void)
{
	SDA_OUT();
	IIC_SCL(0);
	IIC_SDA(0);
 	delay_us(4);
	IIC_SCL(1); 
	IIC_SDA(1);
	delay_us(4);							   	
}

unsigned char IIC_Wait_Ack(void)
{
	unsigned char time=0;
	SDA_IN();   
	IIC_SDA(1);
	delay_us(1);	   
	IIC_SCL(1);
	delay_us(1);	 
	while(READ_SDA)
	{
		time++;
		if(time>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL(0);	   
	return 0;  
} 

void IIC_Ack(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(0);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}	    
void IIC_NAck(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(1);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}					 				     

void IIC_Send(unsigned char txd)
{                        
    unsigned char t;   
	SDA_OUT(); 	    
    IIC_SCL(0);
    for(t=0;t<8;t++)
    {              
        IIC_SDA((txd&0x80)>>7);
        txd<<=1; 	  
		delay_us(2);
		IIC_SCL(1);
		delay_us(2); 
		IIC_SCL(0);	
		delay_us(2);
    }	 
} 	    
 
unsigned char IIC_Read(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();
    for(i=0;i<8;i++ )
	{
        IIC_SCL(0); 
        delay_us(2);
		IIC_SCL(1);
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();
    else
        IIC_Ack();  
    return receive;
}	    
