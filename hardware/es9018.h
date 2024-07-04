#ifndef __ES9018_H
#define __ES9018_H
#include "stm32f4xx.h"

#define RCC_MCO_1

#define ES9018_ADDR				0X90	//ES9018的器件地址
// #define ES9018_ADDR				0X92	//ES9018的器件地址

#define ES9018_GPIO		GPIOB
#define ES9018_PIN		14

#define ES9018_ON()				do{ES9018_GPIO->ODR|=1<<ES9018_PIN;}while(0)
#define ES9018_OFF()			do{ES9018_GPIO->ODR&=~(1<<ES9018_PIN);}while(0)


unsigned char ES9018_Init(void);
void ES9018_SetVol(unsigned char index);
void ES9018_SetMute(unsigned char mute);
#endif
