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

#include <folly/Range.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace apache::thrift::transcode {

// Untagged scalar register storage. The target field's ScalarOp::valueKind is
// the discriminator: Bool/I8/I16/I32/I64/Enum read int64_t, F32/F64 read
// double, and Bytes reads folly::ByteRange.
struct ScalarOverrideValue {
  static constexpr size_t kStorageSize = sizeof(folly::ByteRange) >
          sizeof(double)
      ? sizeof(folly::ByteRange)
      : sizeof(double);

  static ScalarOverrideValue fromInt64(int64_t value) { return from(value); }

  static ScalarOverrideValue fromDouble(double value) { return from(value); }

  static ScalarOverrideValue fromByteRange(folly::ByteRange value) {
    return from(value);
  }

  template <typename T>
  T as() const {
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(sizeof(T) <= kStorageSize);
    T value;
    std::memcpy(&value, storage_.data(), sizeof(T));
    return value;
  }

 private:
  template <typename T>
  static ScalarOverrideValue from(T value) {
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(sizeof(T) <= kStorageSize);
    ScalarOverrideValue out;
    std::memcpy(out.storage_.data(), &value, sizeof(T));
    return out;
  }

  alignas(std::max_align_t) std::array<std::byte, kStorageSize> storage_{};
};

// Side-channel input for root-level fields that are not present in the
// serialized payload, such as values extracted from an HTTP path, header, or
// cookie before dispatching to a Thrift RPC envelope.
struct ScalarFieldOverride {
  int16_t fieldId = 0;
  ScalarOverrideValue value;
};

using ScalarFieldOverrides = folly::Range<const ScalarFieldOverride*>;

} // namespace apache::thrift::transcode
