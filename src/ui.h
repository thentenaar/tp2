/**
 * tp2 - User Interface Routines
 * Copyright (C) 2015 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
 */
#ifndef UI_H
#define UI_H

/* Non-zero if the display has colors and the user wants colors */
extern int colors;

/**
 * Initialize the UI.
 */
void ui_init(void);

/**
 * Detect changes in the window size.
 */
void ui_window_size_changed(void);

/**
 * Draw the score, the grid, and the cells.
 */
void ui_render_game_state(void);

/**
 * Uninitialize the UI.
 */
void ui_uninit(void);

#endif /* UI_H */
