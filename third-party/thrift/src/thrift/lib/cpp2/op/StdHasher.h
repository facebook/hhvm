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

#include <cstdint>
#include <type_traits>

#include <folly/Hash.h>
#include <folly/Range.h>
#include <folly/io/IOBuf.h>

namespace apache {
namespace thrift {
namespace op {

class StdHasher {
 public:
  size_t getResult() const { return result_; }

  template <typename T>
  constexpr std::enable_if_t<std::is_arithmetic<T>::value> combine(
      const T& val) {
    result_ = folly::hash::hash_combine(val, result_);
  }
  template <typename T>
  constexpr std::enable_if_t<std::is_enum<T>::value> combine(const T& val) {
    combine(folly::to_underlying(val));
  }
  void combine(folly::ByteRange value) {
    result_ = folly::hash::hash_combine(
        folly::hash::hash_range(value.begin(), value.end()), result_);
  }
  void combine(const StdHasher& other) { combine(other.result_); }

  void combine(const folly::IOBuf& value) {
    combine(folly::IOBufHash{}(value));
  }

  void finalize() {}

  bool operator<(const StdHasher& other) const {
    return result_ < other.result_;
  }

 private:
  size_t result_ = 0;
};

} // namespace op
} // namespace thrift
} // namespace apache
