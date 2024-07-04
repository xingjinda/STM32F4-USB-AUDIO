#ifndef __I2S_H
#define __I2S_H
#include "sys.h"    									

#define I2S2

#if defined(I2S2)
void I2S_Init(uint8_t std,uint8_t mode,uint8_t cpol,uint8_t datalen);
uint8_t I2S_SampleRate_Set(uint32_t samplerate);
void I2S_TX_DMA_Init(uint8_t* buf0,uint8_t *buf1,uint16_t num);
#define I2S_Play_Start()	DMA1_Stream4->CR|=1<<0;
#define DMA_Stream			DMA1_Stream4

#elif defined(I2S3)
void I2S_Init(unsigned char std,unsigned char mode,unsigned char cpol,unsigned char datalen); 
unsigned char I2S_SampleRate_Set(unsigned int samplerate);
void I2S_TX_DMA_Init(unsigned char* buf0,unsigned char *buf1,unsigned short num);
#define I2S_Play_Start()    DMA1_Stream5->CR|=1<<0;
#define DMA_Stream			DMA1_Stream5
#endif
#endif
