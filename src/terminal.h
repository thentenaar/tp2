/**
 * tp2 - Terminal / Signal Handling Routines
 * Copyright (C) 2015 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef TERM_H
#define TERM_H

/**
 * Initialize the terminal.
 *
 * \return NULL on success, error message on error.
 */
const char *term_init(void);

/**
 * Restore the screen (if supported)
 */
void term_uninit(void);

#endif /* TERM_H */
