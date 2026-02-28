/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "unittest/gunit/strnxfrm.h"

#include <algorithm>

#include "my_inttypes.h"
#include "my_macros.h"

namespace strnxfrm_unittest {

// A copy of the original my_strnxfrm_simple
size_t strnxfrm_orig(const CHARSET_INFO *cs, uchar *dst, size_t dstlen,
                     uint nweights, const uchar *src, size_t srclen,
                     uint flags) {
  const uchar *map = cs->sort_order;
  uchar *d0 = dst;
  size_t frmlen;
  if ((frmlen = std::min<size_t>(dstlen, nweights)) > srclen) frmlen = srclen;
  if (dst != src) {
    const uchar *end;
    for (end = src + frmlen; src < end;) *dst++ = map[*src++];
  } else {
    const uchar *end;
    for (end = dst + frmlen; dst < end; dst++) *dst = map[(uchar)*dst];
  }
  return my_strxfrm_pad(cs, d0, dst, d0 + dstlen, (uint)(nweights - frmlen),
                        flags);
}

size_t strnxfrm_orig_unrolled(const CHARSET_INFO *cs, uchar *dst, size_t dstlen,
                              uint nweights, const uchar *src, size_t srclen,
                              uint flags) {
  const uchar *map = cs->sort_order;
  uchar *d0 = dst;
  size_t frmlen;
  const uchar *end;
  const uchar *remainder;
  if ((frmlen = std::min<size_t>(dstlen, nweights)) > srclen) frmlen = srclen;
  end = src + frmlen;
  remainder = src + (frmlen % 8);
  if (dst != src) {
    for (; src < remainder;) *dst++ = map[*src++];
    for (; src < end;) {
      *dst++ = map[*src++];
      *dst++ = map[*src++];
      *dst++ = map[*src++];
      *dst++ = map[*src++];
      *dst++ = map[*src++];
      *dst++ = map[*src++];
      *dst++ = map[*src++];
      *dst++ = map[*src++];
    }
  } else /* dst == src */
  {
    for (; dst < remainder;) {
      *dst = map[*dst];
      ++dst;
    }
    for (; dst < end;) {
      *dst = map[*dst];
      ++dst;
      *dst = map[*dst];
      ++dst;
      *dst = map[*dst];
      ++dst;
      *dst = map[*dst];
      ++dst;
      *dst = map[*dst];
      ++dst;
      *dst = map[*dst];
      ++dst;
      *dst = map[*dst];
      ++dst;
      *dst = map[*dst];
      ++dst;
    }
  }
  return my_strxfrm_pad(cs, d0, dst, d0 + dstlen, (uint)(nweights - frmlen),
                        flags);
}

// An alternative implementation, skipping the (dst != src) test.
size_t strnxfrm_new(const CHARSET_INFO *cs, uchar *dst, size_t dstlen,
                    uint nweights, const uchar *src, size_t srclen,
                    uint flags) {
  const uchar *map = cs->sort_order;
  uchar *d0 = dst;
  const uchar *end;
  size_t frmlen;
  if ((frmlen = std::min<size_t>(dstlen, nweights)) > srclen) frmlen = srclen;
  for (end = src + frmlen; src < end;) *dst++ = map[*src++];
  return my_strxfrm_pad(cs, d0, dst, d0 + dstlen, (uint)(nweights - frmlen),
                        flags);
}

size_t strnxfrm_new_unrolled(const CHARSET_INFO *cs, uchar *dst, size_t dstlen,
                             uint nweights, const uchar *src, size_t srclen,
                             uint flags) {
  const uchar *map = cs->sort_order;
  uchar *d0 = dst;
  const uchar *end;
  const uchar *remainder;
  size_t frmlen;
  if ((frmlen = std::min<size_t>(dstlen, nweights)) > srclen) frmlen = srclen;
  end = src + frmlen;
  remainder = src + (frmlen % 8);
  for (; src < remainder;) *dst++ = map[*src++];
  for (; src < end;) {
    *dst++ = map[*src++];
    *dst++ = map[*src++];
    *dst++ = map[*src++];
    *dst++ = map[*src++];
    *dst++ = map[*src++];
    *dst++ = map[*src++];
    *dst++ = map[*src++];
    *dst++ = map[*src++];
  }
  return my_strxfrm_pad(cs, d0, dst, d0 + dstlen, (uint)(nweights - frmlen),
                        flags);
}

}  // namespace strnxfrm_unittest
