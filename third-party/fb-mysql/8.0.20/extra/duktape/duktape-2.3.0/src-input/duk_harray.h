/*
 *  Array object representation, used for actual Array instances.
 *
 *  All objects with the exotic array behavior (which must coincide with having
 *  internal class array) MUST be duk_harrays.  No other object can be a
 *  duk_harray.  However, duk_harrays may not always have an array part.
 */

#if !defined(DUK_HARRAY_H_INCLUDED)
#define DUK_HARRAY_H_INCLUDED

#define DUK_ASSERT_HARRAY_VALID(h) do { \
		DUK_ASSERT((h) != NULL); \
		DUK_ASSERT(DUK_HOBJECT_IS_ARRAY((duk_hobject *) (h))); \
		DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_ARRAY((duk_hobject *) (h))); \
	} while (0)

#define DUK_HARRAY_LENGTH_WRITABLE(h)         (!(h)->length_nonwritable)
#define DUK_HARRAY_LENGTH_NONWRITABLE(h)      ((h)->length_nonwritable)
#define DUK_HARRAY_SET_LENGTH_WRITABLE(h)     do { (h)->length_nonwritable = 0; } while (0)
#define DUK_HARRAY_SET_LENGTH_NONWRITABLE(h)  do { (h)->length_nonwritable = 1; } while (0)

struct duk_harray {
	/* Shared object part. */
	duk_hobject obj;

	/* Array .length.
	 *
	 * At present Array .length may be smaller, equal, or even larger
	 * than the allocated underlying array part.  Fast path code must
	 * always take this into account carefully.
	 */
	duk_uint32_t length;

	/* Array .length property attributes.  The property is always
	 * non-enumerable and non-configurable.  It's initially writable
	 * but per Object.defineProperty() rules it can be made non-writable
	 * even if it is non-configurable.  Thus we need to track the
	 * writability explicitly.
	 *
	 * XXX: this field to be eliminated and moved into duk_hobject
	 * flags field to save space.
	 */
	duk_bool_t length_nonwritable;
};

#endif  /* DUK_HARRAY_H_INCLUDED */
