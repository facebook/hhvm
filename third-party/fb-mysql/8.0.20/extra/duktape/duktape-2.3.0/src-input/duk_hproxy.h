/*
 *  Proxy object representation.
 */

#if !defined(DUK_HPROXY_H_INCLUDED)
#define DUK_HPROXY_H_INCLUDED

#define DUK_ASSERT_HPROXY_VALID(h) do { \
		DUK_ASSERT((h) != NULL); \
		DUK_ASSERT((h)->target != NULL); \
		DUK_ASSERT((h)->handler != NULL); \
		DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ((duk_hobject *) (h))); \
	} while (0)

struct duk_hproxy {
	/* Shared object part. */
	duk_hobject obj;

	/* Proxy target object. */
	duk_hobject *target;

	/* Proxy handlers (traps). */
	duk_hobject *handler;
};

#endif  /* DUK_HPROXY_H_INCLUDED */
