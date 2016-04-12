/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/hash.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <cstring>

namespace HPHP {

TEST(HashTest, Case) {
  char* stra = "abcdeFGHHHh";
  char* strb = "ABcdEfghhHH";
  uint32_t len = std::strlen(stra);

  auto const ihasha = hash_string_i(stra, len);
  auto const ihashb = hash_string_i(strb, len);

  EXPECT_EQ(ihasha, ihashb);
  EXPECT_EQ(ihasha, hash_string_i_unsafe(stra, len));
  EXPECT_EQ(hash_string_cs(stra, len), hash_string_cs_unsafe(stra, len));
}

TEST(HashTest, Alignment) {
  char buffer[256];
  char* pattern = "abCDefGH123";
  auto const len = std::strlen(pattern);
  auto const h = hash_string_i(pattern, len);

  for (char* start = buffer; start + len + 8 < buffer + sizeof(buffer);
       start += len) {
    std::memcpy(start, pattern, len);
    // Aligned version should work as long as we don't read beyond end of the
    // buffer.
    auto const aligned_hash = hash_string_i_unsafe(start, len);
    auto const unaligned_hash = hash_string_i(start, len);
    EXPECT_EQ(h, aligned_hash);
    EXPECT_EQ(h, unaligned_hash);
  }
}

#ifdef __x86_64__
TEST(HashTest, SSE42) {
  if (IsSSEHashSupported()) {
    {
      char* stra = "abcdeFGHHHh";
      char* strb = "ABcdEfghhHH";
      uint32_t len = std::strlen(stra);

      auto const ihasha = hash_string_i_unaligned_crc(stra, len);
      auto const ihashb = hash_string_i_unaligned_crc(strb, len);

      EXPECT_EQ(ihasha, ihashb);
      EXPECT_EQ(ihasha, hash_string_i_crc(stra, len));
      EXPECT_EQ(hash_string_cs_crc(stra, len),
                hash_string_cs_unaligned_crc(stra, len));
    }
    {
      char buffer[256];
      char* pattern = "abCDefGH123";
      auto const len = std::strlen(pattern);
      auto const h = hash_string_i_unaligned_crc(pattern, len);

      for (char* start = buffer; start + len + 8 < buffer + sizeof(buffer);
           start += len) {
        std::memcpy(start, pattern, len);
        // Aligned version should work as long as we don't read beyond end of
        // the buffer.
        auto const aligned_hash = hash_string_i_crc(start, len);
        auto const unaligned_hash = hash_string_i_unaligned_crc(start, len);
        EXPECT_EQ(h, aligned_hash);
        EXPECT_EQ(h, unaligned_hash);
      }
    }
  }
}
#endif
}
