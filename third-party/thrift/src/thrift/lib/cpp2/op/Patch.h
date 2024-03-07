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

#include <type_traits>

#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/detail/Patch.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/gen-cpp2/patch_types.h>

namespace apache {
namespace thrift {
namespace op {

namespace detail {

template <class T>
struct PatchType {};

template <>
struct PatchType<type::bool_t> {
  using type = op::BoolPatch;
};
template <>
struct PatchType<type::byte_t> {
  using type = op::BytePatch;
};
template <>
struct PatchType<type::i16_t> {
  using type = op::I16Patch;
};
template <>
struct PatchType<type::i32_t> {
  using type = op::I32Patch;
};
template <>
struct PatchType<type::i64_t> {
  using type = op::I64Patch;
};
template <>
struct PatchType<type::float_t> {
  using type = op::FloatPatch;
};
template <>
struct PatchType<type::double_t> {
  using type = op::DoublePatch;
};
template <>
struct PatchType<type::string_t> {
  using type = op::StringPatch;
};
template <>
struct PatchType<type::binary_t> {
  using type = op::BinaryPatch;
};
template <class T>
struct PatchType<type::struct_t<T>> {
  using type = StructPatch<
      ::apache::thrift::detail::st::private_access::patch_struct<T>>;
};
template <class T>
struct PatchType<type::union_t<T>> {
  using type =
      UnionPatch<::apache::thrift::detail::st::private_access::patch_struct<T>>;
};

template <class T>
struct SafePatchType {};

} // namespace detail

/// The safe patch represenations for the base thrift types.
///
/// Safe patch provides versioning to indicate the minimum Thrift Patch version
/// required to safely and successfully process that patch as well as opaque
/// storage that is resilient to Thrift schema compatibility.
template <typename T>
using safe_patch_type =
    typename detail::SafePatchType<type::infer_tag<T>>::type;

/// The patch represenations for the base thrift types.
///
/// All patch types support the following methods:
/// - empty() - Returns true iff the patch is a noop.
/// - reset() - Makes the patch a noop.
/// - apply(T&& value) - Applies the patch to the given value, in place.
/// - merge(P&& next) - Merges the 'next' patch into this one, such that the
/// result is equivalent to applying this and next in sequence.
/// - assign(U&& value) - Updates the patch to assign the given value.
/// - operator=(U&& value) - An alias for assign.
/// - get() - Returns the underlying Thrift representation for the patch.
///
/// For example:
/// * int32_t value = 1;
/// * I32Patch patch;
/// * patch = 2;          // Equivalent to calling patch.assign(2).
/// * patch.apply(value); // Sets value to 2;
template <typename T>
using patch_type = typename detail::PatchType<type::infer_tag<T>>::type;

template <typename T, typename = void>
inline constexpr bool is_patch_v = false;

template <typename T>
inline constexpr bool
    is_patch_v<T, folly::void_t<typename T::underlying_type>> =
        std::is_base_of_v<detail::BasePatch<typename T::underlying_type, T>, T>;

template <typename T>
inline constexpr bool is_assign_only_patch_v = false;
template <typename T>
inline constexpr bool is_assign_only_patch_v<detail::AssignPatch<T>> = true;

template <typename T>
std::string prettyPrintPatch(
    const T& obj,
    DebugProtocolWriter::Options options =
        DebugProtocolWriter::Options::simple()) {
  static_assert(is_patch_v<T>, "Argument must be a Patch.");
  return debugStringViaEncode(obj, std::move(options));
}

} // namespace op
} // namespace thrift
} // namespace apache
