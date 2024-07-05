#include "sys.h"



void set_vector(uint32_t baseaddr, uint32_t offset)
{
	SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00);
}



void nvic_config(uint8_t group)
{
	uint32_t temp, temp1;
	temp1 = (~group) & 0x07;
	temp1 <<= 8;
	temp = SCB->AIRCR;
	temp &= 0X0000F8FF;
	temp |= 0X05FA0000;
	temp |= temp1;
	SCB->AIRCR = temp;
}

void nvic_init(uint8_t pprio, uint8_t sprio, uint8_t ch, uint8_t group)
{
	uint32_t temp;
	nvic_config(group);
	temp = pprio << (4 - group);
	temp |= sprio & (0x0f >> group);
	temp &= 0xf;
	NVIC->ISER[ch / 32] |= 1 << (ch % 32);
	NVIC->IP[ch] |= temp << 4;
}


void wfi_set(void)
{
	__ASM volatile("wfi");
}

void intx_disable(void)
{
	__ASM volatile("cpsid i");
}

void intx_enable(void)
{
	__ASM volatile("cpsie i");
}

void msr_msp(uint32_t addr)
{
	__set_MSP(addr);
}

void standby(void)
{
	RCC->APB1ENR |= 1 << 28;
	PWR->CSR |= 1 << 8;
	PWR->CR |= 1 << 2;
	PWR->CR |= 1 << 1;
	SCB->SCR |= 1 << 2;
	wfi_set();
}

void soft_reset(void)
{
	SCB->AIRCR = 0X05FA0000 | (uint32_t)0x04;
}

void stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
	SCB->CPACR |= ((3<<10*2)|(3<<11*2));//开启浮点运算
	RCC->CR = 0x00000001;		//复位CR      
	RCC->CFGR = 0x00000000;		//复位CFGR
	RCC->PLLCFGR = 0x24003010;  //复位PLLCFGR
	RCC->CIR = 0x00000000;		//复位CIR

	RCC->CR |=  1<<16;

	while ((RCC->CR & 1<<17) == 0); 	
	
	RCC->APB1ENR |= 1<<28;
	PWR->CR |= 3<<14;

	RCC->CFGR |=(unsigned int) 0<<4;	//AHB 预分频器
	RCC->CFGR |=(unsigned int) 4<<10;	//APB1 预分频器
	RCC->CFGR |=(unsigned int) 0<<13;	//APB2 预分频器
	
	RCC->CFGR |=(unsigned int) 2<<30;	//MCO2	HSE时钟
	RCC->CFGR |=(unsigned int) 2<<21;	//MCO1	HSE时钟

	RCC->PLLCFGR = pllm | (plln << 6) | (((pllp >> 1) - 1) << 16) |(1<<22) | (pllq << 24);

	RCC->CR |= 1<<24;
	while ((RCC->CR & 1<<25) == 0);
	FLASH->ACR =1<<8|1<<9|1<<10|2<<0;
	RCC->CFGR &= 0xFFFFFFFC;
	RCC->CFGR |= 1<<1;
	while ((RCC->CFGR & 3<<2) != 2<<2);

	#ifdef VECT_TAB_SRAM
	SCB->VTOR = 0x20000000;
	#else
	SCB->VTOR = 0x08000000;
	#endif
}
