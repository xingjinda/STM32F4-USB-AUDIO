#ifndef __USART_H
#define __USART_H

#include "sys.h"



#define USART_REC_LEN               200
#define USART_EN_RX                 FALSE


extern unsigned char  usart_rx_buf[USART_REC_LEN];
extern unsigned short usart_rx_sta;

void usart_init(unsigned int sclk, unsigned int baudrate);
void print(char* format, ...); 

#endif  
















