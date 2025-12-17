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
#include <random>
#include <unordered_set>

#if defined(__linux__)
#include <unistd.h>
#include <sys/mman.h>
#endif

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

#if defined(__linux__)
TEST(HashTest, PageBoundaryCheck) {
  int ps_ = getpagesize();
  ASSERT_GT(ps_, 0);
  const uint32_t ps = static_cast<uint32_t>(ps_);
  void *addr_ = mmap(nullptr, (ps * 2), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  ASSERT_NE(addr_, MAP_FAILED);
  ASSERT_NE(addr_, nullptr);
  char *addr = static_cast<char *>(addr_);
  char *s = addr + ps - 8;
  memcpy(s, "aBcDeFgH", 8);
  int r = mprotect(addr + ps, ps, PROT_NONE);
  ASSERT_EQ(r, 0);
  EXPECT_EQ(hash_string_i("aBcDeFgH", 8), hash_string_i(s, 8));
  EXPECT_EQ(hash_string_i("aBcDeFgH", 8), hash_string_i_unsafe(s, 8));
  EXPECT_EQ(hash_string_cs("aBcDeFgH", 8), hash_string_cs(s, 8));
  EXPECT_EQ(hash_string_cs("aBcDeFgH", 8), hash_string_cs_unsafe(s, 8));
}
#endif

///////////////////////////////////////////////////////////////////////////////
// TabulationHash tests

TEST(TabulationHashTest, BasicFunctionality) {
  // Test that the hash function works and produces output.
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t> hash(rng);

  uint64_t input = 12345;
  uint64_t result = hash(input);

  // Just verify we get some result (not checking specific value).
  EXPECT_NE(result, 0); // Extremely unlikely to be zero for this input.
}

TEST(TabulationHashTest, Determinism) {
  // Same seed should produce same hash function and same results.
  std::mt19937_64 rng1(123);
  TabulationHash<uint64_t> hash1(rng1);

  std::mt19937_64 rng2(123);
  TabulationHash<uint64_t> hash2(rng2);

  for (uint64_t i = 0; i < 100; ++i) {
    EXPECT_EQ(hash1(i), hash2(i));
  }
}

TEST(TabulationHashTest, DifferentSeeds) {
  // Different seeds should produce different hash functions.
  std::mt19937_64 rng1(123);
  TabulationHash<uint64_t> hash1(rng1);

  std::mt19937_64 rng2(456);
  TabulationHash<uint64_t> hash2(rng2);

  // With very high probability, at least one of these should differ.
  size_t differences = 0;
  for (uint64_t i = 0; i < 100; ++i) {
    if (hash1(i) != hash2(i)) {
      ++differences;
    }
  }
  EXPECT_GT(differences, 50); // Should be different for most inputs.
}

TEST(TabulationHashTest, DifferentInputs) {
  // Different inputs should produce different outputs (with high probability).
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t> hash(rng);

  std::unordered_set<uint64_t> seen;
  for (uint64_t i = 0; i < 1000; ++i) {
    uint64_t result = hash(i);
    // Check for collisions - should be very rare with 64-bit output.
    EXPECT_EQ(seen.count(result), 0u);
    seen.insert(result);
  }
}

TEST(TabulationHashTest, EdgeCases) {
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t> hash(rng);

  // Test edge case values.
  uint64_t zero = hash(0);
  uint64_t max = hash(std::numeric_limits<uint64_t>::max());
  uint64_t one = hash(1);

  // All should be different with very high probability.
  EXPECT_NE(zero, max);
  EXPECT_NE(zero, one);
  EXPECT_NE(max, one);
}

TEST(TabulationHashTest, BitSensitivity) {
  // Changing a single bit should change the output.
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t> hash(rng);

  uint64_t base = 0x123456789abcdef0ULL;
  uint64_t base_hash = hash(base);

  // Flip each bit and verify the hash changes.
  for (int bit = 0; bit < 64; ++bit) {
    uint64_t modified = base ^ (1ULL << bit);
    uint64_t modified_hash = hash(modified);
    EXPECT_NE(base_hash, modified_hash)
        << "Flipping bit " << bit << " did not change hash";
  }
}

TEST(TabulationHashTest, Uint32Input) {
  // Test with 32-bit input type.
  std::mt19937_64 rng(42);
  TabulationHash<uint32_t> hash(rng);

  std::unordered_set<uint32_t> seen;
  for (uint32_t i = 0; i < 1000; ++i) {
    uint32_t result = hash(i);
    EXPECT_EQ(seen.count(result), 0u);
    seen.insert(result);
  }
}

TEST(TabulationHashTest, DifferentOutputType) {
  // Test with different output type (uint32_t -> uint64_t).
  std::mt19937_64 rng(42);
  TabulationHash<uint32_t, uint64_t> hash(rng);

  uint32_t input = 12345;
  uint64_t result = hash(input);

  // Verify we get a result.
  EXPECT_NE(result, 0);
}

TEST(TabulationHashTest, ArrayOutput) {
  // Test with std::array output for wider hashes.
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t, std::array<uint64_t, 2>> hash(rng);

  uint64_t input = 12345;
  auto result = hash(input);

  // Both elements should be non-zero with high probability.
  EXPECT_TRUE(result[0] != 0 || result[1] != 0);
}

TEST(TabulationHashTest, DifferentBlockSize) {
  // Test with 16-bit blocks instead of default 8-bit.
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t, uint64_t, uint16_t> hash(rng);

  std::unordered_set<uint64_t> seen;
  for (uint64_t i = 0; i < 1000; ++i) {
    uint64_t result = hash(i);
    EXPECT_EQ(seen.count(result), 0u);
    seen.insert(result);
  }
}

TEST(TabulationHashTest, TwistedTabulation) {
  // Test twisted tabulation with D > 0.
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t, uint64_t, uint8_t, 4> hash(rng);

  std::unordered_set<uint64_t> seen;
  for (uint64_t i = 0; i < 1000; ++i) {
    uint64_t result = hash(i);
    EXPECT_EQ(seen.count(result), 0u);
    seen.insert(result);
  }
}

TEST(TabulationHashTest, TwistedVsSimple) {
  // Compare twisted tabulation (D > 0) vs simple (D = 0).
  std::mt19937_64 rng1(42);
  TabulationHash<uint64_t, uint64_t, uint8_t, 0> simple(rng1);

  std::mt19937_64 rng2(42);
  TabulationHash<uint64_t, uint64_t, uint8_t, 4> twisted(rng2);

  // They should produce different results even with same seed.
  size_t differences = 0;
  for (uint64_t i = 0; i < 100; ++i) {
    if (simple(i) != twisted(i)) {
      ++differences;
    }
  }
  EXPECT_GT(differences, 50);
}

TEST(TabulationHashTest, Copyable) {
  // Test that hash objects can be copied.
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t> hash1(rng);

  // Copy construct.
  TabulationHash<uint64_t> hash2 = hash1;

  // Both should produce same results.
  for (uint64_t i = 0; i < 100; ++i) {
    EXPECT_EQ(hash1(i), hash2(i));
  }

  // Copy assign.
  std::mt19937_64 rng2(999);
  TabulationHash<uint64_t> hash3(rng2);
  hash3 = hash1;

  for (uint64_t i = 0; i < 100; ++i) {
    EXPECT_EQ(hash1(i), hash3(i));
  }
}

TEST(TabulationHashTest, Movable) {
  // Test that hash objects can be moved.
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t> hash1(rng);

  // Store some expected values.
  std::vector<uint64_t> expected;
  for (uint64_t i = 0; i < 100; ++i) {
    expected.push_back(hash1(i));
  }

  // Move construct.
  TabulationHash<uint64_t> hash2 = std::move(hash1);

  // hash2 should produce same results.
  for (uint64_t i = 0; i < 100; ++i) {
    EXPECT_EQ(hash2(i), expected[i]);
  }

  // Move assign.
  std::mt19937_64 rng2(999);
  TabulationHash<uint64_t> hash3(rng2);
  hash3 = std::move(hash2);

  // hash3 should produce same results.
  for (uint64_t i = 0; i < 100; ++i) {
    EXPECT_EQ(hash3(i), expected[i]);
  }
}

TEST(TabulationHashTest, SingleElementArray) {
  // Test with single-element std::array output (edge case for N=1).
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t, std::array<uint64_t, 1>> hash(rng);

  uint64_t input = 12345;
  auto result = hash(input);

  // Should produce non-trivial output.
  EXPECT_NE(result[0], 0);

  // Different inputs should produce different outputs.
  auto result2 = hash(54321);
  EXPECT_NE(result[0], result2[0]);
}

TEST(TabulationHashTest, Distribution) {
  // Test that hash outputs are well-distributed (not clustered).
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t> hash(rng);

  // Count how many outputs have each bit set.
  std::array<size_t, 64> bit_counts{};

  size_t n = 10000;
  for (uint64_t i = 0; i < n; ++i) {
    uint64_t result = hash(i);
    for (int bit = 0; bit < 64; ++bit) {
      if (result & (1ULL << bit)) {
        ++bit_counts[bit];
      }
    }
  }

  // Each bit should be set approximately 50% of the time.
  // Allow some deviation but not too much.
  for (int bit = 0; bit < 64; ++bit) {
    double ratio = static_cast<double>(bit_counts[bit]) / n;
    EXPECT_GT(ratio, 0.45) << "Bit " << bit << " too rarely set";
    EXPECT_LT(ratio, 0.55) << "Bit " << bit << " too often set";
  }
}

TEST(TabulationHashTest, ZeroInput) {
  // Specifically test zero input (special case).
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t> hash(rng);

  uint64_t zero_hash = hash(0);
  uint64_t one_hash = hash(1);

  // Should be different.
  EXPECT_NE(zero_hash, one_hash);
  EXPECT_NE(zero_hash, 0); // Unlikely to be zero.
}

TEST(TabulationHashTest, SequentialInputs) {
  // Test that sequential inputs don't produce sequential outputs.
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t> hash(rng);

  std::vector<uint64_t> hashes;
  for (uint64_t i = 0; i < 100; ++i) {
    hashes.push_back(hash(i));
  }

  // Check that hashes are not in sorted order (would indicate bad mixing).
  std::vector<uint64_t> sorted = hashes;
  std::sort(sorted.begin(), sorted.end());
  EXPECT_NE(hashes, sorted);
}

TEST(TabulationHashTest, MultipleInstances) {
  // Test that multiple independent hash instances work correctly.
  std::mt19937_64 rng1(111);
  std::mt19937_64 rng2(222);
  std::mt19937_64 rng3(333);

  TabulationHash<uint64_t> hash1(rng1);
  TabulationHash<uint64_t> hash2(rng2);
  TabulationHash<uint64_t> hash3(rng3);

  uint64_t input = 42;
  uint64_t r1 = hash1(input);
  uint64_t r2 = hash2(input);
  uint64_t r3 = hash3(input);

  // All three should be different with high probability.
  EXPECT_NE(r1, r2);
  EXPECT_NE(r1, r3);
  EXPECT_NE(r2, r3);
}

TEST(TabulationHashTest, ArrayOutputTwisted) {
  // Test combination of array output and twisted tabulation.
  std::mt19937_64 rng(42);
  TabulationHash<uint64_t, std::array<uint64_t, 2>, uint8_t, 4> hash(rng);

  uint64_t input = 12345;
  auto result = hash(input);

  // Should produce non-trivial output.
  EXPECT_TRUE(result[0] != 0 || result[1] != 0);

  // Different inputs should produce different outputs.
  auto result2 = hash(54321);
  EXPECT_TRUE(result[0] != result2[0] || result[1] != result2[1]);
}

} // namespace HPHP
