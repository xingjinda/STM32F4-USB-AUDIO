#include "stm32f4xx.h"
#include "GPIO.h"
#include "delay.h"

void gpio_init()
{
	RCC->AHB1ENR |=1<<0;	//开启GPIOA时钟
	RCC->AHB1ENR |=1<<1;	//开启GPIOB时钟
	RCC->AHB1ENR |=1<<2;	//开启GPIOC时钟
	//RCC->AHB1ENR |=1<<3;	//开启GPIOD时钟
	
	VOL_UP_GPIO->MODER&=~((unsigned int)3<<(2*VOL_UP_PIN));
	VOL_UP_GPIO->MODER|=(unsigned int)0<<(2*VOL_UP_PIN);
	VOL_UP_GPIO->OSPEEDR&=~((unsigned int)3<<(2*VOL_UP_PIN));
	VOL_UP_GPIO->OSPEEDR|=(unsigned int)2<<(2*VOL_UP_PIN);
	VOL_UP_GPIO->PUPDR&=~((unsigned int)3<<(2*VOL_UP_PIN));
	VOL_UP_GPIO->PUPDR|=(unsigned int)1<<(2*VOL_UP_PIN);

	VOL_DOWN_GPIO->MODER&=~((unsigned int)3<<(2*VOL_DOWN_PIN));
	VOL_DOWN_GPIO->MODER|=(unsigned int)0<<(2*VOL_DOWN_PIN);
	VOL_DOWN_GPIO->OSPEEDR&=~((unsigned int)3<<(2*VOL_DOWN_PIN));
	VOL_DOWN_GPIO->OSPEEDR|=(unsigned int)2<<(2*VOL_DOWN_PIN);
	VOL_DOWN_GPIO->PUPDR&=~((unsigned int)3<<(2*VOL_DOWN_PIN));
	VOL_DOWN_GPIO->PUPDR|=(unsigned int)1<<(2*VOL_DOWN_PIN);	
}
