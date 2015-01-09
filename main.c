
#include "hal.h"

enum
{
	blue,
	green,
	red,

	soft_pwm_count
};

uint8_t soft_pwm[soft_pwm_count] = { 8, 0, 0 };

struct pwm_state_t
{
	uint8_t mask;
	uint8_t delay;
};
struct pwm_state_t soft_pwm_fsm[soft_pwm_count];
uint8_t pwm_state;

void pwm_apply()
{
	uint8_t i, ch, prev_delay;
	uint8_t mask  = MASK(soft_pwm_count);
	uint8_t delay = 0;

	for( i = 0; i < soft_pwm_count; ++i )
	{
		if( !mask )
		{
			soft_pwm_fsm[i].mask  = 0;
			soft_pwm_fsm[i].delay = 0;
			continue;
		}

		prev_delay = delay;
		delay = -1;
		for( ch = 0; ch < soft_pwm_count; ++ch )
		{
			uint8_t value = soft_pwm[ch];
			if( value == prev_delay )
				mask &= (uint8_t)~(1 << ch);
			if( (value > prev_delay) && (value < delay) )
				delay = value;
		}
		soft_pwm_fsm[i].mask  = mask;
		soft_pwm_fsm[i].delay = mask ? delay : 0;
	}
}

void timer_overflow_irq()
{
	compare_irq_clear();

	uint8_t delay = soft_pwm_fsm[0].delay;
	led_rgb_write( soft_pwm_fsm[0].mask );

	if( delay )
	{
		if( timer_counter() >= delay )
			delay = timer_counter() + 1;

		compare_set( delay );
		pwm_state = 1;
		compare_irq_enable();
	}
	else
	{
		compare_irq_disable();
	}
}

void compare_irq()
{
	uint8_t delay = soft_pwm_fsm[pwm_state].delay;
	led_rgb_write( soft_pwm_fsm[pwm_state].mask );

	if( delay )
	{
		if( timer_counter() >= delay )
			delay = timer_counter() + 1;

		compare_set( delay );
		++pwm_state;
	}
	else
	{
		compare_irq_disable();
		compare_irq_clear();
	}
}

void update_levels()
{
	static uint16_t last_state = 0;

	uint8_t dec_i, inc_i, zer_i;
	uint16_t state = soft_timer_counter() / 64;
	if( state == last_state ) return;
	last_state = state;

	switch( state >> 8 )
	{
	case 0: dec_i = red;   inc_i = green; zer_i = blue;  break;
	case 1: dec_i = green; inc_i = blue;  zer_i = red;   break;
	case 2: dec_i = blue;  inc_i = red;   zer_i = green; break;
	default: dec_i = inc_i = zer_i = 0; dbg_write(1);
	}

	soft_pwm[dec_i] = 0xFF - (state & 0xFF);
	soft_pwm[inc_i] = state & 0xFF;
	soft_pwm[zer_i] = 0;
	pwm_apply();
}

int main(void)
{
	hal_init();

	pwm_apply();

	compare_set( 0 );
	timer_overflow_irq_enable();
	timer_start();
	irq_enable();

	while(1)
	{
		update_levels();
		hal_process();
	}

	return 0;
}
