/* visual effects for 8x8 led matrix */

#ifndef EFFECTS_H_
#define EFFECTS_H_

#include "decls.h"
#include "drawing.h"
#include "timing.h"

void draw_effect_blink(uint16_t delay_ms, int ntimes)
{
	for (int i = 0; i < ntimes; ++i) {
		timer1a_wait_ms(delay_ms);
		max7219_enable_shutdown(true);
		timer1a_wait_ms(delay_ms);
		max7219_enable_shutdown(false);
	}
}

void draw_effect_shift_left(cimage_t image, uint16_t step_speed_ms)
{
	image_t image_buf;
	image_cpy(image_buf, image);
	image_show_max7219(image_buf);

	for (unsigned int i = 0; i < MAX_IMAGE_WIDTH; ++i) {
		timer1a_wait_ms(step_speed_ms);
		for (int row = 0; row < MAX_IMAGE_HEIGHT; ++row)
			image_buf[row] <<= 1;
		image_show_max7219(image_buf);
	}
}

void draw_effect_swap_shift_left(cimage_t fst, cimage_t snd, uint16_t step_speed_ms)
{
	image_t image_buf;
	image_cpy(image_buf, fst);
	image_show_max7219(image_buf);

	for (unsigned int i = 0; i < MAX_IMAGE_WIDTH; ++i) {
		timer1a_wait_ms(step_speed_ms);
		for (unsigned int row = 0; row < MAX_IMAGE_HEIGHT; ++row) {
			image_buf[row] <<= 1;
			BIT_SET_TO(image_buf[row], 0, snd[row] & (1 << MAX_IMAGE_WIDTH - i - 1));
		}
		image_show_max7219(image_buf);
	}
}

/* works only for 8x8 images */
void draw_effect_shift_to_sides(cimage_t image, uint16_t step_speed_ms)
{
	image_t image_buf;
	image_cpy(image_buf, image);
	image_show_max7219(image_buf);

	for (unsigned int i = 0; i < MAX_IMAGE_WIDTH / 2; ++i) {
		timer1a_wait_ms(step_speed_ms);
		for (int row = 0; row < MAX_IMAGE_HEIGHT; ++row) {
			byte_t row_val = image_buf[row];

			image_buf[row] = (0b11100000 & (image_buf[row] << 1))
				| (0b00000111 & (image_buf[row] >> 1));
		}
		image_show_max7219(image_buf);
	}
}

/* text must be in program memory, not RAM (!) */
void draw_effect_moving_text(cletter_t *text, unsigned int text_len, uint16_t step_speed_ms)
{
	image_t left;
	image_t right = {};

	for (int i = 0; i < text_len; ++i) {
		image_cpy(left, right);
		image_clear(right);
		image_emplace_letter_xy(right, image_progread(text[i]), 0, 2);
		draw_effect_swap_shift_left(left, right, step_speed_ms);
	}
}

#endif // EFFECTS_H_