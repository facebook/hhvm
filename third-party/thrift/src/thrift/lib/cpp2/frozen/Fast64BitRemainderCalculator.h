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

#pragma once

#include <assert.h>
#include <folly/Traits.h>

namespace apache::thrift::frozen::detail {
class Fast64BitRemainderCalculator {
#if FOLLY_HAVE_INT128_T
 public:
  Fast64BitRemainderCalculator() = default;
  explicit Fast64BitRemainderCalculator(uint64_t divisor)
      : fastRemainderConstant_(
            divisor ? (~folly::uint128_t(0) / divisor + 1) : 0) {
#ifndef NDEBUG
    divisor_ = divisor;
#endif
  }

  size_t remainder(size_t lhs, size_t rhs) const {
    const folly::uint128_t lowBits = fastRemainderConstant_ * lhs;
    auto result = mul128_u64(lowBits, rhs);
    assert(rhs == divisor_);
    assert(result == lhs % rhs);
    return result;
  }

 private:
  static uint64_t mul128_u64(folly::uint128_t lowbits, uint64_t d) {
    folly::uint128_t bottom = ((lowbits & 0xFFFFFFFFFFFFFFFFUL) * d) >> 64;
    folly::uint128_t top = (lowbits >> 64) * d;
    return static_cast<uint64_t>((bottom + top) >> 64);
  }
  folly::uint128_t fastRemainderConstant_ = 0;
#ifndef NDEBUG
  size_t divisor_ = 0;
#endif
#else
 public:
  Fast64BitRemainderCalculator() = default;
  explicit Fast64BitRemainderCalculator(size_t) {}

  auto remainder(size_t lhs, size_t rhs) const { return lhs % rhs; }
#endif
};
} // namespace apache::thrift::frozen::detail
