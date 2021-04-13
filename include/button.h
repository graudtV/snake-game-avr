/* interraction with buttons
 * BUTTON_PORT, BUTTON_PORTDD, BUTTON_PORTIN must be defined */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "decls.h"

typedef byte_t button_pin_t;

/* pin must be from 0 to 7 */
void button_init_ports(button_pin_t pin)
{
	BIT_CLEAR(BUTTON_PORTDD, pin); // set as input
	BIT_SET(BUTTON_PORT, pin); // enable pull-up resistor
}

/* pin must be from 0 to 7 */
bool_t button_is_pressed(button_pin_t pin)
{
	return !(BUTTON_PORTIN & (1 << pin));
}

#endif // BUTTON_H_