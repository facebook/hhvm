/*
 *  Memory allocation handling.
 */

#include "duk_internal.h"

/*
 *  Voluntary GC check
 */

#if defined(DUK_USE_VOLUNTARY_GC)
DUK_LOCAL DUK_INLINE void duk__check_voluntary_gc(duk_heap *heap) {
	if (DUK_UNLIKELY(--(heap)->ms_trigger_counter < 0)) {
#if defined(DUK_USE_DEBUG)
		if (heap->ms_prevent_count == 0) {
			DUK_D(DUK_DPRINT("triggering voluntary mark-and-sweep"));
		} else {
			DUK_DD(DUK_DDPRINT("gc blocked -> skip voluntary mark-and-sweep now"));
		}
#endif

		/* Prevention checks in the call target handle cases where
		 * voluntary GC is not allowed.  The voluntary GC trigger
		 * counter is only rewritten if mark-and-sweep actually runs.
		 */
		duk_heap_mark_and_sweep(heap, DUK_MS_FLAG_VOLUNTARY /*flags*/);
	}
}
#define DUK__VOLUNTARY_PERIODIC_GC(heap)  do { duk__check_voluntary_gc((heap)); } while (0)
#else
#define DUK__VOLUNTARY_PERIODIC_GC(heap)  /* no voluntary gc */
#endif  /* DUK_USE_VOLUNTARY_GC */

/*
 *  Allocate memory with garbage collection
 */

DUK_INTERNAL void *duk_heap_mem_alloc(duk_heap *heap, duk_size_t size) {
	void *res;
	duk_small_int_t i;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT_DISABLE(size >= 0);

	/*
	 *  Voluntary periodic GC (if enabled)
	 */

	DUK__VOLUNTARY_PERIODIC_GC(heap);

	/*
	 *  First attempt
	 */

#if defined(DUK_USE_GC_TORTURE)
	/* Simulate alloc failure on every alloc, except when mark-and-sweep
	 * is running.
	 */
	if (heap->ms_prevent_count == 0) {
		DUK_DDD(DUK_DDDPRINT("gc torture enabled, pretend that first alloc attempt fails"));
		res = NULL;
		DUK_UNREF(res);
		goto skip_attempt;
	}
#endif
	res = heap->alloc_func(heap->heap_udata, size);
	if (DUK_LIKELY(res || size == 0)) {
		/* For zero size allocations NULL is allowed. */
		return res;
	}
#if defined(DUK_USE_GC_TORTURE)
 skip_attempt:
#endif

	DUK_D(DUK_DPRINT("first alloc attempt failed, attempt to gc and retry"));

#if 0
	/*
	 *  Avoid a GC if GC is already running.  This can happen at a late
	 *  stage in a GC when we try to e.g. resize the stringtable
	 *  or compact objects.
	 *
	 *  NOTE: explicit handling isn't actually be needed: if the GC is
	 *  not allowed, duk_heap_mark_and_sweep() will reject it for every
	 *  attempt in the loop below, resulting in a NULL same as here.
	 */

	if (heap->ms_prevent_count != 0) {
		DUK_D(DUK_DPRINT("duk_heap_mem_alloc() failed, gc in progress (gc skipped), alloc size %ld", (long) size));
		return NULL;
	}
#endif

	/*
	 *  Retry with several GC attempts.  Initial attempts are made without
	 *  emergency mode; later attempts use emergency mode which minimizes
	 *  memory allocations forcibly.
	 */

	for (i = 0; i < DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_LIMIT; i++) {
		duk_small_uint_t flags;

		flags = 0;
		if (i >= DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_EMERGENCY_LIMIT - 1) {
			flags |= DUK_MS_FLAG_EMERGENCY;
		}

		duk_heap_mark_and_sweep(heap, flags);

		res = heap->alloc_func(heap->heap_udata, size);
		if (res) {
			DUK_D(DUK_DPRINT("duk_heap_mem_alloc() succeeded after gc (pass %ld), alloc size %ld",
			                 (long) (i + 1), (long) size));
			return res;
		}
	}

	DUK_D(DUK_DPRINT("duk_heap_mem_alloc() failed even after gc, alloc size %ld", (long) size));
	return NULL;
}

DUK_INTERNAL void *duk_heap_mem_alloc_zeroed(duk_heap *heap, duk_size_t size) {
	void *res;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT_DISABLE(size >= 0);

	res = DUK_ALLOC(heap, size);
	if (DUK_LIKELY(res != NULL)) {
		duk_memzero(res, size);
	}
	return res;
}

DUK_INTERNAL void *duk_heap_mem_alloc_checked(duk_hthread *thr, duk_size_t size) {
	void *res;

	DUK_ASSERT(thr != NULL);
	res = duk_heap_mem_alloc(thr->heap, size);
	if (DUK_LIKELY(res != NULL || size == 0)) {
		return res;
	}
	DUK_ERROR_ALLOC_FAILED(thr);
	DUK_WO_NORETURN(return NULL;);
}

DUK_INTERNAL void *duk_heap_mem_alloc_checked_zeroed(duk_hthread *thr, duk_size_t size) {
	void *res;

	DUK_ASSERT(thr != NULL);
	res = duk_heap_mem_alloc_zeroed(thr->heap, size);
	if (DUK_LIKELY(res != NULL || size == 0)) {
		return res;
	}
	DUK_ERROR_ALLOC_FAILED(thr);
	DUK_WO_NORETURN(return NULL;);
}

/*
 *  Reallocate memory with garbage collection
 */

DUK_INTERNAL void *duk_heap_mem_realloc(duk_heap *heap, void *ptr, duk_size_t newsize) {
	void *res;
	duk_small_int_t i;

	DUK_ASSERT(heap != NULL);
	/* ptr may be NULL */
	DUK_ASSERT_DISABLE(newsize >= 0);

	/*
	 *  Voluntary periodic GC (if enabled)
	 */

	DUK__VOLUNTARY_PERIODIC_GC(heap);

	/*
	 *  First attempt
	 */

#if defined(DUK_USE_GC_TORTURE)
	/* Simulate alloc failure on every realloc, except when mark-and-sweep
	 * is running.
	 */
	if (heap->ms_prevent_count == 0) {
		DUK_DDD(DUK_DDDPRINT("gc torture enabled, pretend that first realloc attempt fails"));
		res = NULL;
		DUK_UNREF(res);
		goto skip_attempt;
	}
#endif
	res = heap->realloc_func(heap->heap_udata, ptr, newsize);
	if (DUK_LIKELY(res || newsize == 0)) {
		/* For zero size allocations NULL is allowed. */
		return res;
	}
#if defined(DUK_USE_GC_TORTURE)
 skip_attempt:
#endif

	DUK_D(DUK_DPRINT("first realloc attempt failed, attempt to gc and retry"));

#if 0
	/*
	 *  Avoid a GC if GC is already running.  See duk_heap_mem_alloc().
	 */

	if (heap->ms_prevent_count != 0) {
		DUK_D(DUK_DPRINT("duk_heap_mem_realloc() failed, gc in progress (gc skipped), alloc size %ld", (long) newsize));
		return NULL;
	}
#endif

	/*
	 *  Retry with several GC attempts.  Initial attempts are made without
	 *  emergency mode; later attempts use emergency mode which minimizes
	 *  memory allocations forcibly.
	 */

	for (i = 0; i < DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_LIMIT; i++) {
		duk_small_uint_t flags;

		flags = 0;
		if (i >= DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_EMERGENCY_LIMIT - 1) {
			flags |= DUK_MS_FLAG_EMERGENCY;
		}

		duk_heap_mark_and_sweep(heap, flags);

		res = heap->realloc_func(heap->heap_udata, ptr, newsize);
		if (res || newsize == 0) {
			DUK_D(DUK_DPRINT("duk_heap_mem_realloc() succeeded after gc (pass %ld), alloc size %ld",
			                 (long) (i + 1), (long) newsize));
			return res;
		}
	}

	DUK_D(DUK_DPRINT("duk_heap_mem_realloc() failed even after gc, alloc size %ld", (long) newsize));
	return NULL;
}

/*
 *  Reallocate memory with garbage collection, using a callback to provide
 *  the current allocated pointer.  This variant is used when a mark-and-sweep
 *  (e.g. finalizers) might change the original pointer.
 */

DUK_INTERNAL void *duk_heap_mem_realloc_indirect(duk_heap *heap, duk_mem_getptr cb, void *ud, duk_size_t newsize) {
	void *res;
	duk_small_int_t i;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT_DISABLE(newsize >= 0);

	/*
	 *  Voluntary periodic GC (if enabled)
	 */

	DUK__VOLUNTARY_PERIODIC_GC(heap);

	/*
	 *  First attempt
	 */

#if defined(DUK_USE_GC_TORTURE)
	/* Simulate alloc failure on every realloc, except when mark-and-sweep
	 * is running.
	 */
	if (heap->ms_prevent_count == 0) {
		DUK_DDD(DUK_DDDPRINT("gc torture enabled, pretend that first indirect realloc attempt fails"));
		res = NULL;
		DUK_UNREF(res);
		goto skip_attempt;
	}
#endif
	res = heap->realloc_func(heap->heap_udata, cb(heap, ud), newsize);
	if (DUK_LIKELY(res || newsize == 0)) {
		/* For zero size allocations NULL is allowed. */
		return res;
	}
#if defined(DUK_USE_GC_TORTURE)
 skip_attempt:
#endif

	DUK_D(DUK_DPRINT("first indirect realloc attempt failed, attempt to gc and retry"));

#if 0
	/*
	 *  Avoid a GC if GC is already running.  See duk_heap_mem_alloc().
	 */

	if (heap->ms_prevent_count != 0) {
		DUK_D(DUK_DPRINT("duk_heap_mem_realloc_indirect() failed, gc in progress (gc skipped), alloc size %ld", (long) newsize));
		return NULL;
	}
#endif

	/*
	 *  Retry with several GC attempts.  Initial attempts are made without
	 *  emergency mode; later attempts use emergency mode which minimizes
	 *  memory allocations forcibly.
	 */

	for (i = 0; i < DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_LIMIT; i++) {
		duk_small_uint_t flags;

#if defined(DUK_USE_DEBUG)
		void *ptr_pre;
		void *ptr_post;
#endif

#if defined(DUK_USE_DEBUG)
		ptr_pre = cb(heap, ud);
#endif
		flags = 0;
		if (i >= DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_EMERGENCY_LIMIT - 1) {
			flags |= DUK_MS_FLAG_EMERGENCY;
		}

		duk_heap_mark_and_sweep(heap, flags);
#if defined(DUK_USE_DEBUG)
		ptr_post = cb(heap, ud);
		if (ptr_pre != ptr_post) {
			DUK_DD(DUK_DDPRINT("realloc base pointer changed by mark-and-sweep: %p -> %p",
			                   (void *) ptr_pre, (void *) ptr_post));
		}
#endif

		/* Note: key issue here is to re-lookup the base pointer on every attempt.
		 * The pointer being reallocated may change after every mark-and-sweep.
		 */

		res = heap->realloc_func(heap->heap_udata, cb(heap, ud), newsize);
		if (res || newsize == 0) {
			DUK_D(DUK_DPRINT("duk_heap_mem_realloc_indirect() succeeded after gc (pass %ld), alloc size %ld",
			                 (long) (i + 1), (long) newsize));
			return res;
		}
	}

	DUK_D(DUK_DPRINT("duk_heap_mem_realloc_indirect() failed even after gc, alloc size %ld", (long) newsize));
	return NULL;
}

/*
 *  Free memory
 */

DUK_INTERNAL void duk_heap_mem_free(duk_heap *heap, void *ptr) {
	DUK_ASSERT(heap != NULL);
	/* ptr may be NULL */

	/* Must behave like a no-op with NULL and any pointer returned from
	 * malloc/realloc with zero size.
	 */
	heap->free_func(heap->heap_udata, ptr);

	/* Never perform a GC (even voluntary) in a memory free, otherwise
	 * all call sites doing frees would need to deal with the side effects.
	 * No need to update voluntary GC counter either.
	 */
}
