
#include "hal.h"

volatile uint8_t red   = 0;
volatile uint8_t green = 0;
volatile uint8_t blue  = 0;

void compare_irq()
{
	uint8_t state = timer_counter();
	led_red_write  ( state < red   );
	led_green_write( state < green );
	led_blue_write ( state < blue  );
	compare_set( state );
}

uint8_t gamma( uint8_t value )
{
	return value;//(uint16_t)value*value/256;
}

void update_levels()
{
	uint16_t state = soft_timer_counter() / 256;

	if( state < 256 )
	{
		red   = gamma(255 - state);
		green = gamma(state);
		blue  = 0;
		return;
	}

	state -= 256;
	if( state < 256 )
	{
		red   = 0;
		green = gamma(255 - state);
		blue  = gamma(state);
		return;
	}

	state -= 256;
	if( state < 256 )
	{
		red   = gamma(state);
		green = 0;
		blue  = gamma(255 - state);
		return;
	}
}

int main(void)
{
	hal_init();

	compare_set( 0 );
	compare_irq_enable();
	timer_start();
	irq_enable();

	while(1)
	{
		update_levels();
		hal_process();
	}

	return 0;
}
