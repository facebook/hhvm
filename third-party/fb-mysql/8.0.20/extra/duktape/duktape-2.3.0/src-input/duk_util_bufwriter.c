/*
 *  Fast buffer writer with slack management.
 */

#include "duk_internal.h"

/* XXX: Avoid duk_{memcmp,memmove}_unsafe() by imposing a minimum length of
 * >0 for the underlying dynamic buffer.
 */

/*
 *  Macro support functions (use only macros in calling code)
 */

DUK_LOCAL void duk__bw_update_ptrs(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx, duk_size_t curr_offset, duk_size_t new_length) {
	duk_uint8_t *p;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw_ctx != NULL);
	DUK_UNREF(thr);

	/* 'p' might be NULL when the underlying buffer is zero size.  If so,
	 * the resulting pointers are not used unsafely.
	 */
	p = (duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, bw_ctx->buf);
	DUK_ASSERT(p != NULL || (DUK_HBUFFER_DYNAMIC_GET_SIZE(bw_ctx->buf) == 0 && curr_offset == 0 && new_length == 0));
	bw_ctx->p = p + curr_offset;
	bw_ctx->p_base = p;
	bw_ctx->p_limit = p + new_length;
}

DUK_INTERNAL void duk_bw_init(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx, duk_hbuffer_dynamic *h_buf) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw_ctx != NULL);
	DUK_ASSERT(h_buf != NULL);

	bw_ctx->buf = h_buf;
	duk__bw_update_ptrs(thr, bw_ctx, 0, DUK_HBUFFER_DYNAMIC_GET_SIZE(h_buf));
}

DUK_INTERNAL void duk_bw_init_pushbuf(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx, duk_size_t buf_size) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw_ctx != NULL);

	(void) duk_push_dynamic_buffer(thr, buf_size);
	bw_ctx->buf = (duk_hbuffer_dynamic *) duk_known_hbuffer(thr, -1);
	DUK_ASSERT(bw_ctx->buf != NULL);
	duk__bw_update_ptrs(thr, bw_ctx, 0, buf_size);
}

/* Resize target buffer for requested size.  Called by the macro only when the
 * fast path test (= there is space) fails.
 */
DUK_INTERNAL duk_uint8_t *duk_bw_resize(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx, duk_size_t sz) {
	duk_size_t curr_off;
	duk_size_t add_sz;
	duk_size_t new_sz;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw_ctx != NULL);

	/* We could do this operation without caller updating bw_ctx->ptr,
	 * but by writing it back here we can share code better.
	 */

	curr_off = (duk_size_t) (bw_ctx->p - bw_ctx->p_base);
	add_sz = (curr_off >> DUK_BW_SLACK_SHIFT) + DUK_BW_SLACK_ADD;
	new_sz = curr_off + sz + add_sz;
	if (DUK_UNLIKELY(new_sz < curr_off)) {
		/* overflow */
		DUK_ERROR_RANGE(thr, DUK_STR_BUFFER_TOO_LONG);
		DUK_WO_NORETURN(return NULL;);
	}
#if 0  /* for manual torture testing: tight allocation, useful with valgrind */
	new_sz = curr_off + sz;
#endif

	/* This is important to ensure dynamic buffer data pointer is not
	 * NULL (which is possible if buffer size is zero), which in turn
	 * causes portability issues with e.g. memmove() and memcpy().
	 */
	DUK_ASSERT(new_sz >= 1);

	DUK_DD(DUK_DDPRINT("resize bufferwriter from %ld to %ld (add_sz=%ld)", (long) curr_off, (long) new_sz, (long) add_sz));

	duk_hbuffer_resize(thr, bw_ctx->buf, new_sz);
	duk__bw_update_ptrs(thr, bw_ctx, curr_off, new_sz);
	return bw_ctx->p;
}

/* Make buffer compact, matching current written size. */
DUK_INTERNAL void duk_bw_compact(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx) {
	duk_size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw_ctx != NULL);
	DUK_UNREF(thr);

	len = (duk_size_t) (bw_ctx->p - bw_ctx->p_base);
	duk_hbuffer_resize(thr, bw_ctx->buf, len);
	duk__bw_update_ptrs(thr, bw_ctx, len, len);
}

DUK_INTERNAL void duk_bw_write_raw_slice(duk_hthread *thr, duk_bufwriter_ctx *bw, duk_size_t src_off, duk_size_t len) {
	duk_uint8_t *p_base;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw != NULL);
	DUK_ASSERT(src_off <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(len <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(src_off + len <= DUK_BW_GET_SIZE(thr, bw));
	DUK_UNREF(thr);

	p_base = bw->p_base;
	duk_memcpy_unsafe((void *) bw->p,
	                  (const void *) (p_base + src_off),
	                  (size_t) len);
	bw->p += len;
}

DUK_INTERNAL void duk_bw_write_ensure_slice(duk_hthread *thr, duk_bufwriter_ctx *bw, duk_size_t src_off, duk_size_t len) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw != NULL);
	DUK_ASSERT(src_off <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(len <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(src_off + len <= DUK_BW_GET_SIZE(thr, bw));

	DUK_BW_ENSURE(thr, bw, len);
	duk_bw_write_raw_slice(thr, bw, src_off, len);
}

DUK_INTERNAL void duk_bw_insert_raw_bytes(duk_hthread *thr, duk_bufwriter_ctx *bw, duk_size_t dst_off, const duk_uint8_t *buf, duk_size_t len) {
	duk_uint8_t *p_base;
	duk_size_t buf_sz, move_sz;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw != NULL);
	DUK_ASSERT(dst_off <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(buf != NULL);
	DUK_UNREF(thr);

	p_base = bw->p_base;
	buf_sz = (duk_size_t) (bw->p - p_base);  /* constrained by maximum buffer size */
	move_sz = buf_sz - dst_off;

	DUK_ASSERT(p_base != NULL);  /* buffer size is >= 1 */
	duk_memmove_unsafe((void *) (p_base + dst_off + len),
	                   (const void *) (p_base + dst_off),
	                   (size_t) move_sz);
	duk_memcpy_unsafe((void *) (p_base + dst_off),
	                  (const void *) buf,
	                  (size_t) len);
	bw->p += len;
}

DUK_INTERNAL void duk_bw_insert_ensure_bytes(duk_hthread *thr, duk_bufwriter_ctx *bw, duk_size_t dst_off, const duk_uint8_t *buf, duk_size_t len) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw != NULL);
	DUK_ASSERT(dst_off <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(buf != NULL);

	DUK_BW_ENSURE(thr, bw, len);
	duk_bw_insert_raw_bytes(thr, bw, dst_off, buf, len);
}

DUK_INTERNAL void duk_bw_insert_raw_slice(duk_hthread *thr, duk_bufwriter_ctx *bw, duk_size_t dst_off, duk_size_t src_off, duk_size_t len) {
	duk_uint8_t *p_base;
	duk_size_t buf_sz, move_sz;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw != NULL);
	DUK_ASSERT(dst_off <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(src_off <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(len <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(src_off + len <= DUK_BW_GET_SIZE(thr, bw));
	DUK_UNREF(thr);

	p_base = bw->p_base;

	/* Don't support "straddled" source now. */
	DUK_ASSERT(dst_off <= src_off || dst_off >= src_off + len);

	if (dst_off <= src_off) {
		/* Target is before source.  Source offset is expressed as
		 * a "before change" offset.  Account for the memmove.
		 */
		src_off += len;
	}

	buf_sz = (duk_size_t) (bw->p - p_base);
	move_sz = buf_sz - dst_off;

	DUK_ASSERT(p_base != NULL);  /* buffer size is >= 1 */
	duk_memmove_unsafe((void *) (p_base + dst_off + len),
	                   (const void *) (p_base + dst_off),
	                   (size_t) move_sz);
	duk_memcpy_unsafe((void *) (p_base + dst_off),
	                  (const void *) (p_base + src_off),
	                  (size_t) len);
	bw->p += len;
}

DUK_INTERNAL void duk_bw_insert_ensure_slice(duk_hthread *thr, duk_bufwriter_ctx *bw, duk_size_t dst_off, duk_size_t src_off, duk_size_t len) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw != NULL);
	DUK_ASSERT(dst_off <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(src_off <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(len <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(src_off + len <= DUK_BW_GET_SIZE(thr, bw));

	/* Don't support "straddled" source now. */
	DUK_ASSERT(dst_off <= src_off || dst_off >= src_off + len);

	DUK_BW_ENSURE(thr, bw, len);
	duk_bw_insert_raw_slice(thr, bw, dst_off, src_off, len);
}

DUK_INTERNAL duk_uint8_t *duk_bw_insert_raw_area(duk_hthread *thr, duk_bufwriter_ctx *bw, duk_size_t off, duk_size_t len) {
	duk_uint8_t *p_base, *p_dst, *p_src;
	duk_size_t buf_sz, move_sz;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw != NULL);
	DUK_ASSERT(off <= DUK_BW_GET_SIZE(thr, bw));
	DUK_UNREF(thr);

	p_base = bw->p_base;
	buf_sz = (duk_size_t) (bw->p - p_base);
	move_sz = buf_sz - off;
	p_dst = p_base + off + len;
	p_src = p_base + off;
	duk_memmove_unsafe((void *) p_dst, (const void *) p_src, (size_t) move_sz);
	return p_src;  /* point to start of 'reserved area' */
}

DUK_INTERNAL duk_uint8_t *duk_bw_insert_ensure_area(duk_hthread *thr, duk_bufwriter_ctx *bw, duk_size_t off, duk_size_t len) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw != NULL);
	DUK_ASSERT(off <= DUK_BW_GET_SIZE(thr, bw));

	DUK_BW_ENSURE(thr, bw, len);
	return duk_bw_insert_raw_area(thr, bw, off, len);
}

DUK_INTERNAL void duk_bw_remove_raw_slice(duk_hthread *thr, duk_bufwriter_ctx *bw, duk_size_t off, duk_size_t len) {
	duk_size_t move_sz;

	duk_uint8_t *p_base;
	duk_uint8_t *p_src;
	duk_uint8_t *p_dst;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw != NULL);
	DUK_ASSERT(off <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(len <= DUK_BW_GET_SIZE(thr, bw));
	DUK_ASSERT(off + len <= DUK_BW_GET_SIZE(thr, bw));
	DUK_UNREF(thr);

	p_base = bw->p_base;
	p_dst = p_base + off;
	p_src = p_dst + len;
	move_sz = (duk_size_t) (bw->p - p_src);
	duk_memmove_unsafe((void *) p_dst,
	                   (const void *) p_src,
	                   (size_t) move_sz);
	bw->p -= len;
}

/*
 *  Macro support functions for reading/writing raw data.
 *
 *  These are done using mempcy to ensure they're valid even for unaligned
 *  reads/writes on platforms where alignment counts.  On x86 at least gcc
 *  is able to compile these into a bswap+mov.  "Always inline" is used to
 *  ensure these macros compile to minimal code.
 *
 *  Not really bufwriter related, but currently used together.
 */

DUK_INTERNAL DUK_ALWAYS_INLINE duk_uint16_t duk_raw_read_u16_be(duk_uint8_t **p) {
	union {
		duk_uint8_t b[2];
		duk_uint16_t x;
	} u;

	duk_memcpy((void *) u.b, (const void *) (*p), (size_t) 2);
	u.x = DUK_NTOH16(u.x);
	*p += 2;
	return u.x;
}

DUK_INTERNAL DUK_ALWAYS_INLINE duk_uint32_t duk_raw_read_u32_be(duk_uint8_t **p) {
	union {
		duk_uint8_t b[4];
		duk_uint32_t x;
	} u;

	duk_memcpy((void *) u.b, (const void *) (*p), (size_t) 4);
	u.x = DUK_NTOH32(u.x);
	*p += 4;
	return u.x;
}

DUK_INTERNAL DUK_ALWAYS_INLINE duk_double_t duk_raw_read_double_be(duk_uint8_t **p) {
	duk_double_union du;
	union {
		duk_uint8_t b[4];
		duk_uint32_t x;
	} u;

	duk_memcpy((void *) u.b, (const void *) (*p), (size_t) 4);
	u.x = DUK_NTOH32(u.x);
	du.ui[DUK_DBL_IDX_UI0] = u.x;
	duk_memcpy((void *) u.b, (const void *) (*p + 4), (size_t) 4);
	u.x = DUK_NTOH32(u.x);
	du.ui[DUK_DBL_IDX_UI1] = u.x;
	*p += 8;

	return du.d;
}

DUK_INTERNAL DUK_ALWAYS_INLINE void duk_raw_write_u16_be(duk_uint8_t **p, duk_uint16_t val) {
	union {
		duk_uint8_t b[2];
		duk_uint16_t x;
	} u;

	u.x = DUK_HTON16(val);
	duk_memcpy((void *) (*p), (const void *) u.b, (size_t) 2);
	*p += 2;
}

DUK_INTERNAL DUK_ALWAYS_INLINE void duk_raw_write_u32_be(duk_uint8_t **p, duk_uint32_t val) {
	union {
		duk_uint8_t b[4];
		duk_uint32_t x;
	} u;

	u.x = DUK_HTON32(val);
	duk_memcpy((void *) (*p), (const void *) u.b, (size_t) 4);
	*p += 4;
}

DUK_INTERNAL DUK_ALWAYS_INLINE void duk_raw_write_double_be(duk_uint8_t **p, duk_double_t val) {
	duk_double_union du;
	union {
		duk_uint8_t b[4];
		duk_uint32_t x;
	} u;

	du.d = val;
	u.x = du.ui[DUK_DBL_IDX_UI0];
	u.x = DUK_HTON32(u.x);
	duk_memcpy((void *) (*p), (const void *) u.b, (size_t) 4);
	u.x = du.ui[DUK_DBL_IDX_UI1];
	u.x = DUK_HTON32(u.x);
	duk_memcpy((void *) (*p + 4), (const void *) u.b, (size_t) 4);
	*p += 8;
}
