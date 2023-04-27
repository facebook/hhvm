/*
 *  Very simple example program for evaluating expressions from
 *  command line
 */

#include "duktape.h"
#include <stdio.h>

static duk_ret_t native_print(duk_context *ctx) {
	duk_push_string(ctx, " ");
	duk_insert(ctx, 0);
	duk_join(ctx, duk_get_top(ctx) - 1);
	printf("%s\n", duk_to_string(ctx, -1));
	return 0;
}

static duk_ret_t eval_raw(duk_context *ctx, void *udata) {
	(void) udata;
	duk_eval(ctx);
	return 1;
}

static duk_ret_t tostring_raw(duk_context *ctx, void *udata) {
	(void) udata;
	duk_to_string(ctx, -1);
	return 1;
}

static void usage_exit(void) {
	fprintf(stderr, "Usage: eval <expression> [<expression>] ...\n");
	fflush(stderr);
	exit(1);
}

int main(int argc, char *argv[]) {
	duk_context *ctx;
	int i;
	const char *res;

	if (argc < 2) {
		usage_exit();
	}

	ctx = duk_create_heap_default();

	duk_push_c_function(ctx, native_print, DUK_VARARGS);
	duk_put_global_string(ctx, "print");

	for (i = 1; i < argc; i++) {
		printf("=== eval: '%s' ===\n", argv[i]);
		duk_push_string(ctx, argv[i]);
		duk_safe_call(ctx, eval_raw, NULL, 1 /*nargs*/, 1 /*nrets*/);
		duk_safe_call(ctx, tostring_raw, NULL, 1 /*nargs*/, 1 /*nrets*/);
		res = duk_get_string(ctx, -1);
		printf("%s\n", res ? res : "null");
		duk_pop(ctx);
	}

	duk_destroy_heap(ctx);

	return 0;
}
