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
#include <thrift/lib/cpp2/op/detail/Compare.h>

namespace apache {
namespace thrift {
namespace op {

// A binary operator that returns true iff the given Thrift values are equal to
// each other.
//
// For example:
//   equal<i32_t>(1, 2) -> false
//   equal<double_t>(0.0, -0.0) -> true
//   equal<float_t>(NaN, NaN) -> false
//   equal<list<double_t>>([NaN, 0.0], [NaN, -0.0]) -> false
template <typename LTag, typename RTag = LTag>
struct EqualTo : detail::EqualTo<LTag, RTag> {};
template <typename LTag, typename RTag = LTag>
FOLLY_INLINE_VARIABLE constexpr EqualTo<LTag, RTag> equal{};

// A binary operator that returns true iff the given Thrift values are identical
// to each other (i.e. they are same representations).
//
// For example:
//   identical<i32_t>(1, 2) -> false
//   identical<double_t>(0.0, -0.0) -> false
//   identical<float_t>(NaN, NaN) -> true
//   identical<list<double_t>>([NaN, 0.0], [NaN, -0.0]) -> false
template <typename Tag>
struct IdenticalTo : detail::IdenticalTo<Tag> {};
template <typename Tag>
FOLLY_INLINE_VARIABLE constexpr IdenticalTo<Tag> identical{};

// A binary operator that returns true iff one Thrift values is less than
// the another.
//
// For example:
//   less<i32_t>(1, 2) -> true
//   less<double_t>(0.0, -0.0) -> false
//   less<float_t>(NaN, NaN) -> false
template <typename LTag, typename RTag = LTag>
struct Less : detail::LessThan<LTag, RTag> {};
template <typename LTag, typename RTag = LTag>
FOLLY_INLINE_VARIABLE constexpr Less<LTag, RTag> less{};

// Compares two Thrift values, returning the associated folly::ordering value.
//
// For example:
//   compare<i32_t>(1, 2) -> folly::ordering::lt
//   less<double_t>(0.0, -0.0) -> folly::ordering::eq
//   compare<string_t>("aa", "a") -> folly::ordering::gt
template <typename LTag, typename RTag = LTag>
FOLLY_INLINE_VARIABLE constexpr detail::CompareWith<LTag, RTag> compare{};

} // namespace op
} // namespace thrift
} // namespace apache
