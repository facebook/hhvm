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

#include <thrift/lib/cpp2/frozen/FixedSizeStringHash.h>
#include <thrift/lib/cpp2/frozen/Traits.h>

namespace apache::thrift::frozen {

/*
 * For representing sequences of unpacked integral types.
 *
 * Use this in Thrift IDL like:
 *
 *   cpp_include "thrift/lib/cpp2/frozen/HintTypes.h"
 *
 *   struct MyStruct {
 *     7: list<i32>
 *        (cpp.template = "apache::thrift::frozen::VectorUnpacked")
 *        ids,
 *   }
 */
template <class T>
class VectorUnpacked : public std::vector<T> {
  static_assert(
      std::is_arithmetic<T>::value || std::is_enum<T>::value,
      "Unpacked storage is only available for simple item types");
  using std::vector<T>::vector;
};

/*
 * For representing a string with a fixed length.
 * The size is explicitly specified in the IDL schema. As a result, the
 * serialized data should be smaller than the `string` type.
 *
 * Use this in Thrift IDL like:
 *
 *   cpp_include "thrift/lib/cpp2/frozen/HintTypes.h"
 *   struct MyStruct {
 *     1: string
 *        (cpp.type = "apache::thrift::frozen::FixedSizeString<4>")
 *        name,
 *   }
 *
 * Note unqualified thrift fields will be serialized using the default value,
 * and as a result cannot be unset. If a FixedSizeString field is allowed to be
 * absent, explicitly specify it as an optional field.
 */
template <size_t kSize>
class FixedSizeString : public std::string {
 public:
  static constexpr size_t kFixedSize = kSize;

  template <typename... Args>
  explicit FixedSizeString(Args&&... args)
      : std::string(std::forward<Args>(args)...) {}

  FixedSizeString& operator=(const std::string& other) {
    std::string::operator=(other);
    return *this;
  }
};

} // namespace apache::thrift::frozen

THRIFT_DECLARE_TRAIT_TEMPLATE(IsString, apache::thrift::frozen::VectorUnpacked)

namespace std {
template <size_t kSize>
struct hash<apache::thrift::frozen::FixedSizeString<kSize>> {
  size_t operator()(
      const apache::thrift::frozen::FixedSizeString<kSize>& value) const {
    return apache::thrift::frozen::FixedSizeStringHash<
        kSize,
        apache::thrift::frozen::FixedSizeString<kSize>>::hash(value);
  }
};
} // namespace std
