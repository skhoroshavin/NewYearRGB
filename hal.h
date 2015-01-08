
#pragma once

/***********************************************************************\
 * System configuration                                                *
\***********************************************************************/

enum
{
	cpu_freq = 8000000
};

/***********************************************************************\
 * GPIO                                                                *
\***********************************************************************/

#include <drivers/avr/avr_gpio.h>
AVR_GPIO_OUTPUT(led_red,   B,2,1, 0)
AVR_GPIO_OUTPUT(led_green, B,1,1, 0)
AVR_GPIO_OUTPUT(led_blue,  B,0,1, 0)

AVR_GPIO_OUTPUT(dbg,       B,3,1, 0)

/***********************************************************************\
 * Timers                                                              *
\***********************************************************************/

#include <drivers/avr/avr_timer.h>
AVR_BASIC_TIMER(timer, 64)
AVR_TIMER_COMPARE(compare, uint8_t, 0, A)

#include <drivers/soft/soft_timer.h>
SOFT_TIMER(soft_timer, timer, uint32_t, 1000, (uint32_t)3*256*256)

/***********************************************************************\
 * Functions                                                           *
\***********************************************************************/

void hal_init();
void hal_process();
