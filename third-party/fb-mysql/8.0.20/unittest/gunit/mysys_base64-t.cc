/* Copyright (c) 2003, 2019, Oracle and/or its affiliates. All rights reserved.

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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>
#include <string.h>

#include "base64.h"
#include "my_sys.h"

namespace mysys_base64_unittest {

const int BASE64_LOOP_COUNT = 500;

TEST(Mysys, Base64) {
  int i, cmp;
  size_t j, k, l, dst_len, needed_length;
  MY_INIT("base64-t");

  for (i = 0; i < BASE64_LOOP_COUNT; i++) {
    /* Create source data */
    const size_t src_len = rand() % 1000 + 1;

    char *src = (char *)malloc(src_len);
    char *s = src;
    char *str;
    char *dst;

    for (j = 0; j < src_len; j++) {
      char c = rand();
      *s++ = c;
    }

    /* Encode */
    needed_length = base64_needed_encoded_length(src_len);
    str = (char *)malloc(needed_length);
    for (k = 0; k < needed_length; k++)
      str[k] = (char)0xff; /* Fill memory to check correct NUL termination */
    EXPECT_EQ(0, base64_encode(src, src_len, str))
        << "base64_encode: size " << i;
    EXPECT_EQ(needed_length, strlen(str) + 1)
        << "base64_needed_encoded_length: " << i;

    /* Decode */
    dst = (char *)malloc(base64_needed_decoded_length(strlen(str)));
    dst_len = base64_decode(str, strlen(str), dst, nullptr, 0);
    EXPECT_EQ(dst_len, src_len) << "Comparing lengths";

    cmp = memcmp(src, dst, src_len);
    EXPECT_EQ(0, cmp) << "Comparing encode-decode result";
    if (cmp != 0) {
      char buf[80];
      ADD_FAILURE()
          << "       --------- src ---------   --------- dst ---------";
      for (k = 0; k < src_len; k += 8) {
        sprintf(buf, "%.4x   ", (uint)k);
        for (l = 0; l < 8 && k + l < src_len; l++) {
          unsigned char c = src[k + l];
          sprintf(buf, "%.2x ", (unsigned)c);
        }

        sprintf(buf, "  ");

        for (l = 0; l < 8 && k + l < dst_len; l++) {
          unsigned char c = dst[k + l];
          sprintf(buf, "%.2x ", (unsigned)c);
        }
        ADD_FAILURE() << buf;
      }
      ADD_FAILURE() << "src length: " << src_len << "dst length: " << dst_len;
    }
    free(str);
    free(src);
    free(dst);
  }
}

}  // namespace mysys_base64_unittest
