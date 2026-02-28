#ifndef MB_WC_INCLUDED
#define MB_WC_INCLUDED

/* Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License, version 2.0, as published by the Free Software Foundation.

   This library is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the library and your derivative works with the
   separately licensed software that they have included with MySQL.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License, version 2.0, for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
   MA 02110-1301  USA */

/**
  @file mb_wc.h

  Definitions of mb_wc (multibyte to wide character, ie., effectively
  “parse a UTF-8 character”) functions for UTF-8 (both three- and four-byte).
  These are available both as inline functions, as C-style thunks so that they
  can fit into MY_CHARSET_HANDLER, and as functors.

  The functors exist so that you can specialize a class on them and get them
  inlined instead of having to call them through the function pointer in
  MY_CHARSET_HANDLER; mb_wc is in itself so cheap (the most common case is
  just a single byte load and a predictable compare) that the call overhead
  in a tight loop is significant, and these routines tend to take up a lot
  of CPU time when sorting. Typically, at the outermost level, you'd simply
  compare cs->cset->mb_wc with my_mb_wc_{utf8,utf8mb4}_thunk, and if so,
  instantiate your function with the given class. If it doesn't match,
  you can use Mb_wc_through_function_pointer, which calls through the
  function pointer as usual. (It will cache the function pointer for you,
  which is typically faster than looking it up all the time -- the compiler
  cannot always figure out on its own that it doesn't change.)

  The Mb_wc_* classes should be sent by _value_, not by reference, since
  they are never larger than two pointers (and usually simply zero).
*/

#include "m_ctype.h"
#include "my_compiler.h"
#include "my_config.h"

template <bool RANGE_CHECK, bool SUPPORT_MB4>
static int my_mb_wc_utf8_prototype(my_wc_t *pwc, const uchar *s,
                                   const uchar *e);

static int my_mb_wc_utf8(my_wc_t *pwc, const uchar *s, const uchar *e);
static int my_mb_wc_utf8mb4(my_wc_t *pwc, const uchar *s, const uchar *e);

/**
  Functor that converts a UTF-8 multibyte sequence (up to three bytes)
  to a wide character.
*/
struct Mb_wc_utf8 {
  Mb_wc_utf8() {}

  ALWAYS_INLINE
  int operator()(my_wc_t *pwc, const uchar *s, const uchar *e) const {
    return my_mb_wc_utf8(pwc, s, e);
  }
};

/**
  Functor that converts a UTF-8 multibyte sequence (up to four bytes)
  to a wide character.
*/
struct Mb_wc_utf8mb4 {
  Mb_wc_utf8mb4() {}

  ALWAYS_INLINE
  int operator()(my_wc_t *pwc, const uchar *s, const uchar *e) const {
    return my_mb_wc_utf8mb4(pwc, s, e);
  }
};

/**
  Functor that uses a function pointer to convert a multibyte sequence
  to a wide character.
*/
class Mb_wc_through_function_pointer {
 public:
  explicit Mb_wc_through_function_pointer(const CHARSET_INFO *cs)
      : m_funcptr(cs->cset->mb_wc), m_cs(cs) {}

  int operator()(my_wc_t *pwc, const uchar *s, const uchar *e) const {
    return m_funcptr(m_cs, pwc, s, e);
  }

 private:
  typedef int (*mbwc_func_t)(const CHARSET_INFO *, my_wc_t *, const uchar *,
                             const uchar *);

  const mbwc_func_t m_funcptr;
  const CHARSET_INFO *const m_cs;
};

template <bool RANGE_CHECK, bool SUPPORT_MB4>
static ALWAYS_INLINE int my_mb_wc_utf8_prototype(my_wc_t *pwc, const uchar *s,
                                                 const uchar *e) {
  if (RANGE_CHECK && s >= e) return MY_CS_TOOSMALL;

  uchar c = s[0];
  if (c < 0x80) {
    *pwc = c;
    return 1;
  }

  if (c < 0xe0) {
    if (c < 0xc2)  // Resulting code point would be less than 0x80.
      return MY_CS_ILSEQ;

    if (RANGE_CHECK && s + 2 > e) return MY_CS_TOOSMALL2;

    if ((s[1] & 0xc0) != 0x80)  // Next byte must be a continuation byte.
      return MY_CS_ILSEQ;

    *pwc = ((my_wc_t)(c & 0x1f) << 6) + (my_wc_t)(s[1] & 0x3f);
    return 2;
  }

  if (c < 0xf0) {
    if (RANGE_CHECK && s + 3 > e) return MY_CS_TOOSMALL3;

    // Next two bytes must be continuation bytes.
    uint16 two_bytes;
    memcpy(&two_bytes, s + 1, sizeof(two_bytes));
    if ((two_bytes & 0xc0c0) != 0x8080)  // Endianness does not matter.
      return MY_CS_ILSEQ;

    *pwc = ((my_wc_t)(c & 0x0f) << 12) + ((my_wc_t)(s[1] & 0x3f) << 6) +
           (my_wc_t)(s[2] & 0x3f);
    if (*pwc < 0x800) return MY_CS_ILSEQ;
    /*
      According to RFC 3629, UTF-8 should prohibit characters between
      U+D800 and U+DFFF, which are reserved for surrogate pairs and do
      not directly represent characters.
    */
    if (*pwc >= 0xd800 && *pwc <= 0xdfff) return MY_CS_ILSEQ;
    return 3;
  }

  if (SUPPORT_MB4) {
    if (RANGE_CHECK && s + 4 > e) /* We need 4 characters */
      return MY_CS_TOOSMALL4;

    /*
      This byte must be of the form 11110xxx, and the next three bytes
      must be continuation bytes.
    */
    uint32 four_bytes;
    memcpy(&four_bytes, s, sizeof(four_bytes));
#ifdef WORDS_BIGENDIAN
    if ((four_bytes & 0xf8c0c0c0) != 0xf0808080)
#else
    if ((four_bytes & 0xc0c0c0f8) != 0x808080f0)
#endif
      return MY_CS_ILSEQ;

    *pwc = ((my_wc_t)(c & 0x07) << 18) + ((my_wc_t)(s[1] & 0x3f) << 12) +
           ((my_wc_t)(s[2] & 0x3f) << 6) + (my_wc_t)(s[3] & 0x3f);
    if (*pwc < 0x10000 || *pwc > 0x10ffff) return MY_CS_ILSEQ;
    return 4;
  }

  return MY_CS_ILSEQ;
}

/**
  Parses a single UTF-8 character from a byte string.

  @param[out] pwc the parsed character, if any
  @param s the string to read from
  @param e the end of the string; will not read past this

  @return the number of bytes read from s, or a value <= 0 for failure
    (see m_ctype.h)
*/
static inline int my_mb_wc_utf8(my_wc_t *pwc, const uchar *s, const uchar *e) {
  return my_mb_wc_utf8_prototype</*RANGE_CHECK=*/true, /*SUPPORT_MB4=*/false>(
      pwc, s, e);
}

/**
  Parses a single UTF-8 character from a byte string. The difference
  between this and my_mb_wc_utf8 is that this function also can handle
  four-byte UTF-8 characters.

  @param[out] pwc the parsed character, if any
  @param s the string to read from
  @param e the end of the string; will not read past this

  @return the number of bytes read from s, or a value <= 0 for failure
    (see m_ctype.h)
*/
static ALWAYS_INLINE int my_mb_wc_utf8mb4(my_wc_t *pwc, const uchar *s,
                                          const uchar *e) {
  return my_mb_wc_utf8_prototype</*RANGE_CHECK=*/true, /*SUPPORT_MB4=*/true>(
      pwc, s, e);
}

// Non-inlined versions of the above. These are used as function pointers
// in MY_CHARSET_HANDLER structs, and you can compare againt them to see
// if using the Mb_wc_utf8* functors would be appropriate.

extern "C" int my_mb_wc_utf8_thunk(const CHARSET_INFO *cs, my_wc_t *pwc,
                                   const uchar *s, const uchar *e);

extern "C" int my_mb_wc_utf8mb4_thunk(const CHARSET_INFO *cs, my_wc_t *pwc,
                                      const uchar *s, const uchar *e);

#endif  // MB_WC_INCLUDED
