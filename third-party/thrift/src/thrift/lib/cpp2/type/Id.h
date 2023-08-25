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

// Types use for identifying Thrift definitions.
//
// Thrift supports a number of different types of identifiers, including
//  - FieldId - The numeric identifier for a field.
//  - Ident - The assigned 'name' for a definition.
//  - Ordinal - The 1-based order at which a definition was defined in the
//  IDL/AST.
#pragma once

#include <cstdint>
#include <stdexcept>
#include <type_traits>

#include <fmt/core.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache {
namespace thrift {
namespace type {

// Runtime and compile time representations for an ordinal.
enum class Ordinal : uint32_t {};
template <Ordinal ord>
using ordinal_tag = std::integral_constant<Ordinal, ord>;
template <std::underlying_type_t<Ordinal> ord>
using ordinal = ordinal_tag<Ordinal(ord)>;
template <typename Id>
FOLLY_INLINE_VARIABLE constexpr bool is_ordinal_v = false;
template <Ordinal ord>
FOLLY_INLINE_VARIABLE constexpr bool
    is_ordinal_v<std::integral_constant<Ordinal, ord>> = true;

// Runtime and compile time representations for a field id.
enum class FieldId : int16_t; // defined in id.thrift
template <FieldId id>
using field_id_tag = std::integral_constant<FieldId, id>;
template <std::underlying_type_t<FieldId> id>
using field_id = field_id_tag<FieldId(id)>;
template <typename Id>
FOLLY_INLINE_VARIABLE constexpr bool is_field_id_v = false;
template <FieldId id>
FOLLY_INLINE_VARIABLE constexpr bool
    is_field_id_v<std::integral_constant<FieldId, id>> = true;

// Helpers for ident tags defined in code gen.
template <typename Id, typename = void>
FOLLY_INLINE_VARIABLE constexpr bool is_ident_v = false;
template <typename Id>
FOLLY_INLINE_VARIABLE constexpr bool is_ident_v<
    Id,
    decltype(__fbthrift_check_whether_type_is_ident_via_adl(
        FOLLY_DECLVAL(Id &&)))> = true;

namespace detail {
// We can't use std::is_base_of since it's undefined behavior to use it on
// incomplete type, and ident is incomplete type.
void is_type_tag_impl(all_c);
void is_type_tag_impl(void_t);
void is_type_tag_impl(service_c);
template <class Id, class = void>
FOLLY_INLINE_VARIABLE constexpr bool is_type_tag_v = false;
template <class Id>
FOLLY_INLINE_VARIABLE constexpr bool
    is_type_tag_v<Id, decltype(is_type_tag_impl(FOLLY_DECLVAL(Id &&)))> = true;

// TODO(afuller): Static assert bounds check.
template <size_t pos>
using pos_to_ordinal = ordinal<pos + 1>;
} // namespace detail

// If the given type can be used to identify a definition.
template <typename Id>
FOLLY_INLINE_VARIABLE constexpr bool is_id_v = detail::is_type_tag_v<Id> ||
    is_field_id_v<Id> || is_ordinal_v<Id> || is_ident_v<Id>;

template <typename Id, typename R = void>
using if_id = std::enable_if_t<is_id_v<Id>, R>;
template <typename Id, typename R = void>
using if_not_id = std::enable_if_t<!is_id_v<Id>, R>;

/**
 * Converts a value into an ordinal. Useful for represending e.g. list indexes
 * allowing 0 represent all elements in the list.
 *
 * @param pos to convert to ordinal
 * @return ordinal
 */
inline constexpr Ordinal toOrdinal(size_t pos) {
  if (pos == std::numeric_limits<size_t>::max()) {
    return Ordinal{};
  }

  constexpr auto max_size = static_cast<size_t>(
      std::numeric_limits<std::underlying_type_t<Ordinal>>::max());
  if (pos >= max_size) {
    folly::throw_exception<std::out_of_range>(
        fmt::format("max size supported is {}", max_size));
  }
  return Ordinal(pos + 1);
}

/**
 * Does reverse conversion from ordinal obtained by toOrdinal()
 *
 * @param ordinal to convert back to integer
 * @param max if specified, ensures return value is below this number (throwing
 *   std::out_of_range), e.g. for bounds checking before container access
 * @return integer
 */
inline constexpr size_t toPosition(Ordinal ordinal, size_t max = 0) {
  auto val = static_cast<size_t>(ordinal);
  if (max && val > max) {
    folly::throw_exception<std::out_of_range>(
        fmt::format("max size supported is {}", max));
  }
  return val - 1;
}

} // namespace type
} // namespace thrift
} // namespace apache
