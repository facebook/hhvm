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

#include <fmt/core.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/op/PatchTraits.h>
#include <thrift/lib/cpp2/op/detail/Patch.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/gen-cpp2/patch_types.h>

namespace apache::thrift::op {

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
struct SafePatchType {};

template <class T>
struct SafePatchValueType {};

template <typename T>
using detect_safe_patch_value_type = typename SafePatchValueType<T>::type;

template <typename T, FieldId Id>
void checkFieldIsNotDeprecatedTerseWritesWithCustomDefault() {
  static_assert(
      !apache::thrift::detail::qualifier::
          is_deprecated_terse_writes_with_custom_default_field<
              T,
              op::get_field_id<T, type::field_id_tag<Id>>>::value,
      "Thrift Patch is not supported with fields with `deprecated_terse_writes` and custom default value.");
}

} // namespace detail

/// The safe patch represenations for the base thrift types.
///
/// Safe patch provides versioning to indicate the minimum Thrift Patch version
/// required to safely and successfully process that patch as well as opaque
/// storage that is resilient to Thrift schema compatibility.
template <typename T>
using safe_patch_type =
    typename detail::SafePatchType<type::infer_tag<T>>::type;

/// The value type for the safe patch.
template <typename T>
using safe_patch_value_type = typename detail::SafePatchValueType<T>::type;

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

template <typename T>
inline constexpr bool is_assign_only_patch_v = false;
template <typename T>
inline constexpr bool is_assign_only_patch_v<detail::AssignPatch<T>> = true;

template <typename T>
constexpr static bool is_safe_patch_v =
    folly::is_detected_v<detail::detect_safe_patch_value_type, T>;

/**
 * Returns a Thrift Patch instance corresponding to the (decoded) `SafePatch`.
 *
 * @throws std::runtime_error if the given `SafePatch` cannot be successfully
 * decoded or safely applied in this process (eg. if the version of the Thrift
 * Patch library in this process is not compatible with the minimum version
 * required by `SafePatch`).
 */
template <typename T, typename Tag = type::infer_tag<T>>
[[deprecated("Use fromSafePatch(...) method instead.")]] op::patch_type<Tag>
fromSafePatch(const op::safe_patch_type<Tag>& safePatch) {
  return op::patch_type<Tag>::fromSafePatch(safePatch);
}

/**
 * Returns a `SafePatch` instance corresponding to the encoded Thrift Patch.
 */
template <typename T, typename Tag = type::infer_tag<T>>
[[deprecated("Use toSafePatch(...) method instead.")]] op::safe_patch_type<Tag>
toSafePatch(const op::patch_type<Tag>& patch) {
  return patch.toSafePatch();
}

} // namespace apache::thrift::op
