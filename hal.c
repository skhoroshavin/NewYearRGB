
#include "hal.h"
#include <system/soft_irq.h>

IMPLEMENT_AVR_BASIC_TIMER(timer)
IMPLEMENT_AVR_TIMER_COMPARE(compare, 0, A)

IMPLEMENT_SOFT_TIMER(soft_timer, timer)

void hal_init()
{
	CLKPR = 0x80;
	CLKPR = 0x00;

	led_rgb_init();
	dbg_init();

	soft_timer_init();
}

void hal_process()
{
	soft_timer_process();
}
