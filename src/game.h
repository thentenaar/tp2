/**
 * tp2 - Game Logic
 * Copyright (C) 2015 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef GAME_H
#define GAME_H

/* Board width in tiles. */
#define BOARD_WIDTH 4

/* Board height in tiles. */
#define BOARD_HEIGHT 4

/* Number of digits in the score. */
#define SCORE_SIZE 12

/* Game termination states. */
#define GAME_WON  1
#define GAME_OVER 2

/* Whether the cells are free (1) or occupied (0). */
extern short board_state;

/* Game termination state (GAME_WON or GAME_OVER.) */
extern int game_state;

/* Game type (winning exponent of 2) */
extern int game_type;

extern char board[BOARD_WIDTH * BOARD_HEIGHT];
extern char score[SCORE_SIZE + 1];

/**
 * Initialize all memory areas used to represent
 * the game stae.
 */
void init_game_state(void);

/**
 * Handle input from the user, drive the game state,
 * and check for the "game over" condition.
 */
void game_handle_key(int key);

#endif /* GAME_H */
