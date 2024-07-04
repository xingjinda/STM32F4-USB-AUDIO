#ifndef __GPIO_H__
#define __GPIO_H__

void	gpio_init(void);


#define VOL_UP_GPIO		GPIOB
#define VOL_UP_PIN		7

#define VOL_DOWN_GPIO	GPIOB
#define VOL_DOWN_PIN	8


#define	VOL_UP					!(VOL_UP_GPIO->IDR&1<<VOL_UP_PIN)
#define VOL_DOWN				!(VOL_DOWN_GPIO->IDR&1<<VOL_DOWN_PIN)


#endif
