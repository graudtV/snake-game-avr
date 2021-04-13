/* Small library for using max7219 led driver on avr microcontrollers without use of SPI
 * You need to define the following macro before including file:
 *  MAX7219_PORT            -- port, to which max7219 is connected
 *  MAX7219_PORTDD          -- port for switching MAX7219_PORT mode (input/output mode)
 *  MAX7219_CLK_PIN         -- CLK pin index
 *  MAX7219_DATA_IN_PIN     -- DATA_IN pin index
 *  MAX7219_LOAD_PIN        -- LOAD pin index
 * Example:
 *  #define MAX7219_PORT PORTA
 *  #define MAX7219_PORTDD DDRA
 *  #define MAX7219_CLK_PIN 0
 *  #define MAX7219_DATA_IN_PIN 1
 *  #define MAX7219_LOAD_PIN 2
 *  #include "max7219.h"
 * Note: before using library, call max7219_init_ports() or set necessary pins as
 *  outputs manually
 * Note: functions are written inside this header for compilation simplicity,
 *  so don't include this file in multiple files in one project, because it
 *  will fail to compile
 */

#ifndef MAX7219_H_
#define MAX7219_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "decls.h"

#define MAX7219_MODE_DECODE			0x09
#define MAX7219_MODE_INTENSITY		0x0A
#define MAX7219_MODE_SCAN_LIMIT		0x0B
#define MAX7219_MODE_SHUTDOWN		0x0C
#define MAX7219_MODE_DISPLAY_TEST	0x0F
#define MAX7219_MODE_NOOP			0x00
#define MAX7219_DIGIT0				0x01

/* shifts in next byte into internal max7219 register */
void _max7219_send_byte(byte_t byte)
{
	byte_t regval = MAX7219_PORT;
	for (int i = 7; i >= 0; --i) {
		BIT_CLEAR(regval, MAX7219_CLK_PIN); // set CLK bit = 0
		MAX7219_PORT = regval; // send to port

		int bit = (byte >> i) & 1u; // data bit to be sent
		BIT_SET_TO(regval, MAX7219_DATA_IN_PIN, bit); // set DATA_IN bit appropriately
		MAX7219_PORT = regval; // send to port

		BIT_SET(regval, MAX7219_CLK_PIN); // set CLK bit = 1
		MAX7219_PORT = regval; // send to port		
	}
}

/* sends 16-bit packet to max7219 */
void _max7219_send_packet(byte_t register_addr, byte_t data)
{
	byte_t regval = MAX7219_PORT;

	BIT_CLEAR(regval, MAX7219_LOAD_PIN); // set LOAD bit = 0
	MAX7219_PORT = regval; // send to port

	_max7219_send_byte(register_addr);
	_max7219_send_byte(data);

	BIT_SET(regval, MAX7219_LOAD_PIN); // set LOAD bit = 1
	MAX7219_PORT = regval; // send to port
}

void max7219_enable_display_test(bool_t enable)
{
	byte_t data = !!enable;
	/* data = 0b00000001 -> test on, data = 0b00000000 -> test off */
	_max7219_send_packet(MAX7219_MODE_DISPLAY_TEST, data);
}

void max7219_enable_shutdown(bool_t enable)
{
	byte_t data = !enable;
	_max7219_send_packet(MAX7219_MODE_SHUTDOWN, data);
}

/*  max7219 is in shutdown mode on initial power-up. Use this function
 * to start displaying digits */
void max7219_wakeup() { max7219_enable_shutdown(false); }

/* intensity should be from 0 to 15 (decimal) */
void max7219_set_intencity(byte_t intensity)
{
	_max7219_send_packet(MAX7219_MODE_INTENSITY, intensity);
}

/* sets number of digits to be scanned.
 * ndigits must be from 1 to 8 */
void max7219_set_ndigits(byte_t ndigits)
{
	_max7219_send_packet(MAX7219_MODE_SCAN_LIMIT, ndigits - 1);
}

/* digit must be between 0 and 7, val is from 0 to 255 */
void max7219_setdigit(byte_t digit, byte_t val)
{
	_max7219_send_packet(MAX7219_DIGIT0 + digit, val);
}

/* writes 0 to all digits */
void max7219_clear_digits()
{
	for (int i = 0; i < 8; ++i)
		max7219_setdigit(i, 0);
}

void max7219_init_ports()
{
	byte_t regval;

	/* set all pins as 0 before switching them on */
	regval = MAX7219_PORT;
	BIT_CLEAR(regval, MAX7219_CLK_PIN);
	BIT_CLEAR(regval, MAX7219_DATA_IN_PIN);
	BIT_CLEAR(regval, MAX7219_LOAD_PIN);
	MAX7219_PORT = regval; // write to register

	/* switching pins as outputs */
	regval = MAX7219_PORTDD;
	BIT_SET(regval, MAX7219_CLK_PIN);
	BIT_SET(regval, MAX7219_DATA_IN_PIN);
	BIT_SET(regval, MAX7219_LOAD_PIN);
	MAX7219_PORTDD = regval; // write to register
}

#endif // MAX7219_H_