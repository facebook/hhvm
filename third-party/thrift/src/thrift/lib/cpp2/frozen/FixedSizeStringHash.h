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

#include <string>
#include <utility>
#include <vector>

#include <folly/Range.h>
#include <folly/hash/Hash.h>
#include <folly/lang/Bits.h>

#include <thrift/lib/cpp2/frozen/Traits.h>

namespace apache::thrift::frozen {
namespace detail {
template <typename T>
inline const uint8_t* rangeDataStart(const T& value) {
  return reinterpret_cast<const uint8_t*>(value.data());
}

template <>
inline const uint8_t* rangeDataStart<folly::ByteRange>(
    const folly::ByteRange& value) {
  return value.begin();
}

} // namespace detail

template <size_t kSize, typename RangeType>
struct FixedSizeStringHash {
  static uint64_t hash(const RangeType& value) {
    if constexpr (kSize <= 8) {
      uint64_t tmp = 0;
      memcpy(&tmp, detail::rangeDataStart(value), kSize);
      return std::hash<uint64_t>()(tmp);
    } else {
      return folly::hash::fnv64_buf_BROKEN(
          detail::rangeDataStart(value), value.size());
    }
  }
};

} // namespace apache::thrift::frozen
