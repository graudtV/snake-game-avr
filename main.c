#include <avr/io.h>
#include <avr/interrupt.h>

/* images, etc */
#define DRAWING_USING_COMMON_IMAGES
#define DRAWING_USING_NUMBERS
#include "drawing.h"

/* snake game configuration */
#define MAX_SNAKE_LENGTH 64
#define SNAKE_GAME_WIDTH 8
#define SNAKE_GAME_HEIGHT 8
#include "snake_game.h"

/* legs for connecting joystick */
#define JOYSTICK_VX_PIN 0
#define JOYSTICK_VY_PIN 1

/* mappings from joystick directions to directions in snake game */
typedef enum {
	JOYSTICK_UNKNOWN	= DIR_UNKNOWN,
	JOYSTICK_LEFT		= DIR_UP,
	JOYSTICK_RIGHT		= DIR_DOWN,
	JOYSTICK_UP		 	= DIR_LEFT,
	JOYSTICK_DOWN		= DIR_RIGHT
} joystick_dir_t;

#include "async_joystick.h"
#include "timing.h"
#include "effects.h"

/* port for connecting button on joystick */
#define BUTTON_PORT PORTA
#define BUTTON_PORTDD DDRA
#define BUTTON_PORTIN PINA
#include "button.h"
#define JOYSTICK_BUTTON_PIN 2

snake_game_t game;
volatile snake_dir_t snake_dir = DIR_UNKNOWN;
volatile bool_t show_message_for_good_mark = false;

void draw_game_map(snake_game_map_t map)
{
	image_t image = {};
	for (unsigned int y = 0; y < SNAKE_GAME_HEIGHT; ++y)
		for (unsigned int x = 0; x < SNAKE_GAME_WIDTH; ++x)
			if (map[y][x])
				BIT_SET(image[y], SNAKE_GAME_WIDTH - x - 1);
	image_show_max7219(image);
}

/* return val is in milliseconds */
uint16_t score_to_speed(int score)
{
	if (score < 3)
		return 500;
	else if (score < 5)
		return 300;
	else if (score < 10)
		return 250;
	else if (score < 15)
		return 200;
	else if (score < 25)
		return 200 - (score - 15) * 10;
	else
		return 100;
}

/* called on timer1 interrupts during active game phase */
void game_update_callback()
{
	if (button_is_pressed(JOYSTICK_BUTTON_PIN)) {
		show_message_for_good_mark = true;
		return;
	}
	snake_game_update(&game, snake_dir);
	draw_game_map(game.map);
	timer1a_change_timeout_ms(score_to_speed(game.score));
}

/* called by async_joystick notifications */
void snake_dir_update_callback(joystick_dir_t dir)
{
	/*  If user returns joystick to initial pos, the previous pos is stored,
	 * thus user can press joystick a bit earlier, than snake should turn
	 * It feels much more convinient during playing. Implementation
	 * of this behaviour why I have to use asynchronous access to joystick */
	if (dir != DIR_UNKNOWN)
		snake_dir = (snake_dir_t) dir;
}

void start_countdown(int from)
{
	static cimage_t image_zero PROGMEM = {
		0b00000000,
		0b00000000,
		0b01110010,
		0b01010010,
		0b01010010,
		0b01010000,
		0b01110010,
		0b00000000
	};

	for (int i = from; i > 0; --i) {
		image_t number = {};
		image_emplace_number(number, i);
		image_show_max7219(number);
		timer1a_wait_ms(1000);
	}
	image_show_max7219(image_progread(image_zero));
	timer1a_wait_ms(1000);
}

void ask_for_good_mark()
{
	static ctext_t text PROGMEM = {
		{
			0b11100100,
			0b10101010,
			0b10101010,
			0b10101010,
			0b10100100
		}, {
			0b01101110,
			0b10000100,
			0b10000100,
			0b10000100,
			0b01100100
		}, {
			0b01001100,
			0b10101010,
			0b11101100,
			0b10101010,
			0b10101100
		}, {
			0b10001110,
			0b10000100,
			0b11100100,
			0b10100100,
			0b11100100
		}, {
			0b11100000,
			0b10000000,
			0b11100000,
			0b10000000,
			0b11100000
		}, {
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		}
	};

	draw_effect_moving_text(text, ARR_SZ(text), 250);

	image_t image = {};
	image_emplace_number(image, 10);
	image_show_max7219(image);

	draw_effect_blink(250, 5);

	draw_effect_shift_to_sides(image, 700);
	timer1a_wait_ms(300);

	image_show_max7219(image_progread(prog_img_smile));
	timer1a_wait_ms(3000);
}

void wait(long ncycles)
{
	while (ncycles-- > 0)
		asm volatile ("nop");
}

/* may be called multiple times */
void run_game()
{
	snake_dir = DIR_UNKNOWN;
	snake_game_init(&game); // configure game
	start_countdown(3);
	timer1a_start_ms(score_to_speed(game.score), game_update_callback);

	volatile snake_game_t *pgame = &game;
	while (!pgame->is_finished && !show_message_for_good_mark)
		;

	timer1a_stop();

	if (show_message_for_good_mark) {
		cli();
		ask_for_good_mark();
		show_message_for_good_mark = false;
		sei();
		return;
	}

	draw_effect_blink(250, 5);

	image_t score = {};
	image_emplace_number(score, game.score);
	image_show_max7219(score);

	timer1a_wait_ms(3000);
}

int main()
{
	/* led matrix configuration */
	max7219_init_ports();
	max7219_clear_digits();
	max7219_set_ndigits(8);
	max7219_set_intencity(15);
	max7219_wakeup();

	/* timers configuration */
	timer1_init();

	/* joystick configuration */
	async_joystick_init_ports();
	async_joystick_start();
	async_joystick_start_notify(snake_dir_update_callback); // enable notifications about direction changes

	/* buttons configuration */
	button_init_ports(JOYSTICK_BUTTON_PIN);

	sei();

	while (1)
		run_game();
	return 0;
}