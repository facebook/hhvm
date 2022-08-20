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

// Operations supported by all ThriftType values.
#pragma once

#include <folly/Traits.h>
#include <thrift/lib/cpp2/op/detail/Clear.h>

namespace apache {
namespace thrift {
namespace op {

// Returns true iff the given value is 'empty', and not serialized in a
// 'terse' context.
//
// Some types cannot represent an 'empty' value. For example,
// a non-terse, non-optional field always serializes to a non-empty buffer,
// thus any struct with such a field, can never be 'empty'.
// In such cases, it is possible to deserialize from an empty buffer, but it is
// impossible to serialize to an empty buffer.
//
// For example:
//   isEmpty<i32_t>(0) -> true
//   isEmpty<i64_t>(1) -> false
//   isEmpty<set<i32_t>>({}) -> true
//   isEmpty<set<i32_t>>({0}) -> false
template <typename Tag>
FOLLY_INLINE_VARIABLE constexpr detail::Empty<Tag> isEmpty{};

// Returns the 'intrinsic' default for the given type.
//
// Ignores all 'custom' defaults set on fields within structured types.
//
// For example:
//   getDefault<type::i32_t>() -> 0
//   getDefault<type::set<type::i32_t>>() -> {}
//   getDefault<type::string_t>() -> ""
template <typename Tag>
FOLLY_INLINE_VARIABLE constexpr detail::GetIntrinsicDefault<Tag>
    getIntrinsicDefault{};

// Clears the given value, leaving it equal to its intrinsic default.
// For example:
//   clear<i32_t>(myInt) // sets myInt = 0.
//   clear<set<i32_t>>(myIntSet) // calls myIntSet.clear()
template <typename Tag>
FOLLY_INLINE_VARIABLE constexpr detail::Clear<Tag> clear{};

// Clears the given field, setting the field to initial state.
// For example:
//   // clears foo.field().value() to the intrinsic default value.
//   clear_field<field_tag>(foo.field(), foo)
//   // calls foo.optional_field().reset().
//   clear_field<field_tag>(foo.optional_field(), foo)
template <typename Tag>
FOLLY_INLINE_VARIABLE constexpr detail::ClearField<Tag> clear_field{};

} // namespace op
} // namespace thrift
} // namespace apache
