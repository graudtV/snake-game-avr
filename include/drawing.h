/*  Provides some bit-images for drawing on a 8x8 led matrix,
 * including digits and letters
 *  Pre-defined pictures are stored in program memory.
 * Use syntax like image_show(image_progread(prog_img_smile)) to show them
 *  By default pre-defined images are disabled to minimize flashing time,
 * define macros to enable them:
 * #define DRAWING_USING_COMMON_IMAGES // -> smiles, arrows, etc
 * #define DRAWING_USING_NUMBERS // -> digits
 * #define DRAWING_USING_LETTERS // -> english alphabet letters
 *
 * Author: Graudt V.
 **/

#ifndef DRAWING_H_
#define DRAWING_H_

#include "max7219.h"
#include <avr/pgmspace.h>

/*  Constants are not supposed to be changed. This file is developped
 * only for 8x8 led matrices, driven by max7219 */
#define MAX_IMAGE_HEIGHT 8
#define MAX_IMAGE_WIDTH 8
#define MAX_LETTER_HEIGHT 5
#define MAX_LETTER_WIDTH 8

typedef uint8_t image_t[MAX_IMAGE_HEIGHT];
typedef const uint8_t cimage_t[MAX_IMAGE_HEIGHT];
typedef uint8_t *image_ref_t;

/* letter is a small image, used for better space efficiency */
typedef uint8_t letter_t[MAX_LETTER_HEIGHT];
typedef const uint8_t cletter_t[MAX_LETTER_HEIGHT];
typedef uint8_t *letter_ref_t;

/* text is array of letters */
typedef letter_t text_t[];
typedef const letter_t ctext_t[];
typedef letter_t *text_ref_t;

static image_t _image_buffer;
static letter_t _letter_buffer;

/*  Note. Rows in image correspond to digits in max7219, columns - to segments.
 * (0, 0) in image is top right (!) corner on matrix. In such agreement
 * binary numbers in image will be seen non-inverted, for example,
 * if image[0] = 0b00000010, a led near the right top corner will be glowing */
void image_show_max7219(cimage_t image)
{
	for (int i = 0; i < MAX_IMAGE_HEIGHT; ++i)
		max7219_setdigit(i, image[i]);
}

/* x, y are from 0 to 7 */
void image_emplace_letter_xy(image_t image, cletter_t letter, byte_t x, byte_t y)
{
	for (int i = 0; i < (sizeof(cletter_t) / sizeof letter[0]) ; ++i) {
		byte_t mask = letter[i] << x;
		image[i + y] &= ~mask;
		image[i + y] |= mask;
	}
}

void image_clear(image_t image)
{
	for (int i = 0; i < MAX_IMAGE_HEIGHT; ++i)
		image[i] = 0;
}

void image_cpy(image_t dst, cimage_t src)
{
	for (int i = 0; i < MAX_IMAGE_HEIGHT; ++i)
		dst[i] = src[i];
}

/* i, j are from 0 to 7 */
void image_set_px(image_t image, int i, int j)
{
	BIT_SET(image[i], j);
}

/*  Reads image from program memory to local buffer RAM
 * and returns pointer on that buffer. Buffer can be modified by user,
 * but note that will be overriden on the next call to image_progread() */
image_ref_t image_progread(cimage_t image)
	{ return memcpy_P(_image_buffer, image, sizeof(cimage_t)); }

/* Similar to image_progread() */
letter_ref_t letter_progread(cletter_t letter)
	{ return memcpy_P(_letter_buffer, letter, sizeof(cletter_t)); }

#ifdef DRAWING_USING_COMMON_IMAGES
	cimage_t prog_img_smile PROGMEM = {
		0b00000000,
		0b01100110,
		0b01100110,
		0b00000000,
		0b00000000,
		0b01100110,
		0b00111100,
		0b00000000
	};

	cimage_t prog_img_sad_smile PROGMEM = {
		0b00000000,
		0b01100110,
		0b01100110,
		0b00000000,
		0b00000000,
		0b00111100,
		0b01100110,
		0b00000000
	};

	cimage_t prog_img_arrow_up PROGMEM = {
		0b00000000,
		0b00011000,
		0b00111100,
		0b01011010,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00000000
	};

	cimage_t prog_img_arrow_down PROGMEM = {
		0b00000000,
		0b00011000,
		0b00011000,
		0b00011000,
		0b01011010,
		0b00111100,
		0b00011000,
		0b00000000
	};

	cimage_t prog_img_arrow_right PROGMEM = {
		0b00000000,
		0b00001000,
		0b00000100,
		0b01111110,
		0b01111110,
		0b00000100,
		0b00001000,
		0b00000000
	};

	cimage_t prog_img_arrow_left PROGMEM = {
		0b00000000,
		0b00010000,
		0b00100000,
		0b01111110,
		0b01111110,
		0b00100000,
		0b00010000,
		0b00000000
	};
#endif // DRAWING_USING_COMMON_IMAGES

#ifdef DRAWING_USING_NUMBERS
	const cletter_t small_digits[] PROGMEM = {
		{ // zero
			0b111,
			0b101,
			0b101,
			0b101,
			0b111
		}, { // one 
			0b001,
			0b011,
			0b101,
			0b001,
			0b001		
		}, { // two
			0b111,
			0b001,
			0b111,
			0b100,
			0b111
		}, { // three
			0b111,
			0b001,
			0b111,
			0b001,
			0b111
		}, { // four
			0b101,
			0b101,
			0b111,
			0b001,
			0b001
		}, { // five
			0b111,
			0b100,
			0b111,
			0b001,
			0b111
		}, { // six
			0b111,
			0b100,
			0b111,
			0b101,
			0b111
		}, { // seven
			0b111,
			0b001,
			0b010,
			0b100,
			0b100
		}, { // eight
			0b111,
			0b101,
			0b111,
			0b101,
			0b111
		}, { // nine
			0b111,
			0b101,
			0b111,
			0b001,
			0b111
		}
	};

	void image_emplace_letter_left(image_t image, cletter_t letter) { image_emplace_letter_xy(image, letter, 5, 2); }
	void image_emplace_letter_right(image_t image, cletter_t letter) { image_emplace_letter_xy(image, letter, 0, 2); }
	void image_emplace_letter_center(image_t image, cletter_t letter) { image_emplace_letter_xy(image, letter, 2, 2); }

	/* number from 0 to 99 */
	void image_emplace_number(image_t image, int number)
	{
		if (number < 10)
			image_emplace_letter_center(image, letter_progread(small_digits[number % 10]));
		else {
			image_emplace_letter_right(image, letter_progread(small_digits[number % 10]));
			image_emplace_letter_left(image, letter_progread(small_digits[number / 10]));
		}
	}
#endif // DRAWING_USING_NUMBERS

#endif // DRAWING_H_