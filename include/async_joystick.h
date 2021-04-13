/* Small lib for reading joystick direction asynchronously
 * Before including this file, you need to define some consts and types according to
 *  the following example:
 *  // pins describing joystick connection. Joystick must be connected to PORTA 
 *  // (because only PORTA is connected to ADC)
 *  #define JOYSTICK_VX_PIN 0
 *  #define JOYSTICK_VY_PIN 1
 *
 *  // declare enum joystick_dir_t, which is returned from joystick_get_dir() with
 *  // values which are most convinient for you
 *  typedef enum {
 *  	JOYSTICK_UNKNOWN	= 0,
 *  	JOYSTICK_LEFT		= -1,
 *  	JOYSTICK_RIGHT		= 1,
 *  	JOYSTICK_UP		 	= 2,
 *  	JOYSTICK_DOWN		= -2
 *  } joystick_dir_t;
 *
 *  #include "async_joystick.h"
 *
 * Macro JOYSTICK_CUTOFF may be defined to adjust range of joystick
 *  values around central position which are interpreted as JOYSTICK_UNKNOWN.
 *  JOYSTICK_CUTOFF must be between 0 and 51. Default value is 10. 
 *
 * Note. Before using any library functions, call joystick_init_ports() or init
 * necessary pins as inputs manually (if they were set as outputs some time before)
 */

#ifndef ASYNC_JOYSTICK_H_
#define ASYNC_JOYSTICK_H_

#include "decls.h"

typedef void (*PFN_joystick_callback)(joystick_dir_t dir);

static volatile int8_t _joystick_prev_x, _joystick_prev_y;
static volatile enum { eJoystickCurrentPinVX, eJoystickCurrentPinVY } _joystick_current_pin;
static volatile PFN_joystick_callback _joystick_callback = NULL;

static joystick_dir_t _async_joystick_pos_to_dir(int8_t x, int8_t y);

void async_joystick_init_ports()
{
	/* set necessary pins as inputs */
	byte_t regval = DDRA;
	BIT_CLEAR(regval, JOYSTICK_VX_PIN);
	BIT_CLEAR(regval, JOYSTICK_VY_PIN);
	DDRA = regval; // write to register
}

/* Starts tracking joystick direction by reading state
 * from ADC repetitively */
void async_joystick_start()
{
	_joystick_prev_x = _joystick_prev_y = 0;
	_joystick_current_pin = eJoystickCurrentPinVX;

	ADMUX = JOYSTICK_VX_PIN;
	/* enable adc, start conversion, auto trigger disabled, clear interrupt flag,
	 * interrupts enabled, frequency division = 128 */
	ADCSRA = 0b11011111;
}

void async_joystick_stop()
{
	ADCSRA &= ~(1 << ADEN | 1 << ADIE);
}

/*  Nonblock, acquires last direction, recieved from joystick.
 *  Joystick should be started with async_joystick_init_ports() and
 * async_joystick_start() beforehand */
joystick_dir_t async_joystick_getdir()
{
	return _async_joystick_pos_to_dir(_joystick_prev_x, _joystick_prev_y);
}

/* First callback is invoked when direction will be not equal to JOYSTICK_UNKNOWN
 * Callbacks are recieved only when async_joystick_start() was called (and
 * global interrupts are enabled)*/
void async_joystick_start_notify(PFN_joystick_callback callback)
	{ _joystick_callback = callback; }

/* Stops callbacks, but joystick continues running */
void async_joystick_stop_notify()
	{ _joystick_callback = NULL; }

/* Interrupt handler for updating joystick state */
ISR(ADC_vect)
{
	static volatile int8_t joystick_new_x;
	int8_t adc_val = ((int16_t) ADC) / 10 - 51;
	
	if (_joystick_current_pin == eJoystickCurrentPinVX) {
		joystick_new_x = adc_val;
		_joystick_current_pin = eJoystickCurrentPinVY;
		ADMUX = JOYSTICK_VY_PIN;
	}
	else {
		int8_t joystick_new_y = adc_val;
		joystick_dir_t prev_dir = async_joystick_getdir();
		joystick_dir_t new_dir = _async_joystick_pos_to_dir(joystick_new_x, joystick_new_y);
		bool_t invoke_callback = _joystick_callback && (prev_dir != new_dir);

		_joystick_prev_x = joystick_new_x;
		_joystick_prev_y = joystick_new_y;
		_joystick_current_pin = eJoystickCurrentPinVX;

		if (invoke_callback)
			_joystick_callback(new_dir);

		ADMUX = JOYSTICK_VX_PIN;
	}

	/* enable adc, start conversion, auto trigger disabled, clear interrupt flag,
	 * interrupts enabled, frequency division = 128 */
	ADCSRA = 0b11011111;
}

/* range which is interpreted as UNKNOWN. 7 - 15 seems optimal */
#ifndef JOYSTICK_CUTOFF
#define JOYSTICK_CUTOFF 10
#endif // JOYSTICK_CUTOFF

joystick_dir_t _async_joystick_pos_to_dir(int8_t x, int8_t y)
{
	if (ABS(x) < JOYSTICK_CUTOFF && ABS(y) < JOYSTICK_CUTOFF)
		return JOYSTICK_UNKNOWN;
	if (x > 0 && ABS(y) < x)
		return JOYSTICK_RIGHT;
	if (x < 0 && ABS(y) < -x)
		return JOYSTICK_LEFT;
	if (y > 0)
		return JOYSTICK_UP;
	return JOYSTICK_DOWN;
}

#endif // ASYNC_JOYSTICK_H_