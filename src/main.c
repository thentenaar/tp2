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
#include <unistd.h>
#include <signal.h>
#include <curses.h>
#include <term.h>

#include "ui.h"
#include "game.h"

/* Buffer for termcap setting strings */
static char buf[128];

/* Signal flags */
static volatile sig_atomic_t got_signal = 0;
static volatile sig_atomic_t got_winch = 0;

static void sighandler(int sig)
{
#ifdef SIGWINCH
	if (sig == SIGWINCH) got_winch = 1;
	else got_signal = 1;
#else
	got_signal = 1;
#endif
}

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
	struct sigaction sa;
	const char *err = NULL;
	char *tmp, *bufp = buf;
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

	init_game_state();
	memset(buf, 0, sizeof(buf));
	if (!isatty(STDOUT_FILENO))
		goto not_a_tty;

	tmp = getenv("TERM");
	if (!tmp) goto term_not_set;

	/* SUSv2+ tgetent() ignores the buffer parameter. */
	retval = tgetent(NULL, tmp);
	if (retval <= 0)
		goto termcap_not_found;

	/* Switch to the alternate screen (if supported.) */
	tmp = tgetstr("ti", &bufp);
	if (tmp) putp(tmp);

	/* Setup signal handling */
	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = sighandler;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
#ifdef SIGWINCH
	sigaction(SIGWINCH, &sa, NULL);
#endif

	/* Render the UI and feed input into the game logic. */
	ui_init();
	while (!got_signal) {
		if (got_winch) {
			got_winch = 0;
			ui_window_size_changed();
		}

		ui_render_game_state();
		key = getch();
		if (key == ERR) continue;
#ifdef KEY_RESIZE
		if (key == KEY_RESIZE)
			ui_window_size_changed();
#endif
		/* Allow the user to restart when 'r' is pressed. */
		if (!game_state) game_handle_key(key);
		else if (key == 'r') init_game_state();
	}
	ui_uninit();

	/* Restore the screen (if supported.) */
	bufp = buf;
	tmp = tgetstr("te", &bufp);
	if (tmp) putp(tmp);

err:
	/* If we have an error message, print it */
	if (err) {
		fprintf(stderr, "error: %s\n", err);
		retval = EXIT_FAILURE;
	} else retval = EXIT_SUCCESS;
	return retval;

not_a_tty:
	err = "stdout is not a tty";
	goto err;

term_not_set:
	err = "TERM is not set";
	goto err;

termcap_not_found:
	err = "termcap entry not found";
	goto err;
}
