/* Small library for using max7219 led driver on atmega8535
 * max7219 is accessed via SPI. Connection is the following:
 *  MAX7219 DIN (pin 1)		--> avr SPI MOSI
 *  MAX7219 LOAD (pin 12)	-->	avr SPI SS
 *  MAX7219 CLK (pin 13)	--> avr SPI SCK
 * SS is used not as a slave selector, but as a pin to write to max7219 LOAD
 */

#ifndef MAX7219_H_
#define MAX7219_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "decls.h"

/* SPI pins are given for atmega8535 and may differ on other avrs */
#define SPI_PORT	PORTB
#define SPI_PORTDD	DDRB
#define SPI_SCK		PORTB7
#define SPI_MOSI	PORTB5
#define SPI_SS		PORTB4

#define MAX7219_PORT		SPI_PORT
#define MAX7219_PORTDD		SPI_PORTDD
#define MAX7219_LOAD_PIN	SPI_SS

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
	SPDR = byte;
	while (!(SPSR & (1 << SPIF)))
		;
	SPSR &= ~(1 << SPIF);
}

/* sends 16-bit packet to max7219 */
void _max7219_send_packet(byte_t register_addr, byte_t data)
{
	MAX7219_PORT &= ~(1 << MAX7219_LOAD_PIN); // set LOAD bit = 0
	_max7219_send_byte(register_addr);
	_max7219_send_byte(data);
	MAX7219_PORT |= (1 << MAX7219_LOAD_PIN); // set LOAD bit = 1
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
	/* set pins as outputs */
	MAX7219_PORTDD |= (1 << SPI_SCK) | (1 << SPI_MOSI) | (1 << MAX7219_LOAD_PIN);

	/* no interrupts, enable SPI, MSB first, master, cpol=0, cpha=0, freq div = CK/4 */
	SPCR = 0b01010000;
}

#endif // MAX7219_H_