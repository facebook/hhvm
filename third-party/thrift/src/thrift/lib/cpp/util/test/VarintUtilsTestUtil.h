/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <type_traits>
#include <vector>

#include <folly/Utility.h>

inline constexpr size_t N = 1000;

constexpr uint64_t lim(size_t bytes) {
  return ((uint64_t(1) << (bytes * 7)) - 1);
}

template <typename I>
static constexpr auto max_v = std::numeric_limits<I>::max();

template <typename I, size_t N, I First, I Last>
struct varint_case {
  using integer_type = I;
  using unsigned_case = varint_case<
      std::make_unsigned_t<I>,
      N,
      folly::to_unsigned(First),
      folly::to_unsigned(Last)>;
  using signed_case = varint_case<
      std::make_signed_t<I>,
      N,
      folly::to_signed(First),
      folly::to_signed(Last)>;

  static std::vector<I> gen() {
    if constexpr (std::is_signed_v<I>) {
      auto u = unsigned_case::gen();
      std::vector<std::make_signed_t<I>> s(N);
      for (size_t i = 0; i < N; ++i) {
        s[i] = folly::to_signed(u[i]);
      }
      return s;
    }

    std::default_random_engine rng(1729); // Make PRNG deterministic.
    std::uniform_int_distribution<uint64_t> dist(First, Last);
    std::vector<I> v(N);
    std::generate(v.begin(), v.end(), [&] { return I(dist(rng)); });
    return v;
  }
};

using u8_1b = varint_case<uint8_t, N, 0, lim(1)>;
using u16_1b = varint_case<uint16_t, N, 0, lim(1)>;
using u32_1b = varint_case<uint32_t, N, 0, lim(1)>;
using u64_1b = varint_case<uint64_t, N, 0, lim(1)>;

using u8_2b = varint_case<uint8_t, N, lim(1), uint8_t(lim(2))>;
using u16_2b = varint_case<uint16_t, N, lim(1), lim(2)>;
using u32_2b = varint_case<uint32_t, N, lim(1), lim(2)>;
using u64_2b = varint_case<uint64_t, N, lim(1), lim(2)>;

using u16_3b = varint_case<uint16_t, N, lim(2), uint16_t(lim(3))>;
using u32_3b = varint_case<uint32_t, N, lim(2), lim(3)>;
using u64_3b = varint_case<uint64_t, N, lim(2), lim(3)>;

using u32_4b = varint_case<uint32_t, N, lim(3), lim(4)>;
using u64_4b = varint_case<uint64_t, N, lim(3), lim(4)>;

using u32_5b = varint_case<uint32_t, N, lim(4), uint32_t(lim(5))>;
using u64_5b = varint_case<uint64_t, N, lim(4), lim(5)>;

using u64_6b = varint_case<uint64_t, N, lim(5), lim(6)>;
using u64_7b = varint_case<uint64_t, N, lim(6), lim(7)>;
using u64_8b = varint_case<uint64_t, N, lim(7), lim(8)>;
using u64_9b = varint_case<uint64_t, N, lim(8), lim(9)>;
using u64_10b = varint_case<uint64_t, N, lim(9), max_v<uint64_t>>;

using u8_any = varint_case<uint8_t, N, 0, max_v<uint8_t>>;
using u16_any = varint_case<uint16_t, N, 0, max_v<uint16_t>>;
using u32_any = varint_case<uint32_t, N, 0, max_v<uint32_t>>;
using u64_any = varint_case<uint64_t, N, 0, max_v<uint64_t>>;

using s8_1b = u8_1b::signed_case;
using s16_1b = u16_1b::signed_case;
using s32_1b = u32_1b::signed_case;
using s64_1b = u64_1b::signed_case;

using s8_2b = u8_2b::signed_case;
using s16_2b = u16_2b::signed_case;
using s32_2b = u32_2b::signed_case;
using s64_2b = u64_2b::signed_case;

using s16_3b = u16_3b::signed_case;
using s32_3b = u32_3b::signed_case;
using s64_3b = u64_3b::signed_case;

using s32_4b = u32_4b::signed_case;
using s64_4b = u64_4b::signed_case;

using s32_5b = u32_5b::signed_case;
using s64_5b = u64_5b::signed_case;

using s64_6b = u64_6b::signed_case;
using s64_7b = u64_7b::signed_case;
using s64_8b = u64_8b::signed_case;
using s64_9b = u64_9b::signed_case;
using s64_10b = u64_10b::signed_case;

using s8_any = u8_any::signed_case;
using s16_any = u16_any::signed_case;
using s32_any = u32_any::signed_case;
using s64_any = u64_any::signed_case;

// Generate test values using an exponential distribution with given median
template <uint64_t Median>
struct varint_case_exponential {
  using integer_type = uint64_t;

  static std::vector<uint64_t> gen() {
    std::default_random_engine rng(1729); // Make PRNG deterministic.
    constexpr double kLn2 = 0.69314718056;
    constexpr double lambda =
        kLn2 / Median; // median of Exp(lambda) is ln(2) / lambda
    std::exponential_distribution<double> dist(lambda);
    std::vector<uint64_t> v(N);
    std::generate(v.begin(), v.end(), [&] { return uint64_t(dist(rng)); });
    return v;
  }
};

using exponential_1b = varint_case_exponential<lim(1)>;
using exponential_2b = varint_case_exponential<lim(2)>;
using exponential_3b = varint_case_exponential<lim(3)>;
