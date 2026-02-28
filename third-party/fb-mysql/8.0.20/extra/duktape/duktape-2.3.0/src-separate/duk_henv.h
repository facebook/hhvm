/*
 *  Environment object representation.
 */

#if !defined(DUK_HENV_H_INCLUDED)
#define DUK_HENV_H_INCLUDED

#define DUK_ASSERT_HDECENV_VALID(h) do { \
		DUK_ASSERT((h) != NULL); \
		DUK_ASSERT(DUK_HOBJECT_IS_DECENV((duk_hobject *) (h))); \
		DUK_ASSERT((h)->thread == NULL || (h)->varmap != NULL); \
	} while (0)

#define DUK_ASSERT_HOBJENV_VALID(h) do { \
		DUK_ASSERT((h) != NULL); \
		DUK_ASSERT(DUK_HOBJECT_IS_OBJENV((duk_hobject *) (h))); \
		DUK_ASSERT((h)->target != NULL); \
		DUK_ASSERT((h)->has_this == 0 || (h)->has_this == 1); \
	} while (0)

struct duk_hdecenv {
	/* Shared object part. */
	duk_hobject obj;

	/* These control variables provide enough information to access live
	 * variables for a closure that is still open.  If thread == NULL,
	 * the record is closed and the identifiers are in the property table.
	 */
	duk_hthread *thread;
	duk_hobject *varmap;
	duk_size_t regbase_byteoff;
};

struct duk_hobjenv {
	/* Shared object part. */
	duk_hobject obj;

	/* Target object and 'this' binding for object binding. */
	duk_hobject *target;

	/* The 'target' object is used as a this binding in only some object
	 * environments.  For example, the global environment does not provide
	 * a this binding, but a with statement does.
	 */
	duk_bool_t has_this;
};

#endif  /* DUK_HENV_H_INCLUDED */
