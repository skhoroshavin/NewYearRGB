
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
volatile struct pwm_state_t soft_pwm_fsm[soft_pwm_count];

#define find_min_value_index(data, size, value, index) \
	do { \
		uint8_t i; \
		value = data[0]; \
		index = 0; \
		for( i = 0; i < size; ++i ) \
		{ \
			if( data[i] < value ) \
			{ \
				value = data[i]; \
				index = i; \
			} \
		} \
	} while(0)

#define find_next_value_index(data, size, value, index) \
	do { \
		uint8_t i; \
		for( i = index+1; i < size; ++i ) \
		{ \
			if( data[i] == value ) \
			{ \
				index = i; \
				break; \
			} \
		} \
		index = i; \
	} while(0)

#define find_next_min_value_index(data, size, value, index, prev_min) \
	do { \
		uint8_t i; \
		for( i = 0; i < size; ++i ) \
		{ \
			if( data[i] > prev_min ) \
			{ \
				value = data[i]; \
				index = i; \
				break; \
			} \
		} \
		for( i = index+1; i < size; ++i ) \
		{ \
			if( (data[i] > prev_min) && (data[i] < value) ) \
			{ \
				value = data[i]; \
				index = i; \
			} \
		} \
	} while(0)

void pwm_apply()
{
	uint8_t mask = 0xff;

	uint8_t delay, channel, i;
	find_min_value_index( soft_pwm, soft_pwm_count, delay, channel );

	irq_disable();
	soft_pwm_fsm[0].mask = mask;
	soft_pwm_fsm[0].delay = delay;
	irq_enable();

	for( i = 1; i < soft_pwm_count; ++i )
	{
		mask &= ~(uint8_t)(1 << channel);

		find_next_value_index( soft_pwm, soft_pwm_count, delay, channel );
		if( channel == soft_pwm_count )
		{
			uint8_t min_delay = delay;
			find_next_min_value_index( soft_pwm, soft_pwm_count, delay, channel, min_delay );
		}

		irq_disable();
		soft_pwm_fsm[i].mask = mask;
		soft_pwm_fsm[i].delay = delay;
		irq_enable();
	}
}

void pwm_update()
{
	static uint8_t state = 0;

	while( soft_pwm_fsm[state].delay <= timer_counter() )
	{
		++state;
		if( state == soft_pwm_count )
		{
			state = 0;
			led_rgb_write( 0 );
			if( timer_counter() < 255 )
			{
				compare_set( -1 );
				return;
			}
		}
	}

	led_rgb_write( soft_pwm_fsm[state].mask );
	compare_set( soft_pwm_fsm[state].delay-1 );
}

void compare_irq()
{
	pwm_update();
}

void update_levels()
{
	static uint16_t last_state = 0;

	uint8_t dec_i, inc_i, zer_i;
	uint16_t state = soft_timer_counter() / 256;
	if( state == last_state ) return;
	last_state = state;

	switch( state >> 8 )
	{
	case 0: dec_i = red;   inc_i = green; zer_i = blue;  break;
	case 1: dec_i = green; inc_i = blue;  zer_i = red;   break;
	case 2: dec_i = blue;  inc_i = red;   zer_i = green; break;
	default: dec_i = inc_i = zer_i = 0; dbg_write(1);
	}

	soft_pwm[dec_i] = 251;//0xFF - (state & 0xFF);
	soft_pwm[inc_i] = 0;//state & 0xFF;
	soft_pwm[zer_i] = 0;
	pwm_apply();
}

int main(void)
{
	hal_init();

	pwm_apply();

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
