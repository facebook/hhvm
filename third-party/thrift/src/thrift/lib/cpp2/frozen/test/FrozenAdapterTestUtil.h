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
#include <string>
#include <string_view>
#include <vector>

#include <folly/Conv.h>
#include <folly/String.h>

namespace apache::thrift::test {

/**
 * Adapter whose adapted type is a plain std::vector<int64_t>, decoded from a
 * packed binary blob. This mirrors production field adapters that unpack a
 * compressed integer list: the adapted type is a native container frozen
 * already knows how to lay out.
 */
struct PackedIntListAdapter {
  static std::vector<std::int64_t> fromThrift(const std::string& packed) {
    std::vector<std::int64_t> values;
    if (packed.empty()) {
      return values;
    }
    std::vector<std::string_view> parts;
    folly::split(',', packed, parts);
    values.reserve(parts.size());
    for (const auto& part : parts) {
      values.push_back(folly::to<std::int64_t>(part));
    }
    return values;
  }

  static std::string toThrift(const std::vector<std::int64_t>& values) {
    return folly::join(',', values);
  }
};

/**
 * Field adapter (takes a FieldContext) whose adapted type differs from the
 * thrift type. The FieldContext overload is the one that genuinely needs the
 * enclosing-struct template parameter of `adapted_field_t`, which the frozen
 * layout header has to be able to name from namespace apache::thrift::frozen.
 */
struct WidenToI64FieldAdapter {
  template <typename Context>
  static std::int64_t fromThriftField(std::int32_t value, Context&&) {
    return static_cast<std::int64_t>(value);
  }

  static std::int32_t toThrift(std::int64_t value) {
    return static_cast<std::int32_t>(value);
  }
};

} // namespace apache::thrift::test
