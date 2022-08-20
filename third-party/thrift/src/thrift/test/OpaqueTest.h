/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/lib/cpp2/Thrift.h>

// Implement basic opaque typedef suitable for testing
//
// In real life it would have operators defined that access underlying value in
// a type-safe manner

namespace apache {
namespace thrift {
namespace test {

template <typename RawType, typename Tag>
class Opaque {
 private:
  RawType val_{};

 public:
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(__value());

  Opaque() = default;
  explicit Opaque(const RawType& val) : val_(val) {}
  RawType& __value() { return val_; }
  const RawType& __value() const { return val_; }
  explicit operator RawType() const { return val_; }
  bool operator==(const Opaque& rhs) const { return val_ == rhs.val_; }
};

using OpaqueDouble1 = Opaque<double, std::integral_constant<int, 1>>;
using OpaqueDouble2 = Opaque<double, std::integral_constant<int, 2>>;
using NonConvertibleId = Opaque<int64_t, void>;

} // namespace test
} // namespace thrift
} // namespace apache
