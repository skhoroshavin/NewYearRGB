
#include "hal.h"

enum
{
	red,
	green,
	blue,

	soft_pwm_count
};
volatile uint8_t pwm[soft_pwm_count];

void compare_irq()
{
	dbg_write( 1 );

	uint8_t state = timer_counter();
	led_red_write  ( state < pwm[red]   );
	led_green_write( state < pwm[green] );
	led_blue_write ( state < pwm[blue]  );
	compare_set( state );

	dbg_write( 0 );
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
		pwm[red]   = gamma(255 - state);
		pwm[green] = gamma(state);
		pwm[blue]  = 0;
		return;
	}
	state -= 256;

	if( state < 256 )
	{
		pwm[red]   = 0;
		pwm[green] = gamma(255 - state);
		pwm[blue]  = gamma(state);
		return;
	}
	state -= 256;

	if( state < 256 )
	{
		pwm[red]   = gamma(state);
		pwm[green] = 0;
		pwm[blue]  = gamma(255 - state);
		return;
	}
	state -= 256;
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
