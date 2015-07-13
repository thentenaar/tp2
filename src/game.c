/**
 * tp2 - Game Logic
 * Copyright (C) 2015 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <curses.h>

#include "game.h"

/* Whether the cells are free (1) or occupied (0). */
short board_state = -1;

/* Game termination state (GAME_WON or GAME_OVER.) */
int game_state = 0;

/* Game type (winning exponent of 2) */
int game_type = 11; /* 2048 */

/* Number of tiles to start with */
static int starting_tiles = 2;

char board[BOARD_WIDTH * BOARD_HEIGHT];
char score[SCORE_SIZE + 1];

/**
 * Add a tile to a random position on the board.
 *
 * In "2048", tiles are added as:
 *   "2" (90% probability)
 *   "4" (10% probability)
 * and I've preserved those odds here.
 */
static void add_random_tile(void)
{
	int cell;
	int e = 1;

	/* Generate the exponent for the random tile. */
	srand((unsigned int)time(NULL));
	if ((rand() / (double)RAND_MAX) > 0.90) e <<= 1;

	do {
		cell = 1 + (rand() % 15);
		if (!(board_state & (1 << cell)))
			continue;

		/* Set the cell. */
		board[cell] = (char)e;
		board_state &= (short)~(1 << cell);
		break;
	} while (board_state && board_state != 1);
}

/**
 * Add a value to the score, updating each digit independently.
 *
 * \param[in] num value to add
 */
static void add_to_score(unsigned int num)
{
	int i = SCORE_SIZE - 2;

	do {
		score[i] = (char)(score[i] + (int)(num % 10));
		if (score[i] > '9') {
			score[i - 1]++;
			score[i] = '0';
		}
		num /= 10;
	} while (score[i - 1] != '0' && --i);
}

/**
 * If two cells match, merge them, and make the 2nd cell
 * unoccupied.
 *
 * This function also updates the score, and checks if the player
 * has won.
 *
 * \param[in] a Cell to merge into.
 * \param[in] b Cell to merge from.
 * \return 1 if the two cells were merged, 0 otherwise.
 */
static int reduce_line(int a, int b)
{
	int merged = 0;

	if (board[a] == board[b]) {
		board[a] = (board[a] + 1) & 0x0f;
		board[b] = 0;
		board_state |= (short)(1 << b);

		if (board[a] == game_type)
			game_state = GAME_WON;
		add_to_score((unsigned int)(4 << board[a]));
		merged = 1;
	}

	return merged;
}

/**
 * Find the next occupied cell in the line and move it
 * down.
 *
 * \param[in] start      Current unoccupied cell.
 * \param[in] stride     Distance to the next cell.
 * \param[in] last_empty Last scanned empty cell on the line.
 * \return The index of the last empty cell scanned on the line.
 */
static int shift_line(int start, int end, int stride, int last_empty)
{
	int i = (last_empty == -1) ? start : last_empty;

	while (!board[start] && i != end) {
		if (!board[i]) {
			board_state |= (short)(1 << i);
			i += stride;
			continue;
		}

		/* Move the next occupied cell down. */
		board_state &= (short)~((1 << start));
		board_state |= (short)(1 << i);
		board[start] = board[i];
		board[i] = 0;
	}

	return i;
}

/**
 * Move cells in a row or column, merging the first pair
 * of matching cells.
 *
 * \param[in] start  Starting index in the board array.
 * \param[in] stride Distance to the next cell.
 */
static void move_line(int start, int stride)
{
	int i = start, end = start + stride * BOARD_WIDTH;
	int empty = -1, matched = 0;

	do {
		if (!board[i]) {
			empty = shift_line(i, end, stride, empty);
			if (empty == end) break;
		}

		if (i != start && !matched) {
			matched = reduce_line(i - stride, i);
			if (matched) i -= stride;
		}

		i += stride;
	} while (i != end);
}

/**
 * Move the whole board in a given direction.
 *
 * \param[in] start       Cell from which to start moving.
 * \param[in] stride      Distance to the next cell.
 * \param[in] next_stride Distance to the next line.
 */
static void move_board(int start, int stride, int next_stride)
{
	int i, end = start + next_stride * BOARD_WIDTH;

	for (i = start; i != end; i += next_stride)
		move_line(i, stride);
	add_random_tile();
}

/**
 * Check for a match across a the board, scanning right and down,
 * stopping at the first match.
 *
 * \return 1 if a match was found, 0 otherwise.
 */
static int find_match(void)
{
	int i, row = 0, matched = 0;

	while (row < BOARD_WIDTH * BOARD_HEIGHT && !matched) {
		for (i = 0; i < BOARD_WIDTH && !matched; i++) {
			if (!board[row + i]) continue;

			/* Check right */
			if ((i + 1 < BOARD_WIDTH &&
			    board[row + i] == board[row + i + 1]))
				matched = 1;

			/* Check down */
			if (!matched &&
			    board[row + i] == board[row + i + BOARD_WIDTH])
				matched = 1;
		}

		row += BOARD_WIDTH;
	}

	return matched;
}

/**
 * Initialize all memory areas used to represent
 * the game stae.
 */
void init_game_state(void)
{
	int i;

	board_state = -1;
	game_state = 0;
	memset(board, 0, sizeof(board));
	memset(score, '0', SCORE_SIZE - 1);
	score[SCORE_SIZE] = 0;

	/* Add the starting tiles */
	for (i = 0; i < starting_tiles; i++)
		add_random_tile();
}

/**
 * Handle input from the user, drive the game state,
 * and check for the "game over" condition.
 */
void game_handle_key(int key)
{
	switch (key) {
	case KEY_UP:
		move_board(0, BOARD_WIDTH, 1);
		break;
	case KEY_DOWN:
		move_board(BOARD_WIDTH * (BOARD_HEIGHT - 1),
		           -BOARD_WIDTH, 1);
		break;
	case KEY_LEFT:
		move_board(0, 1, BOARD_WIDTH);
		break;
	case KEY_RIGHT:
		move_board(BOARD_WIDTH - 1, -1, BOARD_WIDTH);
		break;
	}

	/**
	 * If the board is full, and no matches remain,
	 * the game is over.
	 */
	if ((!board_state || board_state == 1) && !find_match())
		game_state = GAME_OVER;
}
