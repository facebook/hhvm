/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

// This test has a terrible name, as it simply tests alignment issues around
// the accelerated functions.
#if defined(__x86_64__) || defined(__aarch64__)
TEST(HashTest, SSE42) {
  if (IsHWHashSupported()) {
    {
      char* stra = "abcdeFGHHHh";
      char* strb = "ABcdEfghhHH";
      uint32_t len = std::strlen(stra);

      auto const ihasha = hash_string_i(stra, len);
      auto const ihashb = hash_string_i(strb, len);

      EXPECT_EQ(ihasha, ihashb);
      EXPECT_EQ(ihasha, hash_string_i_unsafe(stra, len));
      EXPECT_EQ(hash_string_cs(stra, len), hash_string_cs_unsafe(stra, len));
    }
    {
      char buffer[256];
      char* pattern = "abCDefGH123";
      auto const len = std::strlen(pattern);
      auto const h = hash_string_i(pattern, len);

      for (char* start = buffer; start + len + 8 < buffer + sizeof(buffer);
           start += len) {
        std::memcpy(start, pattern, len);
        // Aligned version should work as long as we don't read beyond end of
        // the buffer.
        auto const aligned_hash = hash_string_i_unsafe(start, len);
        auto const unaligned_hash = hash_string_i(start, len);
        EXPECT_EQ(h, aligned_hash);
        EXPECT_EQ(h, unaligned_hash);
      }
    }
  }
}
#endif

TEST(HashTest, CaseSensitiveAlignmentStress) {
  static const char baseline_ascii[] = "abcdefghijklmnop";
  unsigned int baseline = hash_string_cs(baseline_ascii, 16);

  static const char off1s[] = "\0abcdefghijklmnop";
  EXPECT_EQ(baseline, hash_string_cs(off1s + 1, 16));

  static const char off2s[] = "\0\0abcdefghijklmnop";
  EXPECT_EQ(baseline, hash_string_cs(off2s + 2, 16));

  static const char off3s[] = "\0\0\0abcdefghijklmnop";
  EXPECT_EQ(baseline, hash_string_cs(off3s + 3, 16));

  static const char off4s[] = "\0\0\0\0abcdefghijklmnop";
  EXPECT_EQ(baseline, hash_string_cs(off4s + 4, 16));

  static const char off5s[] = "\0\0\0\0\0abcdefghijklmnop";
  EXPECT_EQ(baseline, hash_string_cs(off5s + 5, 16));

  static const char off6s[] = "\0\0\0\0\0\0abcdefghijklmnop";
  EXPECT_EQ(baseline, hash_string_cs(off6s + 6, 16));

  static const char off7s[] = "\0\0\0\0\0\0\0abcdefghijklmnop";
  EXPECT_EQ(baseline, hash_string_cs(off7s + 7, 16));
}

TEST(HashTest, CaseSensitiveUnsafeEquivalence) {
  static const char base_ascii[] = "abcdefghijklm";
  unsigned int base = hash_string_cs(base_ascii, strlen(base_ascii));
  EXPECT_EQ(base, hash_string_cs_unsafe(base_ascii, strlen(base_ascii)));
}

TEST(HashTest, CaseInsensitiveAlignmentStress) {
  static const char base_uc_ascii[] = "ABCDEFGHIJKLMNOP";
  unsigned int base_uc = hash_string_cs(base_uc_ascii, 16);

  static const char base_aligned_ascii[] = "aBcDeFgHiJkLmNoP";
  static const char off1s[] = "\0aBcDeFgHiJkLmNoP";
  static const char off2s[] = "\0\0aBcDeFgHiJkLmNoP";
  static const char off3s[] = "\0\0\0aBcDeFgHiJkLmNoP";
  static const char off4s[] = "\0\0\0\0aBcDeFgHiJkLmNoP";
  static const char off5s[] = "\0\0\0\0\0aBcDeFgHiJkLmNoP";
  static const char off6s[] = "\0\0\0\0\0\0aBcDeFgHiJkLmNoP";
  static const char off7s[] = "\0\0\0\0\0\0\0aBcDeFgHiJkLmNoP";

  EXPECT_EQ(base_uc, hash_string_i(base_aligned_ascii, 16));
  EXPECT_EQ(base_uc, hash_string_i(off1s + 1, 16));
  EXPECT_EQ(base_uc, hash_string_i(off2s + 2, 16));
  EXPECT_EQ(base_uc, hash_string_i(off3s + 3, 16));
  EXPECT_EQ(base_uc, hash_string_i(off4s + 4, 16));
  EXPECT_EQ(base_uc, hash_string_i(off5s + 5, 16));
  EXPECT_EQ(base_uc, hash_string_i(off6s + 6, 16));
  EXPECT_EQ(base_uc, hash_string_i(off7s + 7, 16));
}

TEST(HashTest, CaseInsensitiveUnsafeEquivalence) {
  static const char base_ascii[] = "aBcDeFgHiJkLmNoP";
  unsigned int base = hash_string_i(base_ascii, strlen(base_ascii));
  EXPECT_EQ(base, hash_string_i_unsafe(base_ascii, strlen(base_ascii)));
}

} // namespace HPHP
