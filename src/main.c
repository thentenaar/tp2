/**
 * tp2 - Main Routines
 * Copyright (C) 2015 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <curses.h>

#include "ui.h"
#include "game.h"

#ifndef PDCURSES
#include "terminal.h"
#endif

/* Signal flags (see terminal.c) */
volatile sig_atomic_t got_signal = 0;
volatile sig_atomic_t got_winch = 0;

/**
 * Show usage information.
 */
static void usage(char *argv0)
{
	printf("Usage: %s [-t game_type] [-b]\n", argv0);
	puts("\t-b:           Black & white mode");
	puts("\t-t game_type: Set the game type.\n");
	puts("\t  The game type signfies the exponent of");
	puts("\t  2 you have to reach to win the game. It");
	puts("\t  must be between 10 [1024] and 15 [32768], ");
	puts("\t  and the default value is 11 [2048].\n");
}

int main(int argc, char *argv[])
{
	const char *err = NULL;
	int i, retval, key;

	/* Handle args */
	for (i = 1; i < argc; i++) {
		if (!argv[i] || argv[i][0] != '-')
			break;

		switch (argv[i][1]) {
		case 't': /* -t: Game type (10 - 15, default: 11 [2048]) */
			if (i + 1 < argc) {
				key = atoi(argv[i + 1]);
				if (key < 10 || key > 15) {
					err = "game type must be between 10 and 15.";
					goto err;
				}
				game_type = key & 0x0f;
				++i;
			}
			break;
		case 'b': /* -b: Black & White mode (i.e. don't use colors) */
			colors = 0;
			break;
		default:
			usage(argv[0]);
			goto err;
		}
	}

#ifndef PDCURSES
	err = term_init();
	if (err) goto err;
#endif

	init_game_state();
	ui_init();

	/* Render the UI and feed input into the game logic. */
	while (!got_signal) {
		if (got_winch) {
			got_winch = 0;
			ui_window_size_changed();
		}

		ui_render_game_state();
		key = getch();
		if (key == ERR) continue;

#ifdef PDCURSES
		/* PDCurses sends this on Ctrl + C. */
		if (key == 3) got_signal = 1;
#endif

#ifdef KEY_RESIZE
		if (key == KEY_RESIZE)
			ui_window_size_changed();
#endif
		/* Allow the user to restart when 'r' is pressed. */
		if (!game_state) game_handle_key(key);
		else if (key == 'r') init_game_state();
	}
	ui_uninit();

#ifndef PDCURSES
	term_uninit();
#endif

err:
	/* If we have an error message, print it */
	if (err) {
		fprintf(stderr, "error: %s\n", err);
		retval = EXIT_FAILURE;
	} else retval = EXIT_SUCCESS;
	return retval;
}
