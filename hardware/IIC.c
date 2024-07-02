#include "iic.h"
#include "stm32f4xx.h"  
#include "delay.h"

void IIC_SCL(unsigned char data)
{
	if(data)
	SCL_GPIO->BSRR|=1<<SCL_PIN;
	else
	SCL_GPIO->BSRR|=(1<<SCL_PIN)<<16;
}

void IIC_SDA(unsigned char data)
{
	if(data)
	SDA_GPIO->BSRR|=1<<SDA_PIN;
	else
	SDA_GPIO->BSRR|=(1<<SDA_PIN)<<16;
}

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


unsigned char IIC_ByteRead(unsigned char DeviceAddr,unsigned char DataAddr)
{
	unsigned char temp=0;
	IIC_Start();
	if(EE_TYPE>AT24C16)
	{
		IIC_Send(DeviceAddr);		//发送写命令
		IIC_Wait_Ack();
		IIC_Send(DataAddr>>8);		//发送高地址
	}else
	IIC_Send(DeviceAddr+((DataAddr/256)<<1));   //发送器件地址DeviceAddr,写数据
	IIC_Wait_Ack();
    IIC_Send(DataAddr%256);			//发送低地址
	IIC_Wait_Ack();
	IIC_Start();
	IIC_Send(DeviceAddr|0x01);		//进入接收模式
	IIC_Wait_Ack();
    temp=IIC_Read(0);
    IIC_Stop();						//产生一个停止条件
	return temp;
}
void IIC_ByteWrite(unsigned char DeviceAddr,unsigned char DataAddr,unsigned char Data)
{
	IIC_Start();
	if(EE_TYPE>AT24C16)
	{
		IIC_Send(DeviceAddr);	    //发送写命令
		IIC_Wait_Ack();
		IIC_Send(DataAddr>>8);//发送高地址
	}else
	IIC_Send(DeviceAddr+((DataAddr/256)<<1));   //发送器件地址DeviceAddr,写数据
	IIC_Wait_Ack();
    IIC_Send(DataAddr%256);   //发送低地址
	IIC_Wait_Ack();
	IIC_Send(Data);     //发送字节
	IIC_Wait_Ack();
    IIC_Stop();//产生一个停止条件
	delay_ms(10);
}
unsigned char AT24CXX_Check(void)
{
	unsigned char temp;
	temp=IIC_ByteRead(0xA0,255);//避免每次开机都写AT24CXX			   
	if(temp==0X55)return 0;		   
	else//排除第一次初始化的情况
	{
		IIC_ByteWrite(0xA0,255,0X55);
	    temp=IIC_ByteRead(0xA0,255);	  
		if(temp==0X55)return 0;
	}
	return 1;											  
}