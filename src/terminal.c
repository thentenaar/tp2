/**
 * tp2 - Terminal / Signal Handling Routines
 * Copyright (C) 2015 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <curses.h>
#include <term.h>

#include "terminal.h"

/* Signal flags */
extern volatile sig_atomic_t got_signal;
extern volatile sig_atomic_t got_winch;

/* Buffer for termcap setting strings */
static char buf[128];

/* Error messages for term_init() */
static const char *error_messages[3] = {
	"stdout is not a tty",
	"TERM is not set",
	"termcap entry not found"
};

static void sighandler(int sig)
{
#ifdef SIGWINCH
	if (sig == SIGWINCH) got_winch = 1;
	else got_signal = 1;
#else
	(void)sig;
	got_signal = 1;
#endif
}

/**
 * Initialize the terminal.
 *
 * \return NULL on success, error message on error.
 */
const char *term_init(void)
{
	struct sigaction sa;
	const char *err = NULL;
	char *tmp, *bufp = buf;
	int retval;

	memset(buf, 0, sizeof(buf));
	if (!isatty(STDOUT_FILENO)) {
		err = error_messages[0];
		goto ret;
	}

	/* Make sure we have TERM */
	tmp = getenv("TERM");
	if (!tmp) {
		err = error_messages[1];
		goto ret;
	}

	/* SUSv2+ tgetent() ignores the buffer parameter. */
	retval = tgetent(NULL, tmp);
	if (retval <= 0) {
		err = error_messages[2];
		goto ret;
	}

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

ret:
	return err;
}

/**
 * Restore the screen (if supported)
 */
void term_uninit(void)
{
	char *bufp = buf, *tmp;
	tmp = tgetstr("te", &bufp);
	if (tmp) putp(tmp);
}
