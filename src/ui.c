/**
 * tp2 - User Interface Routines
 * Copyright (C) 2015 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdlib.h>
#include <curses.h>

#include "game.h"
#include "ui.h"

/* Size of the game display area (in lines/cols) */
#define WIDTH  29
#define HEIGHT 20

/* Non-zero if the display has colors and the user wants colors */
int colors = 1;

/* Starting row, col for our display. */
static int row0 = 1;
static int col0 = 1;

/* Total number of cols and rows */
static int cols = 0;
static int rows = 0;

/**
 * Non-zero if the screen is too small to
 * draw our display.
 */
static int screen_too_small = 0;

/**
 * Non-zero if we already drew the grid that encapsulates
 * the cells.
 */
static int rendered_grid = 0;

static const char *instructions[4] = {
	"Use the arrow keys to move the",
	"tiles, Ctrl + C to exit       ",
	"Press 'r' to restart the game,",
	"or Ctrl + C to exit           "
};

/* Numbers to be displayed within a cell */
static const char *numbers[16] = {
	"      ",
	"   2  ",
	"   4  ",
	"   8  ",
	"  16  ",
	"  32  ",
	"  64  ",
	"  128 ",
	"  256 ",
	"  512 ",
	" 1024 ",
	" 2048 ",
	" 4096 ",
	" 8192 ",
	" 16384",
	" 32768"
};

/**
 * The default colors for rendering cells.
 *
 * These correspond to color pairs 2 - 8.
 */
static const short cell_colors[6] = {
	(COLOR_RED << 8) | COLOR_WHITE,
	(COLOR_BLUE << 8) | COLOR_WHITE,
	(COLOR_GREEN << 8) | COLOR_BLACK,
	(COLOR_MAGENTA << 8) | COLOR_WHITE,
	(COLOR_CYAN << 8) | COLOR_BLACK,
	(COLOR_YELLOW << 8) | COLOR_BLACK,
};

/**
 * Color pairs for each type of cell.
 */
static short cell_color_pairs[17] = {
	/* Empty cell */
	1,

	/* 2, 4, 8, 16, 32 */
	2, 3, 4, 5, 6,

	/* 64, 128, 256, 512, 1024, 2048 */
	7, 3, 4, 5, 6, 7,

	/* 4096, 8192, 16384, 32768 */
	5, 4, 3, 2
};

/**
 * Additional colors for terminals that support color
 * redefinition.
 */
static const long custom_colors[9] = {
	0xd75f0000 | COLOR_BLACK, /* 128 (Orange) */
	0x005fd700 | COLOR_WHITE, /* 256 (Light Blue) */
	0x5fff5f00 | COLOR_BLACK, /* 512 (Light Green) */
	0x5f008700 | COLOR_WHITE, /* 1024 (Purple-ish) */
	0x8a8a8a00 | COLOR_BLACK, /* 2048 (Gray) */
	0xffd70000 | COLOR_BLACK, /* 4096 (Gold) */
	0xd7870000 | COLOR_BLACK, /* 8192 (Lighter Orange) */
	0x005f5f00 | COLOR_WHITE, /* 16394 (Teal) */
	0x00d70000 | COLOR_BLACK, /* 32768 (Lighter Green) */
};

/**
 * Alternatives to custom_colors for xterm-color-compatible terminals
 * with 256 color support, that lack init_color() capability
 * (e.g. screen-256color, putty-256color.)
 */
static const long xterm_colors[9] = {
	(166 << 8) | COLOR_BLACK,
	(26 << 8) | COLOR_WHITE,
	(83 << 8) | COLOR_BLACK,
	(54 << 8) | COLOR_WHITE,
	(245 << 8) | COLOR_BLACK,
	(220 << 8) | COLOR_BLACK,
	(172 << 8) | COLOR_BLACK,
	(23 << 8) | COLOR_WHITE,
	(40 << 8) | COLOR_BLACK,
};

/* 24-bit RGB to curses intensities */
#define R(X) (short)((((X) >> 24) & 0xff) << 2)
#define G(X) (short)((((X) >> 16) & 0xff) << 2)
#define B(X) (short)((((X) >>  8) & 0xff) << 2)

/**
 * Use xterm/custom colors on terminals that support
 * more than 8 colors.
 *
 * This routine will use as many colors as possible. If
 * the terminal supports 256 colors, but can't init colors,
 * xterm-compatible color numbers will be used.
 *
 * \param[in] next_pair Next available color pair
 */
static void define_custom_colors(short next_pair)
{
	int i, ccc = can_change_color();
	short color;
	long c;

	for (i = 1; i < 10 && COLORS - i > 7; i++, next_pair++) {
		if (ccc) {
			color = (short)(COLORS - i);
			c = custom_colors[i - 1];
			if (init_color(color, R(c), G(c), B(c)) != OK) {
				/* Fall back to xterm colors */
				ccc = 0;
				i = 1;
				next_pair--;
				continue;
			}
		} else if (COLORS == 256) {
			c = xterm_colors[i - 1];
			color = (short)((c >> 8) & 0xff);
		} else break;

		if (init_pair(next_pair, c & 0xff, color) != OK)
			break;
		cell_color_pairs[7 + i - 1] = next_pair;
	}
}

/**
 * Setup the colors for the state area and grid, and the
 * blocks themselves.
 */
static void define_colors(void)
{
	int i;
	short c;

	colors = has_colors();
	if (colors && init_pair(1, COLOR_WHITE, COLOR_BLACK) != OK)
		colors = 0;

	if (colors) {
		/* Initialize the basic cell color pairs */
		for (i = 2; i < 8; i++) {
			c = cell_colors[i - 2];
			init_pair((short)i, c & 0xff, c >> 8);
		}

		if (COLORS > 8 && COLOR_PAIRS > i)
			define_custom_colors((short)i);
	}
}

/**
 * Draw one cell.
 *
 * In the cell we store the current exponent of two
 * that the cell represents, and render this on the middle
 * line. The other two lines are blank.
 *
 * The result will look like:
 *
 * +----------+
 * |          |
 * |  number  |
 * |          |
 * +----------+
 *
 * If no colors are available, the blocks will be drawn in
 * reverse video mode.
 *
 * \param[in] cell Cell number to draw
 */
static void draw_cell(int cell)
{
	int row, col, e;

	row = cell & -4;
	col = cell & 3;
	e = board[cell] & 0x0f;

	if (colors) attron(COLOR_PAIR(cell_color_pairs[e]));
	else if (e) attron(A_REVERSE);
	mvaddstr(row0 + 2 + row, col0 + 1 + col * 7, numbers[0]);
	mvaddstr(row0 + 3 + row, col0 + 1 + col * 7, numbers[e]);
	mvaddstr(row0 + 4 + row, col0 + 1 + col * 7, numbers[0]);
	if (colors) attroff(COLOR_PAIR(cell_color_pairs[e]));
	else if (e) attroff(A_REVERSE);
}

/**
 * Draw the grid that will encapsulate the cells.
 *
 * Since our display is designed for a 4x4 grid,
 * and each cell consists of 3 inner lines,
 * the horizontal lines will be 4 rows apart.
 *
 * The vertical lines will be 7 columns apart.
 */
static void draw_grid(void)
{
	int i, j;

	/* Top line */
	if (colors) attron(COLOR_PAIR(1));
	mvhline(row0 + 1, col0, ACS_ULCORNER, 1);
	mvhline(row0 + 1, col0 + 1, 0, WIDTH - 2);
	mvhline(row0 + 1, col0 + WIDTH - 1, ACS_URCORNER, 1);

	/* Left line */
	mvvline(row0 + 2, col0, 0, HEIGHT - 4);
	mvvline(row0 + HEIGHT - 3, col0, ACS_LLCORNER, 1);

	/* Right line */
	mvvline(row0 + 2, col0 + WIDTH - 1, 0, HEIGHT - 4);
	mvvline(row0 + HEIGHT - 3, col0 + WIDTH - 1, ACS_LRCORNER, 1);

	/* Bottom line */
	mvhline(row0 + HEIGHT - 3, col0 + 1, 0, WIDTH - 2);

	/* Vertical inner lines */
	for (i = 7; i < WIDTH - 1; i += 7) {
		mvvline(row0 + 1, col0 + i, ACS_TTEE, 1);
		mvvline(row0 + 2, col0 + i, 0, HEIGHT - 5);
		mvvline(row0 + HEIGHT - 3, col0 + i, ACS_BTEE, 1);
	}

	/* Horizontal inner lines */
	for (i = 4; i < (4 * BOARD_HEIGHT); i += 4) {
		mvhline(row0 + 1 + i, col0, ACS_LTEE, 1);
		for (j = 0; j < WIDTH - 1; j += 7) {
			mvhline(row0 + 1 + i, col0 + 1 + j, 0, 6);
			mvhline(row0 + 1 + i, col0 + 7 + j, ACS_PLUS, 1);
		}
		mvhline(row0 + 1 + i, col0 + WIDTH - 1, ACS_RTEE, 1);
	}
	if (colors) attroff(COLOR_PAIR(1));
	rendered_grid = 1;
}

#ifdef DEBUG
/**
 * Draw debug info on the left-hand side of the
 * screen, if the board is drawn after our
 * requisite 20 columns.
 */
static void draw_debug(void)
{
	int i;
	if (col0 < 20) return;

	mvaddstr(row0, 5, "DEBUG");
	move(row0 + 2, 0);
	printw("state       = %d", game_state);
	move(row0 + 3, 0);
	printw("board_state = 0x%04x", ~board_state & 0xffff);

	for (i = 0; i < BOARD_WIDTH * BOARD_HEIGHT; i += BOARD_WIDTH) {
		move(row0 + 5 + (i / BOARD_WIDTH), 0);
		printw("board[%d] = 0x%1x%1x%1x%1x ", i / BOARD_WIDTH,
		       board[i] & 0x0f, board[i + 1] & 0x0f,
		       board[i + 2] & 0x0f, board[i + 3] & 0x0f);
	}
}
#endif

/**
 * Initialize the UI.
 */
void ui_init(void)
{
	/* Initialize curses */
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	scrollok(stdscr, FALSE);

	/* Set-up our colors */
	if (colors) {
		if (start_color() == OK)
			define_colors();
		else colors = 0;
	}

	/* Render the initial game state */
	ui_window_size_changed();
	ui_render_game_state();
}

/**
 * Detect changes in the window size.
 */
void ui_window_size_changed(void)
{
	screen_too_small = 0;
	endwin();
	clear();
	refresh();

	if (COLS < WIDTH + 1 || LINES < HEIGHT + 1) {
		clear();
		move(0, 0);
		printw("The screen is too small to play this game.\n");
		printw("Resize the window, or press Ctrl + C to exit.");
		refresh();
		screen_too_small = 1;
	} else if (col0 != (COLS - WIDTH) / 2 ||
	           row0 != (LINES - HEIGHT) / 2) {
		col0 = row0 = 1;
		if (COLS > WIDTH) col0 = (COLS - WIDTH) / 2;
		if (LINES > HEIGHT) row0 = (LINES - HEIGHT) / 2;
	}

	cols = COLS;
	rows = LINES;
	rendered_grid = 0;
}

/**
 * Draw the score, the grid, and the cells.
 */
void ui_render_game_state(void)
{
	int i;

	if (screen_too_small)
		goto ret;

	if (!rendered_grid) draw_grid();
	if (colors) attron(COLOR_PAIR(1));

#ifdef DEBUG
	draw_debug();
#endif

	if (game_state == GAME_WON) {
		beep();
		attron(A_BLINK);
		mvaddstr(row0, col0, "YOU WIN  ");
		attroff(A_BLINK);
		mvaddstr(row0 + HEIGHT - 2, col0, instructions[2]);
		mvaddstr(row0 + HEIGHT - 1, col0, instructions[3]);
	} else if (game_state == GAME_OVER) {
		beep();
		mvaddstr(row0, col0, "GAME OVER");
		mvaddstr(row0 + HEIGHT - 2, col0, instructions[2]);
		mvaddstr(row0 + HEIGHT - 1, col0, instructions[3]);
	} else {
		move(row0, col0);
		clrtoeol();
		mvaddstr(row0, col0, numbers[game_type]);
		mvaddstr(row0 + HEIGHT - 2, col0, instructions[0]);
		mvaddstr(row0 + HEIGHT - 1, col0, instructions[1]);
	}

	mvaddstr(row0, col0 + WIDTH - SCORE_SIZE, score);
	if (colors) attroff(COLOR_PAIR(1));

	/* Draw the cells */
	for (i = 0; i < 16; i++)
		draw_cell(i);
ret:
	refresh();
}

/**
 * Uninitialize the UI.
 */
void ui_uninit(void)
{
	erase();
	endwin();
}
