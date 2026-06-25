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

#include <thrift/lib/cpp2/op/detail/Compare.h>

namespace apache::thrift::op {

template <typename Tag = void>
struct EqualTo : detail::EqualTo<Tag> {};

/// A binary operator that returns true iff the given Thrift values are equal to
/// each other.
///
/// For example:
/// * equal<int32_t>(1, 2) -> false
/// * equal<double_t>(0.0, -0.0) -> true
/// * equal<float_t>(NaN, NaN) -> false
/// * equal<list<double_t>>([NaN, 0.0], [NaN, -0.0]) -> false
template <typename TagOrT = void>
inline constexpr EqualTo<type::infer_tag<TagOrT>> equal{};

template <typename Tag = void>
struct IdenticalTo : detail::IdenticalTo<Tag> {};

/// A binary operator that returns true iff the given Thrift values are
/// identical to each other (i.e. they are same representations).
///
/// For example:
/// * identical<int32_t>(1, 2) -> false
/// * identical<double_t>(0.0, -0.0) -> false
/// * identical<float_t>(NaN, NaN) -> true
/// * identical<list<double_t>>([NaN, 0.0], [NaN, -0.0]) -> false
template <typename TagOrT = void>
inline constexpr IdenticalTo<type::infer_tag<TagOrT>> identical{};

template <typename Tag = void>
struct Less : detail::LessThan<Tag> {};

/// A binary operator that returns true iff one Thrift value is less than
/// another.
///
/// For example:
/// * less<int32_t>(1, 2) -> true
/// * less<double_t>(0.0, -0.0) -> false
/// * less<float_t>(NaN, NaN) -> false
template <typename TagOrT = void>
inline constexpr Less<type::infer_tag<TagOrT>> less{};

/// Compares two Thrift values, returning the associated std::partial_ordering
/// value.
///
/// For example:
/// * compare<int32_t>(1, 2) -> std::partial_ordering::less
/// * compare<double_t>(0.0, -0.0) -> std::partial_ordering::equivalent
/// * compare<string_t>("aa", "a") -> std::partial_ordering::greater
template <typename TagOrT = void>
inline constexpr detail::CompareThreeWay<type::infer_tag<TagOrT>> compare{};

/// A binary operator that returns true iff one Thrift value is less than
/// another, comparing struct fields by sorted field ID order (instead of
/// field declaration order). This matches the comparison behavior of the
/// Thrift Object Model.
///
/// For example:
/// * stable_less<MyStruct>(a, b) -> compares fields by field ID
template <typename TagOrT = void>
inline constexpr detail::StableLessThan<type::infer_tag<TagOrT>> stable_less{};

} // namespace apache::thrift::op
