/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <folly/Range.h>

namespace carbon {
namespace test {

class TestRequest;
class TestCompactRequest;

namespace util {

constexpr auto kMinInt8 = std::numeric_limits<int8_t>::min();
constexpr auto kMinInt16 = std::numeric_limits<int16_t>::min();
constexpr auto kMinInt32 = std::numeric_limits<int32_t>::min();
constexpr auto kMinInt64 = std::numeric_limits<int64_t>::min();

constexpr auto kMaxUInt8 = std::numeric_limits<uint8_t>::max();
constexpr auto kMaxUInt16 = std::numeric_limits<uint16_t>::max();
constexpr auto kMaxUInt32 = std::numeric_limits<uint32_t>::max();
constexpr auto kMaxUInt64 = std::numeric_limits<uint64_t>::max();

constexpr folly::StringPiece kShortString = "aaaaaaaaaa";

std::string longString();
template <class T>
void expectEqSimpleStruct(const T& a, const T& b);
void expectEqTestRequest(const TestRequest& a, const TestRequest& b);

template <class T, class Out = T>
Out serializeAndDeserialize(const T&);

template <class T, class Out = T>
Out serializeCarbonAndDeserializeCompactCompatibility(const T&);

template <class T, class Out = T>
Out serializeAndDeserialize(const T&, size_t& bytesWritten);

/**
 * Takes a boolean predicate and returns the list of subranges over
 * [numeric_limits<T>::min(), numeric_limits<T>::max()] satisfying the
 * predicate.
 */
template <class T>
std::vector<std::pair<T, T>> satisfiedSubranges(std::function<bool(T)> pred) {
  static_assert(
      std::is_integral<T>::value,
      "satisfiedSubranges may only be used over ranges of integers");

  bool inSatisfiedRange = false;
  T startRange;
  T endRange;
  std::vector<std::pair<T, T>> satisfiedRanges;

  auto i = std::numeric_limits<T>::min();
  while (true) {
    const bool satisfied = pred(i);
    if (satisfied && !inSatisfiedRange) {
      inSatisfiedRange = true;
      startRange = i;
    } else if (!satisfied && inSatisfiedRange) {
      inSatisfiedRange = false;
      endRange = i - 1;
      satisfiedRanges.emplace_back(startRange, endRange);
    }

    if (i == std::numeric_limits<T>::max()) {
      break;
    }
    ++i;
  }

  if (inSatisfiedRange) {
    endRange = i;
    satisfiedRanges.emplace_back(startRange, endRange);
  }

  return satisfiedRanges;
}

} // namespace util
} // namespace test
} // namespace carbon

#include "Util-inl.h"
