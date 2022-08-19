/*
 *  CBOR bindings for Duktape.
 *
 *  https://tools.ietf.org/html/rfc7049
 */

#include <math.h>
#include "duktape.h"
#include "duk_cbor.h"

#undef DUK_CBOR_DPRINT

typedef struct {
	duk_context *ctx;
	duk_idx_t idx_buf;
	duk_size_t off;
	duk_size_t len;
} duk_cbor_encode_context;

typedef struct {
	duk_context *ctx;
	const duk_uint8_t *buf;
	duk_size_t off;
	duk_size_t len;
} duk_cbor_decode_context;

typedef union {
	duk_uint8_t x[8];
	duk_uint16_t s[4];
	duk_uint32_t i[2];
	double d;
} duk_cbor_dblunion;

typedef union {
	float f;
	duk_uint8_t x[4];
} duk_cbor_fltunion;

/*
 *  Misc
 */

/* Endian detection.  Technically happens at runtime, but in practice
 * resolves at compile time to a true/false value.
 */
static int duk__cbor_is_little_endian(void) {
	duk_cbor_dblunion u;

	/* >>> struct.pack('>d', 1.23456789).encode('hex')
	 * '3ff3c0ca4283de1b'
	 */

	u.d = 1.23456789;
	if (u.x[0] == 0x1bU) {
		return 1;
	}
	return 0;
}

static void duk__cbor_bswap4(duk_uint8_t *x) {
	int i;

	for (i = 0; i < 2; i++) {
		duk_uint8_t t;

		t = x[i]; x[i] = x[3 - i]; x[3 - i] = t;
	}
}

static void duk__cbor_bswap8(duk_uint8_t *x) {
	int i;

	for (i = 0; i < 4; i++) {
		duk_uint8_t t;

		t = x[i]; x[i] = x[7 - i]; x[7 - i] = t;
	}
}

/*
 *  Encoding
 */

static void duk__cbor_encode_error(duk_cbor_encode_context *enc_ctx) {
	(void) duk_type_error(enc_ctx->ctx, "cbor encode error");
}

/* Check that a size_t is in uint32 range to avoid out-of-range casts. */
static void duk__cbor_encode_sizet_uint32_check(duk_cbor_encode_context *enc_ctx, duk_size_t len) {
	if (sizeof(duk_size_t) > sizeof(duk_uint32_t) && len > (duk_size_t) DUK_UINT32_MAX) {
		duk__cbor_encode_error(enc_ctx);
	}
}

static duk_uint8_t *duk__cbor_encode_reserve(duk_cbor_encode_context *enc_ctx, duk_size_t len) {
	duk_uint8_t *res;
	duk_size_t ignlen;

	for (;;) {
		duk_size_t tmp;
		duk_size_t newlen;

		tmp = enc_ctx->off + len;
		if (tmp < len) {
			duk__cbor_encode_error(enc_ctx);  /* Overflow. */
		}
		if (tmp <= enc_ctx->len) {
			break;
		}
		newlen = enc_ctx->len * 2U;
		if (newlen < enc_ctx->len) {
			duk__cbor_encode_error(enc_ctx);  /* Overflow. */
		}
#if defined(DUK_CBOR_DPRINT)
		fprintf(stderr, "resize to %ld\n", (long) newlen);
#endif
		duk_resize_buffer(enc_ctx->ctx, enc_ctx->idx_buf, newlen);
		enc_ctx->len = newlen;
	}
	res = (duk_uint8_t *) duk_require_buffer(enc_ctx->ctx, enc_ctx->idx_buf, &ignlen) + enc_ctx->off;
	enc_ctx->off += len;
	return res;
}

static void duk__cbor_encode_overwritebyte(duk_cbor_encode_context *enc_ctx, duk_size_t off, duk_uint8_t val) {
	duk_uint8_t *res;
	size_t len;

	res = (duk_uint8_t *) duk_require_buffer(enc_ctx->ctx, enc_ctx->idx_buf, &len);
	if (off >= len) {
		duk__cbor_encode_error(enc_ctx);
	}
	res[off] = val;
}

static void duk__cbor_encode_emitbyte(duk_cbor_encode_context *enc_ctx, duk_uint8_t val) {
	duk_uint8_t *p;

	p = duk__cbor_encode_reserve(enc_ctx, 1);
	*p = val;
}

static void duk__cbor_encode_uint32(duk_cbor_encode_context *enc_ctx, duk_uint32_t u, duk_uint8_t base) {
	duk_uint8_t *p;

	if (u <= 23UL) {
		duk__cbor_encode_emitbyte(enc_ctx, (duk_uint8_t) (base + (duk_uint8_t) u));
	} else if (u <= 0xffUL) {
		p = duk__cbor_encode_reserve(enc_ctx, 1 + 1);
		*p++ = base + 0x18U;
		*p++ = (duk_uint8_t) u;
	} else if (u <= 0xffffUL) {
		p = duk__cbor_encode_reserve(enc_ctx, 1 + 2);
		*p++ = base + 0x19U;
		*p++ = (duk_uint8_t) ((u >> 8) & 0xffU);
		*p++ = (duk_uint8_t) (u & 0xffU);
	} else {
		p = duk__cbor_encode_reserve(enc_ctx, 1 + 4);
		*p++ = base + 0x1aU;
		*p++ = (duk_uint8_t) ((u >> 24) & 0xffU);
		*p++ = (duk_uint8_t) ((u >> 16) & 0xffU);
		*p++ = (duk_uint8_t) ((u >> 8) & 0xffU);
		*p++ = (duk_uint8_t) (u & 0xffU);
	}
}

static void duk__cbor_encode_double(duk_cbor_encode_context *enc_ctx, double d) {
	duk_uint8_t *p;
	duk_cbor_dblunion u;

	/* Integers and floating point values of all types are conceptually
	 * equivalent in CBOR.  Try to always choose the shortest encoding
	 * which is not always immediately obvious.  For example, NaN and Inf
	 * can be most compactly represented as a half-float (assuming NaN
	 * bits are not preserved), and 0x1'0000'0000 as a single precision
	 * float.  Shortest forms in preference order (prefer integer over
	 * float when equal length):
	 *
	 *   uint        1 byte    [0,23] (not -0)
	 *   sint        1 byte    [-24,-1]
	 *   uint+1      2 bytes   [24,255]
	 *   sint+1      2 bytes   [-256,-25]
	 *   uint+2      3 bytes   [256,65535]
	 *   sint+2      3 bytes   [-65536,-257]
	 *   half-float  3 bytes   -0, NaN, +/- Infinity, range [-65504,65504]
	 *   uint+4      5 bytes   [65536,4294967295]
	 *   sint+4      5 bytes   [-4294967296,-258]
	 *   float       5 bytes   range [-(1 - 2^(-24)) * 2^128, (1 - 2^(-24)) * 2^128]
	 *   uint+8      9 bytes   [4294967296,18446744073709551615]
	 *   sint+8      9 bytes   [-18446744073709551616,-4294967297]
	 *   double      9 bytes
	 *
	 * For whole numbers (compatible with integers):
	 *   - 1-byte or 2-byte uint/sint representation is preferred for
	 *     [-256,255].
	 *   - 3-byte uint/sint is preferred for [-65536,65535].  Half floats
	 *     are never preferred because they have the same length.
	 *   - 5-byte uint/sint is preferred for [-4294967296,4294967295].
	 *     Single precision floats are never preferred, and half-floats
	 *     don't each above the 3-byte uint/sint range so they're never
	 *     preferred.
	 *   - So, for all integers up to signed/unsigned 32-bit range the
	 *     preferred encoding is always an integer uint/sint.
	 *   - For integers above 32 bits the situation is more complicated.
	 *     Half-floats are never useful for them because of their limited
	 *     range, but IEEE single precision floats (5 bytes encoded) can
	 *     represent some integers between the 32-bit and 64-bit ranges
	 *     which require 9 bytes as a uint/sint.
	 *
	 * For floating point values not compatible with integers, the
	 * preferred encoding is quite clear:
	 *   - For +Inf/-Inf use half-float.
	 *   - For NaN use a half-float, assuming NaN bits ("payload") is
	 *     not worth preserving.  Duktape doesn't in general guarantee
	 *     preservation of the NaN payload so using a half-float seems
	 *     consistent with that.
	 *   - For remaining values, prefer the shortest form which doesn't
	 *     lose any precision.  For normal half-floats and single precision
	 *     floats this is simple: just check exponent and mantissa bits
	 *     using a fixed mask.  For denormal half-floats and single
	 *     precision floats the check is a bit more complicated: a normal
	 *     IEEE double can sometimes be represented as a denormal
	 *     half-float or single precision float.
	 *
	 * https://en.wikipedia.org/wiki/Half-precision_floating-point_format#IEEE_754_half-precision_binary_floating-point_format:_binary16
	 */

	u.d = d;

	if (isfinite(d)) {
		double d_floor = floor(d);
		duk_int16_t exp;

		if (d_floor == d) {
			if (d == 0.0) {
				if (signbit(d)) {
					/* Shortest -0 is using half-float. */
					p = duk__cbor_encode_reserve(enc_ctx, 1 + 2);
					*p++ = 0xf9U;
					*p++ = 0x80U;
					*p++ = 0x00U;
				} else {
					duk__cbor_encode_emitbyte(enc_ctx, 0x00U);
				}
				return;
			} else if (d > 0.0) {
				if (d <= 4294967295.0) {
					duk__cbor_encode_uint32(enc_ctx, (duk_uint32_t) d, 0x00U);
					return;
				}
			} else {
				if (d >= -4294967296.0) {
					duk__cbor_encode_uint32(enc_ctx, (duk_uint32_t) (-1.0 - d), 0x20U);
					return;
				}
			}
			/* 64-bit integers are not supported at present.  So
			 * we also don't need to deal with choosing between a
			 * 64-bit uint/sint representation vs. IEEE double or
			 * float.
			 */
		}

		/* Check if 'd' can represented as a normal half-float.
		 * Denormal half-floats could also be used, but that check
		 * isn't done now (denormal half-floats are decoded of course).
		 * So just check exponent range and that at most 10 significant
		 * bits (excluding implicit leading 1) are used in 'd'.
		 */
		if (duk__cbor_is_little_endian()) {
			exp = (duk_int16_t) ((u.s[3] & 0x7ff0U) >> 4) - 1023;
		} else {
			exp = (duk_int16_t) ((u.s[0] & 0x7ff0U) >> 4) - 1023;
		}

		if (exp >= -14 && exp <= 15) {  /* half-float normal exponents (excl. denormals) */
			/* double: seeeeeee eeeemmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm */
			/* half:         seeeee mmmm mmmmmm00 00000000 00000000 00000000 00000000 00000000 */
			int use_half_float;

			if (duk__cbor_is_little_endian()) {
				use_half_float =
				    (u.x[0] == 0 && u.x[1] == 0 && u.x[2] == 0 && u.x[3] == 0 &&
				     u.x[4] == 0 && (u.x[5] & 0x03U) == 0);
			} else {
				use_half_float =
				    (u.x[7] == 0 && u.x[6] == 0 && u.x[5] == 0 && u.x[4] == 0 &&
				     u.x[3] == 0 && (u.x[2] & 0x03U) == 0);
			}
			if (use_half_float) {
				duk_uint32_t t;

				p = duk__cbor_encode_reserve(enc_ctx, 1 + 2);

				exp += 15;
				if (duk__cbor_is_little_endian()) {
					t = (duk_uint32_t) (u.x[7] & 0x80U) << 8;
					t += (duk_uint32_t) exp << 10;
					t += ((duk_uint32_t) u.x[6] & 0x0fU) << 6;
					t += ((duk_uint32_t) u.x[5]) >> 2;
				} else {
					t = (duk_uint32_t) (u.x[0] & 0x80U) << 8;
					t += (duk_uint32_t) exp << 10;
					t += ((duk_uint32_t) u.x[1] & 0x0fU) << 6;
					t += ((duk_uint32_t) u.x[2]) >> 2;
				}

				/* seeeeemm mmmmmmmm */
				*p++ = 0xf9U;
				*p++ = (t >> 8) & 0xffU;
				*p++ = (t >> 0) & 0xffU;
				return;
			}
		}

		/* Same check for plain float.  Also no denormal support here. */
		if (exp >= -126 && exp <= 127) {  /* float normal exponents (excl. denormals) */
			/* double: seeeeeee eeeemmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm */
			/* float:     seeee eeeemmmm mmmmmmmm mmmmmmmm mmm00000 00000000 00000000 00000000 */
			int use_float;

			/* We could do this explicit mantissa check, but doing
			 * a double-float-double cast is fine because we've
			 * already verified that the exponent is in range so
			 * that the narrower cast is not undefined behavior.
			 */
#if 0
			if (duk__cbor_is_little_endian()) {
				use_float =
				    (u.x[0] == 0 && u.x[1] == 0 && u.x[2] == 0 && (u.x[3] & 0xe0U) == 0);
			} else {
				use_float =
				    (u.x[7] == 0 && u.x[6] == 0 && u.x[5] == 0 && (u.x[4] & 0xe0U) == 0);
			}
#endif
			use_float = ((duk_double_t) (duk_float_t) d == d);
			if (use_float) {
				duk_uint32_t t;

				p = duk__cbor_encode_reserve(enc_ctx, 1 + 4);

				exp += 127;
				if (duk__cbor_is_little_endian()) {
					t = (duk_uint32_t) (u.x[7] & 0x80U) << 24;
					t += (duk_uint32_t) exp << 23;
					t += ((duk_uint32_t) u.x[6] & 0x0fU) << 19;
					t += ((duk_uint32_t) u.x[5]) << 11;
					t += ((duk_uint32_t) u.x[4]) << 3;
					t += ((duk_uint32_t) u.x[3]) >> 5;
				} else {
					t = (duk_uint32_t) (u.x[0] & 0x80U) << 24;
					t += (duk_uint32_t) exp << 23;
					t += ((duk_uint32_t) u.x[1] & 0x0fU) << 19;
					t += ((duk_uint32_t) u.x[2]) << 11;
					t += ((duk_uint32_t) u.x[3]) << 3;
					t += ((duk_uint32_t) u.x[4]) >> 5;
				}

				/* seeeeeee emmmmmmm mmmmmmmm mmmmmmmm */
				*p++ = 0xfaU;
				*p++ = (t >> 24) & 0xffU;
				*p++ = (t >> 16) & 0xffU;
				*p++ = (t >> 8) & 0xffU;
				*p++ = (t >> 0) & 0xffU;
				return;
			}
		}

		/* Cannot use half-float or float, encode as full IEEE double. */
		p = duk__cbor_encode_reserve(enc_ctx, 1 + 8);
		*p++ = 0xfbU;
		if (duk__cbor_is_little_endian()) {
			*p++ = u.x[7];
			*p++ = u.x[6];
			*p++ = u.x[5];
			*p++ = u.x[4];
			*p++ = u.x[3];
			*p++ = u.x[2];
			*p++ = u.x[1];
			*p++ = u.x[0];
		} else {
			*p++ = u.x[0];
			*p++ = u.x[1];
			*p++ = u.x[2];
			*p++ = u.x[3];
			*p++ = u.x[4];
			*p++ = u.x[5];
			*p++ = u.x[6];
			*p++ = u.x[7];
		}
	} else if (isinf(d)) {
		/* Shortest +/- Infinity encoding is using a half-float. */
		p = duk__cbor_encode_reserve(enc_ctx, 1 + 2);
		*p++ = 0xf9U;
		if (signbit(d)) {
			*p++ = 0xfcU;
		} else {
			*p++ = 0x7cU;
		}
		*p++ = 0x00U;
	} else {
		/* Shortest NaN encoding is using a half-float.  Lose the
		 * exact NaN bits in the process.  IEEE double would be
		 * 7ff8 0000 0000 0000, i.e. a quiet NaN in most architectures
		 * (https://en.wikipedia.org/wiki/NaN#Encoding).  The
		 * equivalent half float is 7e00.
		 */
		p = duk__cbor_encode_reserve(enc_ctx, 1 + 2);
		*p++ = 0xf9U;
		*p++ = 0x7eU;
		*p++ = 0x00U;
	}
}

static void duk__cbor_encode_string_top(duk_cbor_encode_context *enc_ctx) {
	const duk_uint8_t *str;
	duk_size_t len;
	duk_uint8_t *p;

	/* XXX: String data should be transcoded to UTF-8, surrogate pairs
	 * combined and invalid surrogates replaced with U+FFFD.  Internal
	 * representation is used as is for now.
	 */
	str = (const duk_uint8_t *) duk_require_lstring(enc_ctx->ctx, -1, &len);
	duk__cbor_encode_sizet_uint32_check(enc_ctx, len);
	duk__cbor_encode_uint32(enc_ctx, (duk_uint32_t) len, 0x60U);
	p = duk__cbor_encode_reserve(enc_ctx, len);
	(void) memcpy((void *) p, (const void *) str, len);
}

static void duk__cbor_encode_value(duk_cbor_encode_context *enc_ctx) {
	/* XXX: How to avoid losing information in an encode/decode cycle?
	 * Type tags would be enough to provide knowledge that a value is
	 * e.g. a pointer, but the tag space doesn't support non-registered
	 * custom values (all values are assigned by IANA).
	 */
	duk_uint8_t *buf;
	duk_size_t len;
	duk_uint8_t *p;

	/* When working with deeply recursive structures, this is important
	 * to ensure there's no effective depth limit.
	 */
	duk_require_stack(enc_ctx->ctx, 4);

	switch (duk_get_type(enc_ctx->ctx, -1)) {
	case DUK_TYPE_UNDEFINED:
		duk__cbor_encode_emitbyte(enc_ctx, 0xf7U);
		break;
	case DUK_TYPE_NULL:
		duk__cbor_encode_emitbyte(enc_ctx, 0xf6U);
		break;
	case DUK_TYPE_BOOLEAN:
		duk__cbor_encode_emitbyte(enc_ctx, duk_get_boolean(enc_ctx->ctx, -1) ?
		                                   0xf5U : 0xf4U);
		break;
	case DUK_TYPE_NUMBER:
		duk__cbor_encode_double(enc_ctx, duk_get_number(enc_ctx->ctx, -1));
		break;
	case DUK_TYPE_STRING:
		duk__cbor_encode_string_top(enc_ctx);
		break;
	case DUK_TYPE_OBJECT:
		/* XXX: Date support */
		if (duk_is_array(enc_ctx->ctx, -1)) {
			duk_size_t i;
			len = duk_get_length(enc_ctx->ctx, -1);
			duk__cbor_encode_sizet_uint32_check(enc_ctx, len);
			duk__cbor_encode_uint32(enc_ctx, (duk_uint32_t) len, 0x80U);
			for (i = 0; i < len; i++) {
				duk_get_prop_index(enc_ctx->ctx, -1, (duk_uarridx_t) i);
				duk__cbor_encode_value(enc_ctx);
			}
		} else if (duk_is_buffer_data(enc_ctx->ctx, -1)) {
			/* Tag buffer data? */
			buf = (duk_uint8_t *) duk_require_buffer_data(enc_ctx->ctx, -1, &len);
			duk__cbor_encode_sizet_uint32_check(enc_ctx, len);
			duk__cbor_encode_uint32(enc_ctx, (duk_uint32_t) len, 0x40U);
			p = duk__cbor_encode_reserve(enc_ctx, len);
			(void) memcpy((void *) p, (const void *) buf, len);
		} else {
			/* We don't know the number of properties in advance
			 * but would still like to encode at least small
			 * objects without indefinite length.  Emit an
			 * indefinite length byte initially, and if the final
			 * property count is small enough to also fit in one
			 * byte, backpatch it later.  Otherwise keep the
			 * indefinite length.  This works well up to 23
			 * properties which is practical and good enough.
			 */
			duk_size_t off_ib = enc_ctx->off;
			duk_uint32_t count = 0U;
			duk__cbor_encode_emitbyte(enc_ctx, 0xa0U + 0x1fU);  /* indefinite length */
			duk_enum(enc_ctx->ctx, -1, DUK_ENUM_OWN_PROPERTIES_ONLY);
			while (duk_next(enc_ctx->ctx, -1, 1 /*get_value*/)) {
				duk_insert(enc_ctx->ctx, -2);  /* [ ... key value ] -> [ ... value key ] */
				duk__cbor_encode_value(enc_ctx);
				duk__cbor_encode_value(enc_ctx);
				count++;
				if (count == 0U) {
					duk__cbor_encode_error(enc_ctx);
				}
			}
			duk_pop(enc_ctx->ctx);
			if (count <= 0x17U) {
				duk__cbor_encode_overwritebyte(enc_ctx, off_ib, (duk_size_t) (0xa0U + count));
			} else {
				duk__cbor_encode_emitbyte(enc_ctx, 0xffU);  /* break */
			}
		}
		break;
	case DUK_TYPE_BUFFER:
		/* Tag buffer data? */
		buf = (duk_uint8_t *) duk_require_buffer(enc_ctx->ctx, -1, &len);
		duk__cbor_encode_sizet_uint32_check(enc_ctx, len);
		duk__cbor_encode_uint32(enc_ctx, (duk_uint32_t) len, 0x40U);
		p = duk__cbor_encode_reserve(enc_ctx, len);
		(void) memcpy((void *) p, (const void *) buf, len);
		break;
	case DUK_TYPE_POINTER:
		/* Pointers (void *) are challenging to encode.  They can't
		 * be relied to be even 64-bit integer compatible (there are
		 * pointer models larger than that), nor can floats encode
		 * them.  They could be encoded as strings (%p format), or
		 * as direct memory representations.  Recovering pointers is
		 * non-portable in any case but it would be nice to be able
		 * to recover compatible pointers.
		 *
		 * For now, encode as %p string.  There doesn't seem to be an
		 * appropriate tag, so pointers don't currently survive a CBOR
		 * encode/decode roundtrip intact.
		 */
		duk_to_string(enc_ctx->ctx, -1);
		duk__cbor_encode_string_top(enc_ctx);
		break;
	case DUK_TYPE_LIGHTFUNC:
		/* For now encode as an empty object. */
		duk__cbor_encode_emitbyte(enc_ctx, 0xa0U + 0);  /* zero-length */
		break;
	case DUK_TYPE_NONE:
	default:
		goto fail;
	}
	duk_pop(enc_ctx->ctx);
	return;

 fail:
	duk__cbor_encode_error(enc_ctx);
}

/*
 *  Decoding
 */

static void duk__cbor_decode_error(duk_cbor_decode_context *dec_ctx) {
	(void) duk_type_error(dec_ctx->ctx, "cbor decode error");
}

static duk_uint8_t duk__cbor_decode_readbyte(duk_cbor_decode_context *dec_ctx) {
	if (dec_ctx->off >= dec_ctx->len) {
		duk__cbor_decode_error(dec_ctx);
	}
	return dec_ctx->buf[dec_ctx->off++];
}

static duk_uint8_t duk__cbor_decode_peekbyte(duk_cbor_decode_context *dec_ctx) {
	if (dec_ctx->off >= dec_ctx->len) {
		duk__cbor_decode_error(dec_ctx);
	}
	return dec_ctx->buf[dec_ctx->off];
}

static void duk__cbor_decode_rewind(duk_cbor_decode_context *dec_ctx, duk_size_t len) {
	if (len > dec_ctx->off) {
		duk__cbor_decode_error(dec_ctx);
	}
	dec_ctx->off -= len;
}

#if 0
static void duk__cbor_decode_ensure(duk_cbor_decode_context *dec_ctx, duk_size_t len) {
	if (dec_ctx->off + len > dec_ctx->len) {
		duk__cbor_decode_error(dec_ctx);
	}
}
#endif

static const duk_uint8_t *duk__cbor_decode_consume(duk_cbor_decode_context *dec_ctx, duk_size_t len) {
	const duk_uint8_t *res;
	duk_size_t tmp;

	tmp = dec_ctx->off + len;
	if (tmp < len) {
		duk__cbor_decode_error(dec_ctx);  /* Overflow. */
	}
	if (tmp > dec_ctx->len) {
		duk__cbor_decode_error(dec_ctx);  /* Not enough input. */
	}
	res = dec_ctx->buf + dec_ctx->off;
	dec_ctx->off += len;
	return res;
}

static int duk__cbor_decode_checkbreak(duk_cbor_decode_context *dec_ctx) {
	if (duk__cbor_decode_peekbyte(dec_ctx) == 0xffU) {
		(void) duk__cbor_decode_readbyte(dec_ctx);
		return 1;
	}
	return 0;
}

static duk_double_t duk__cbor_decode_aival_uint(duk_cbor_decode_context *dec_ctx, duk_uint8_t ib) {
	duk_uint8_t ai;
	duk_uint32_t t, t2;

	ai = ib & 0x1fU;
	if (ai <= 0x17U) {
		return (duk_double_t) ai;
	}

	switch (ai) {
	case 0x18U:  /* 1 byte */
		return (duk_double_t) duk__cbor_decode_readbyte(dec_ctx);
	case 0x19U:  /* 2 byte */
		t = ((duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx)) << 8U;
		t += (duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx);
		return (duk_double_t) t;
	case 0x1aU:  /* 4 byte */
		t = ((duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx)) << 24U;
		t += ((duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx)) << 16U;
		t += ((duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx)) << 8U;
		t += (duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx);
		return (duk_double_t) t;
	case 0x1bU:  /* 8 byte */
		t = ((duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx)) << 24U;
		t += ((duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx)) << 16U;
		t += ((duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx)) << 8U;
		t += (duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx);
		t2 = t;
		t = ((duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx)) << 24U;
		t += ((duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx)) << 16U;
		t += ((duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx)) << 8U;
		t += (duk_uint32_t) duk__cbor_decode_readbyte(dec_ctx);
		return (duk_double_t) t2 * 4294967296.0 + (duk_double_t) t;
	}

	duk__cbor_decode_error(dec_ctx);
	return 0.0;
}

static duk_uint32_t duk__cbor_decode_aival_uint32(duk_cbor_decode_context *dec_ctx, duk_uint8_t ib) {
	duk_double_t t;

	t = duk__cbor_decode_aival_uint(dec_ctx, ib);
	if (t >= 4294967296.0) {
		duk__cbor_decode_error(dec_ctx);
	}
	return (duk_uint32_t) t;
}

static void duk__cbor_decode_buffer(duk_cbor_decode_context *dec_ctx, duk_uint8_t expected_base) {
	duk_uint32_t len;
	duk_uint8_t *buf;
	const duk_uint8_t *inp;
	duk_uint8_t ib;

	ib = duk__cbor_decode_readbyte(dec_ctx);
	if ((ib & 0xe0U) != expected_base) {
		duk__cbor_decode_error(dec_ctx);
	}
	/* indefinite format is rejected by the following */
	len = duk__cbor_decode_aival_uint32(dec_ctx, ib);
	inp = duk__cbor_decode_consume(dec_ctx, len);
	buf = (duk_uint8_t *) duk_push_fixed_buffer(dec_ctx->ctx, (duk_size_t) len);
	(void) memcpy((void *) buf, (const void *) inp, (size_t) len);
}

static void duk__cbor_decode_join_buffers(duk_cbor_decode_context *dec_ctx, duk_idx_t count) {
	duk_size_t total_size = 0;
	duk_idx_t top = duk_get_top(dec_ctx->ctx);
	duk_idx_t base = top - count;
	duk_idx_t idx;
	duk_uint8_t *p = NULL;

	for (;;) {
		for (idx = base; idx < top; idx++) {
			duk_uint8_t *buf_data;
			duk_size_t buf_size;

			buf_data = (duk_uint8_t *) duk_require_buffer(dec_ctx->ctx, idx, &buf_size);
			if (p != NULL) {
				if (buf_size > 0U) {
					(void) memcpy((void *) p, (const void *) buf_data, buf_size);
				}
				p += buf_size;
			} else {
				total_size += buf_size;
				if (total_size < buf_size) {
					duk__cbor_decode_error(dec_ctx);
				}
			}
		}

		if (p != NULL) {
			break;
		} else {
			p = (duk_uint8_t *) duk_push_fixed_buffer(dec_ctx->ctx, total_size);
		}
	}

	duk_replace(dec_ctx->ctx, base);
	duk_pop_n(dec_ctx->ctx, count - 1);
}

static void duk__cbor_decode_and_join_strbuf(duk_cbor_decode_context *dec_ctx, duk_uint8_t expected_base) {
	duk_idx_t count = 0;
	for (;;) {
		if (duk__cbor_decode_checkbreak(dec_ctx)) {
			break;
		}
		duk__cbor_decode_buffer(dec_ctx, expected_base);
		count++;
		if (count <= 0) {
			duk__cbor_decode_error(dec_ctx);
		}
	}
	if (count == 0) {
		(void) duk_push_fixed_buffer(dec_ctx->ctx, 0);
	} else if (count > 1) {
		duk__cbor_decode_join_buffers(dec_ctx, count);
	}
}

static duk_double_t duk__cbor_decode_half_float(duk_cbor_decode_context *dec_ctx) {
	duk_cbor_dblunion u;
	const duk_uint8_t *inp;
	duk_int_t exp;
	duk_uint_t u16;
	duk_uint_t tmp;
	duk_double_t res;

	inp = duk__cbor_decode_consume(dec_ctx, 2);
	u16 = ((duk_uint_t) inp[0] << 8) + (duk_uint_t) inp[1];
	exp = (duk_int_t) ((u16 >> 10) & 0x1fU) - 15;

	if (exp == -15) {
		/* Zero or denormal; but note that half float
		 * denormals become double normals.
		 */
		if ((u16 & 0x03ffU) == 0) {
			u.x[0] = inp[0] & 0x80U;
			u.x[1] = 0U;
			u.x[2] = 0U;
			u.x[3] = 0U;
			u.x[4] = 0U;
			u.x[5] = 0U;
			u.x[6] = 0U;
			u.x[7] = 0U;
		} else {
			/* Create denormal by first creating a double that
			 * contains the denormal bits and a leading implicit
			 * 1-bit.  Then subtract away the implicit 1-bit.
			 *
			 *    0.mmmmmmmmmm * 2^-14
			 *    1.mmmmmmmmmm 0.... * 2^-14
			 *   -1.0000000000 0.... * 2^-14
			 *
			 * Double exponent: -14 + 1023 = 0x3f1
			 */
			u.x[0] = 0x3fU;
			u.x[1] = 0x10U + (duk_uint8_t) ((u16 >> 6) & 0x0fU);
			u.x[2] = (duk_uint8_t) ((u16 << 2) & 0xffU);  /* Mask is really 0xfcU */
			u.x[3] = 0U;
			u.x[4] = 0U;
			u.x[5] = 0U;
			u.x[6] = 0U;
			u.x[7] = 0U;
			if (duk__cbor_is_little_endian()) {
				duk__cbor_bswap8(u.x);
			}
			res = u.d - 0.00006103515625;  /* 2^(-14) */
			if (u16 & 0x8000U) {
				res = -res;
			}
			return res;
		}
	} else if (exp == 16) {
		/* +/- Inf or NaN. */
		if ((u16 & 0x03ffU) == 0) {
			u.x[0] = (inp[0] & 0x80U) + 0x7fU;
			u.x[1] = 0xf0U;
			u.x[2] = 0U;
			u.x[3] = 0U;
			u.x[4] = 0U;
			u.x[5] = 0U;
			u.x[6] = 0U;
			u.x[7] = 0U;
		} else {
			/* Create a 'quiet NaN' with highest
			 * bit set (there are some platforms
			 * where the NaN payload convention is
			 * the opposite).  Keep sign.
			 */
			u.x[0] = (inp[0] & 0x80U) + 0x7fU;
			u.x[1] = 0xf8U;
			u.x[2] = 0U;
			u.x[3] = 0U;
			u.x[4] = 0U;
			u.x[5] = 0U;
			u.x[6] = 0U;
			u.x[7] = 0U;
		}
	} else {
		/* Normal. */
		tmp = (inp[0] & 0x80U) ? 0x80000000UL : 0UL;
		tmp += (duk_uint_t) (exp + 1023) << 20;
		tmp += (duk_uint_t) (inp[0] & 0x03U) << 18;
		tmp += (duk_uint_t) (inp[1] & 0xffU) << 10;
		u.x[0] = (tmp >> 24) & 0xffU;
		u.x[1] = (tmp >> 16) & 0xffU;
		u.x[2] = (tmp >> 8) & 0xffU;
		u.x[3] = (tmp >> 0) & 0xffU;
		u.x[4] = 0U;
		u.x[5] = 0U;
		u.x[6] = 0U;
		u.x[7] = 0U;
	}
	if (duk__cbor_is_little_endian()) {
		duk__cbor_bswap8(u.x);
	}

	return u.d;
}

static void duk__cbor_decode_value(duk_cbor_decode_context *dec_ctx) {
	duk_uint8_t ib, mt, ai;

	/* When working with deeply recursive structures, this is important
	 * to ensure there's no effective depth limit.
	 */
	duk_require_stack(dec_ctx->ctx, 4);

 reread_initial_byte:
#if defined(DUK_CBOR_DPRINT)
	fprintf(stderr, "decode off=%ld len=%ld\n", (long) dec_ctx->off, (long) dec_ctx->len);
#endif

	ib = duk__cbor_decode_readbyte(dec_ctx);
	mt = ib >> 5U;
	ai = ib & 0x1fU;

	/* Additional information in [24,27] = [0x18,0x1b] has relatively
	 * uniform handling for all major types: read 1/2/4/8 additional
	 * bytes.  For major type 7 the 1-byte value is a 'simple type', and
	 * 2/4/8-byte values are floats.  For other major types the 1/2/4/8
	 * byte values are integers.  The lengths are uniform, but the typing
	 * is not.
	 */

	switch (mt) {
	case 0U: {  /* unsigned integer */
		duk_push_number(dec_ctx->ctx, duk__cbor_decode_aival_uint(dec_ctx, ib));
		break;
	}
	case 1U: {  /* negative integer */
		/* XXX: precision, for -1.0 and 64-bit integers */
		duk_push_number(dec_ctx->ctx, -1.0 - duk__cbor_decode_aival_uint(dec_ctx, ib));
		break;
	}
	case 2U: {  /* byte string */
		if (ai == 0x1fU) {
			duk__cbor_decode_and_join_strbuf(dec_ctx, 0x40U);
		} else {
			duk__cbor_decode_rewind(dec_ctx, 1U);
			duk__cbor_decode_buffer(dec_ctx, 0x40U);
		}
		break;
	}
	case 3U: {  /* text string */
		/* XXX: Text data should be validated to be UTF-8 on input,
		 * and then be converted to surrogate pairs etc.  Right now
		 * import as is, but reject strings that would create symbols.
		 */
		if (ai == 0x1fU) {
			duk_uint8_t *buf_data;
			duk_size_t buf_size;

			duk__cbor_decode_and_join_strbuf(dec_ctx, 0x60U);
			buf_data = (duk_uint8_t *) duk_require_buffer(dec_ctx->ctx, -1, &buf_size);
			(void) duk_push_lstring(dec_ctx->ctx, (const char *) buf_data, buf_size);
			duk_remove(dec_ctx->ctx, -2);
		} else {
			duk_uint32_t len;
			const duk_uint8_t *inp;

			len = duk__cbor_decode_aival_uint32(dec_ctx, ib);
			inp = duk__cbor_decode_consume(dec_ctx, len);
			(void) duk_push_lstring(dec_ctx->ctx, (const char *) inp, (duk_size_t) len);
		}
		/* XXX: Here a Duktape API call to convert input -> utf-8 with
		 * replacements would be nice.
		 */
		break;
	}
	case 4U: {  /* array of data items */
		duk_uint32_t idx, len;

		/* Support arrays up to 0xfffffffeU in length.  0xffffffff is
		 * used as an indefinite length marker.
		 */
		if (ai == 0x1fU) {
			len = 0xffffffffUL;
		} else {
			len = duk__cbor_decode_aival_uint32(dec_ctx, ib);
			if (len == 0xffffffffUL) {
				goto format_error;
			}
		}

		duk_push_array(dec_ctx->ctx);
		for (idx = 0U; ;) {
			if (len == 0xffffffffUL && duk__cbor_decode_checkbreak(dec_ctx)) {
				break;
			}
			if (idx == len) {
				if (ai == 0x1fU) {
					goto format_error;
				}
				break;
			}
			duk__cbor_decode_value(dec_ctx);
			duk_put_prop_index(dec_ctx->ctx, -2, (duk_uarridx_t) idx);
			idx++;
			if (idx == 0U) {
				goto format_error;  /* wrapped */
			}
		}
		break;
	}
	case 5U: {  /* map of pairs of data items */
		duk_uint32_t count;

		if (ai == 0x1fU) {
			count = 0xffffffffUL;
		} else {
			count = duk__cbor_decode_aival_uint32(dec_ctx, ib);
			if (count == 0xffffffffUL) {
				goto format_error;
			}
		}

		duk_push_object(dec_ctx->ctx);
		for (;;) {
			if (count == 0xffffffffUL) {
				if (duk__cbor_decode_checkbreak(dec_ctx)) {
					break;
				}
			} else {
				if (count == 0UL) {
					break;
				}
				count--;
			}

			/* Non-string keys are coerced to strings,
			 * possibly leading to overwriting previous
			 * keys.  Last key of a certain coerced name
			 * wins.  If key is an object, it will coerce
			 * to '[object Object]' which is consistent
			 * but potentially misleading.  One alternative
			 * would be to skip non-string keys.
			 */
			duk__cbor_decode_value(dec_ctx);
			duk__cbor_decode_value(dec_ctx);
			duk_put_prop(dec_ctx->ctx, -3);
		}
		break;
	}
	case 6U: {  /* semantic tagging */
		/* Tags are ignored now, re-read initial byte.  A tagged
		 * value may itself be tagged (an unlimited number of times)
		 * so keep on peeling away tags.
		 */
		(void) duk__cbor_decode_aival_uint32(dec_ctx, ib);
		goto reread_initial_byte;
	}
	case 7U: {  /* floating point numbers, simple data types, break; other */
		switch (ai) {
		case 0x14U: {
			duk_push_false(dec_ctx->ctx);
			break;
		}
		case 0x15U: {
			duk_push_true(dec_ctx->ctx);
			break;
		}
		case 0x16U: {
			duk_push_null(dec_ctx->ctx);
			break;
		}
		case 0x17U: {
			duk_push_undefined(dec_ctx->ctx);
			break;
		}
		case 0x18U: {  /* more simple values (1 byte) */
			goto format_error;  /* none defined so far, and 0-31 not allowed */
		}
		case 0x19U: {  /* half-float (2 bytes) */
			duk_double_t d;
			d = duk__cbor_decode_half_float(dec_ctx);
			duk_push_number(dec_ctx->ctx, d);
			break;
		}
		case 0x1aU: {  /* float (4 bytes) */
			duk_cbor_fltunion u;
			const duk_uint8_t *inp;
			inp = duk__cbor_decode_consume(dec_ctx, 4);
			(void) memcpy((void *) u.x, (const void *) inp, 4);
			if (duk__cbor_is_little_endian()) {
				duk__cbor_bswap4(u.x);
			}
			duk_push_number(dec_ctx->ctx, (duk_double_t) u.f);
			break;
		}
		case 0x1bU: {  /* double (8 bytes) */
			duk_cbor_dblunion u;
			const duk_uint8_t *inp;
			inp = duk__cbor_decode_consume(dec_ctx, 8);
			(void) memcpy((void *) u.x, (const void *) inp, 8);
			if (duk__cbor_is_little_endian()) {
				duk__cbor_bswap8(u.x);
			}
			duk_push_number(dec_ctx->ctx, u.d);
			break;
		}
		case 0xffU:  /* unexpected break */
		default: {
			goto format_error;
		}
		}  /* end switch */
		break;
	}
	default: {
		goto format_error;  /* will never actually occur */
	}
	}  /* end switch */

	return;

 format_error:
	duk__cbor_decode_error(dec_ctx);
}

/*
 *  Public APIs
 */

static duk_ret_t duk__cbor_encode_binding(duk_context *ctx) {
	/* Produce an ArrayBuffer by first decoding into a plain buffer which
	 * mimics a Uint8Array and gettings its .buffer property.
	 */
	duk_cbor_encode(ctx, -1, 0);
	duk_get_prop_string(ctx, -1, "buffer");
	return 1;
}

static duk_ret_t duk__cbor_decode_binding(duk_context *ctx) {
	/* Lenient: accept any buffer like. */
	duk_cbor_decode(ctx, -1, 0);
	return 1;
}

void duk_cbor_init(duk_context *ctx, duk_uint_t flags) {
	(void) flags;
	duk_push_global_object(ctx);
	duk_push_string(ctx, "CBOR");
	duk_push_object(ctx);
	duk_push_string(ctx, "encode");
	duk_push_c_function(ctx, duk__cbor_encode_binding, 1);
	duk_def_prop(ctx, -3, DUK_DEFPROP_ATTR_WC | DUK_DEFPROP_HAVE_VALUE);
	duk_push_string(ctx, "decode");
	duk_push_c_function(ctx, duk__cbor_decode_binding, 1);
	duk_def_prop(ctx, -3, DUK_DEFPROP_ATTR_WC | DUK_DEFPROP_HAVE_VALUE);
	duk_def_prop(ctx, -3, DUK_DEFPROP_ATTR_WC | DUK_DEFPROP_HAVE_VALUE);
	duk_pop(ctx);
}

void duk_cbor_encode(duk_context *ctx, duk_idx_t idx, duk_uint_t encode_flags) {
	duk_cbor_encode_context enc_ctx;

	(void) encode_flags;

	idx = duk_require_normalize_index(ctx, idx);

	enc_ctx.ctx = ctx;
	enc_ctx.idx_buf = duk_get_top(ctx);
	enc_ctx.off = 0;
	enc_ctx.len = 64;

	duk_push_dynamic_buffer(ctx, enc_ctx.len);
	duk_dup(ctx, idx);
	duk__cbor_encode_value(&enc_ctx);
	duk_resize_buffer(enc_ctx.ctx, enc_ctx.idx_buf, enc_ctx.off);
	duk_replace(ctx, idx);
}

void duk_cbor_decode(duk_context *ctx, duk_idx_t idx, duk_uint_t decode_flags) {
	duk_cbor_decode_context dec_ctx;

	(void) decode_flags;

	idx = duk_require_normalize_index(ctx, idx);

	dec_ctx.ctx = ctx;
	dec_ctx.buf = (const duk_uint8_t *) duk_require_buffer_data(ctx, idx, &dec_ctx.len);
	dec_ctx.off = 0;
	/* dec_ctx.len: set above */

	duk__cbor_decode_value(&dec_ctx);
	if (dec_ctx.off != dec_ctx.len) {
		(void) duk_type_error(ctx, "trailing garbage");
	}

	duk_replace(ctx, idx);
}
