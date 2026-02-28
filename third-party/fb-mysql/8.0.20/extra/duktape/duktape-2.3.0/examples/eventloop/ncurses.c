/*
 *  Ncurses bindings example.
 *
 *  VALGRIND NOTE: when you use ncurses, there seems to be no way to get a
 *  clean valgrind run.  Even if ncurses state is properly shut down, there
 *  will still be some residual leaks.
 *
 *  Debian: install libncurses5-dev
 */

#include <curses.h>
#include "duktape.h"

static int ncurses_initscr(duk_context *ctx) {
	WINDOW *win;

	win = initscr();
	duk_push_pointer(ctx, (void *) win);
	return 1;
}

static int ncurses_endwin(duk_context *ctx) {
	int rc;

	rc = endwin();
	duk_push_int(ctx, rc);
	return 1;
}

static int ncurses_delscreen(duk_context *ctx) {
	/* XXX: no screen management now */
	(void) ctx;
	return 0;
}

static int ncurses_getmaxyx(duk_context *ctx) {
	int row, col;

	getmaxyx(stdscr, row, col);

	duk_push_array(ctx);
	duk_push_int(ctx, row);
	duk_put_prop_index(ctx, -2, 0);
	duk_push_int(ctx, col);
	duk_put_prop_index(ctx, -2, 1);
	return 1;
}

static int ncurses_printw(duk_context *ctx) {
	int rc;
	const char *str;

	str = duk_to_string(ctx, 0);
	rc = printw("%s", str);
	duk_push_int(ctx, rc);
	return 1;
}

static int ncurses_mvprintw(duk_context *ctx) {
	int y = duk_to_int(ctx, 0);
	int x = duk_to_int(ctx, 1);
	const char *str = duk_to_string(ctx, 2);
	int rc;

	rc = mvprintw(y, x, "%s", str);
	duk_push_int(ctx, rc);
	return 1;
}

static int ncurses_refresh(duk_context *ctx) {
	int rc;

	rc = refresh();
	duk_push_int(ctx, rc);
	return 1;
}

static int ncurses_getch(duk_context *ctx) {
	int rc;

	rc = getch();
	duk_push_int(ctx, rc);
	return 1;
}

static duk_function_list_entry ncurses_funcs[] = {
	{ "initscr", ncurses_initscr, 0 },
	{ "endwin", ncurses_endwin, 0 },
	{ "delscreen", ncurses_delscreen, 0 },
	{ "getmaxyx", ncurses_getmaxyx, 0 },
	{ "printw", ncurses_printw, 1 },
	{ "mvprintw", ncurses_mvprintw, 3 },
	{ "refresh", ncurses_refresh, 0 },
	{ "getch", ncurses_getch, 0 },
	{ NULL, NULL, 0 }
};

void ncurses_register(duk_context *ctx) {
	/* Set global 'Ncurses'. */
	duk_push_global_object(ctx);
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, ncurses_funcs);
	duk_put_prop_string(ctx, -2, "Ncurses");
	duk_pop(ctx);
}
