/* Platform-independent snake game logic
 * Declare the folowing constants before including this file:
 * MAX_SNAKE_LENGTH
 * SNAKE_GAME_WIDTH
 * SNAKE_GAME_HEIGHT
 */

#ifndef SNAKE_GAME_H_
#define SNAKE_GAME_H_

#include "decls.h"

typedef struct {
	unsigned int y, x;
} coord_t;

typedef enum {
	DIR_UNKNOWN, DIR_LEFT = -1, DIR_RIGHT = 1, DIR_UP = -2, DIR_DOWN = 2
} snake_dir_t;

typedef struct {
	coord_t segments[MAX_SNAKE_LENGTH];
	snake_dir_t dir;
	unsigned int tail, head;
} snake_t;

typedef enum {
	CELL_EMPTY, CELL_SNAKE, CELL_RABBIT
} cell_t; // type of inhabitant inside a cell in game map

typedef cell_t snake_game_map_t[SNAKE_GAME_HEIGHT][SNAKE_GAME_WIDTH];

typedef struct {
/* public: */
	bool_t is_finished;
/* read-only: */
	unsigned int score; // current game score (i.e. length of snake)
	snake_game_map_t map; // can be used to draw the game
/* private: */
	snake_t snake;
	coord_t rabbit;	
} snake_game_t;

#define MAP_ELEM(map, coord) ((map)[(coord.y)][(coord.x)])

/*  Snake must either have enough space,
 * or it must have size == MAX_SNAKE_LENGTH and snake_pop_segment() must
 * be called before any other operations with snake (the latter feature is used
 * when moving snake of maximal size) */
void snake_add_segment(snake_t *s, coord_t segment)
{
	s->head = (s->head + 1) % MAX_SNAKE_LENGTH;
	s->segments[s->head] = segment;
}

/* snake must have at least 2 elements */
void snake_pop_segment(snake_t *s)
	{ s->tail = (s->tail + 1) % MAX_SNAKE_LENGTH; }

void snake_move(snake_t *s, coord_t new_head)
{
	/* works correctly even if snake size == MAX_SNAKE_LENGTH, see 
	 * comment to snake_add_segment() */
	snake_add_segment(s, new_head);
	snake_pop_segment(s);
}

/* Chooses direction between previous and new one (next_dir)
 * If next_dir == DIR_UNKNOWN or is opposite to the current dir, s->dir is used as dir
 * Otherwise next_dir is used */
snake_dir_t snake_choose_dir(const snake_t *s, snake_dir_t next_dir)
{
	return (next_dir == DIR_UNKNOWN || next_dir == -s->dir) // opposite dirs have opposite values
		? s->dir : next_dir;
}

/* Returns coord, where snake head will be according to direction, stored in snake */
coord_t snake_next_head_pos(snake_t *s)
{
	coord_t new_head = s->segments[s->head];

	switch (s->dir) {
	case DIR_LEFT: new_head.x = (new_head.x + SNAKE_GAME_WIDTH - 1) % SNAKE_GAME_WIDTH; break;
	case DIR_RIGHT: new_head.x = (new_head.x + 1) % SNAKE_GAME_WIDTH; break;
	case DIR_UP: new_head.y = (new_head.y + SNAKE_GAME_HEIGHT - 1) % SNAKE_GAME_HEIGHT; break;
	case DIR_DOWN: new_head.y = (new_head.y + 1) % SNAKE_GAME_HEIGHT; break;
	default: /* error */ break;
	}
	return new_head;
}

void snake_clear_game_map(snake_game_map_t map)
{
	for (unsigned int y = 0; y < SNAKE_GAME_HEIGHT; ++y)
		for (unsigned int x = 0; x < SNAKE_GAME_WIDTH; ++x)
			map[y][x] = CELL_EMPTY;	
}

void snake_init(snake_t *s, coord_t init_pos)
{
	s->dir = DIR_UP;
	s->segments[0] = init_pos;
	s->head = s->tail = 0;
}

int count_empty_neighbours(snake_game_map_t map, unsigned int y, unsigned int x)
{
	int res = 0;
	if (map[y][x] != CELL_EMPTY)
		return -1;
	if (y > 0)
		res += (map[y - 1][x] == CELL_EMPTY);
	if (y < SNAKE_GAME_HEIGHT - 1)
		res += (map[y + 1][x] == CELL_EMPTY);
	if (x > 0)
		res += (map[y][x - 1] == CELL_EMPTY);
	if (x < SNAKE_GAME_WIDTH - 1)
		res += (map[y][x + 1] == CELL_EMPTY);
	// res *= 4;
	// res += 8 - ABS(x - 4) - ABS(y - 4);
	return res;
}

#define SNAKE_GET_EMPTY_COORD_NATTEMPTS_ 3

coord_t snake_get_empty_coord(snake_game_map_t map)
{
	coord_t best_coord;
	int maxempty = -1;

	for (unsigned int y = 0; y < SNAKE_GAME_HEIGHT; ++y)
		for (unsigned int x = 0; x < SNAKE_GAME_WIDTH; ++x) {
			int nempty = count_empty_neighbours(map, y, x);
			if (nempty > maxempty) {
				best_coord.y = y;
				best_coord.x = x;
				maxempty = nempty;
			}
		}
	/* cycle should always find at least one empty coord */
	return best_coord;
}

void snake_game_init(snake_game_t *game)
{
	coord_t snake_init_pos = {3, 3};
	
	snake_init(&game->snake, snake_init_pos);
	snake_clear_game_map(game->map);
	MAP_ELEM(game->map, snake_init_pos) = CELL_SNAKE;

	coord_t rabbit_init_pos = snake_get_empty_coord(game->map);

	game->rabbit = rabbit_init_pos;
	MAP_ELEM(game->map, rabbit_init_pos) = CELL_RABBIT;

	game->is_finished = false;
	game->score = 1;
}

/* If next_dir == DIR_UNKNOWN, snake continues moving in the same direction */
void snake_game_update(snake_game_t *game, snake_dir_t next_dir)
{
	if (game->is_finished)
		return;

	game->snake.dir = snake_choose_dir(&game->snake, next_dir);
	coord_t new_head = snake_next_head_pos(&game->snake);

	if (new_head.x == game->rabbit.x && new_head.y == game->rabbit.y) { // rabbit collision
		game->rabbit = snake_get_empty_coord(game->map);
		MAP_ELEM(game->map, game->rabbit) = CELL_RABBIT;
		snake_add_segment(&game->snake, new_head);
		MAP_ELEM(game->map, new_head) = CELL_SNAKE; // rewrites CELL_RABBIT
		++game->score;
		return;
	}
	coord_t tail = game->snake.segments[game->snake.tail];
	MAP_ELEM(game->map, tail) = CELL_EMPTY;

	if (MAP_ELEM(game->map, new_head) != CELL_EMPTY) { // self-collision
		game->is_finished = true;
	} else {
		snake_move(&game->snake, new_head);
		MAP_ELEM(game->map, new_head) = CELL_SNAKE;
	}
}

#endif // SNAKE_GAME_H_