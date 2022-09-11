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

#include <iterator>
#include <stdexcept>

#include <thrift/lib/cpp2/op/detail/BaseOp.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// TODO(afuller): Support heterogenous comparisons.
template <typename Tag>
struct NumberOp : BaseOp<Tag> {
  using T = type::native_type<Tag>;
  using Base = BaseOp<Tag>;
  using Base::ref;

  static bool add(T& self, const T& val) { return (self += val, true); }
  static bool add(void* s, const Dyn& v) { return add(ref(s), v.as<Tag>()); }
};

template <>
struct AnyOp<type::bool_t> : NumberOp<type::bool_t> {};
template <>
struct AnyOp<type::byte_t> : NumberOp<type::byte_t> {};
template <>
struct AnyOp<type::i16_t> : NumberOp<type::i16_t> {};
template <>
struct AnyOp<type::i32_t> : NumberOp<type::i32_t> {};
template <>
struct AnyOp<type::i64_t> : NumberOp<type::i64_t> {};
template <>
struct AnyOp<type::float_t> : NumberOp<type::float_t> {};
template <>
struct AnyOp<type::double_t> : NumberOp<type::double_t> {};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
