/*
 *  Bound function representation.
 */

#if !defined(DUK_HBOUNDFUNC_H_INCLUDED)
#define DUK_HBOUNDFUNC_H_INCLUDED

/* Artificial limit for args length.  Ensures arithmetic won't overflow
 * 32 bits when combining bound functions.
 */
#define DUK_HBOUNDFUNC_MAX_ARGS 0x20000000UL

#define DUK_ASSERT_HBOUNDFUNC_VALID(h) do { \
		DUK_ASSERT((h) != NULL); \
		DUK_ASSERT(DUK_HOBJECT_IS_BOUNDFUNC((duk_hobject *) (h))); \
		DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(&(h)->target) || \
		           (DUK_TVAL_IS_OBJECT(&(h)->target) && \
		            DUK_HOBJECT_IS_CALLABLE(DUK_TVAL_GET_OBJECT(&(h)->target)))); \
		DUK_ASSERT(!DUK_TVAL_IS_UNUSED(&(h)->this_binding)); \
		DUK_ASSERT((h)->nargs == 0 || (h)->args != NULL); \
	} while (0)

struct duk_hboundfunc {
	/* Shared object part. */
	duk_hobject obj;

	/* Final target function, stored as duk_tval so that lightfunc can be
	 * represented too.
	 */
	duk_tval target;

	/* This binding. */
	duk_tval this_binding;

	/* Arguments to prepend. */
	duk_tval *args;  /* Separate allocation. */
	duk_idx_t nargs;
};

#endif  /* DUK_HBOUNDFUNC_H_INCLUDED */
