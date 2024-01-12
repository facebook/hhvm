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
#include <utility>

#include <fatal/type/transform.h>
#include <folly/Traits.h>
#include <folly/dynamic.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

#include <thrift/lib/cpp2/reflection/internal/folly_dynamic-inl-pre.h>

/**
 * READ ME FIRST: this header enhances Thrift support for the `folly::dynamic`
 * container.
 *
 * Please refer to the top of `thrift/lib/cpp2/reflection/reflection.h` on how
 * to enable compile-time reflection for Thrift types. The present header relies
 * on it for its functionality.
 *
 * TROUBLESHOOTING:
 *  - make sure you've followed the instructions on `reflection.h` to enable
 *    generation of compile-time reflection;
 *  - make sure you've included the metadata for your Thrift types, as specified
 *    in `reflection.h`.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */

namespace apache {
namespace thrift {

/**
 * Describes the format of the data contained in the `folly::dynamic`.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
enum class dynamic_format {
  /**
   * A data format that's aimed at being more portable and easier to read by
   * humans.
   *
   * @author: Marcelo Juchem <marcelo@fb.com>
   */
  PORTABLE,

  /**
   * A data format that's compatible with `readFromJson()` and
   * `TSimpleJSONProtocol` from Thrift 1.
   *
   * @author: Marcelo Juchem <marcelo@fb.com>
   */
  JSON_1
};

/**
 * Tells how much a decoder should adhere to the data format specification.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
enum class format_adherence {
  /**
   * Demands the data to strictly follow the given format. Any deviation from
   * the format will be rejected.
   *
   * @author: Marcelo Juchem <marcelo@fb.com>
   */
  STRICT,

  /**
   * Accepts data that deviates from the format, as long as the deviation is not
   * ambiguous and can be safely interpreted by the decoder.
   *
   * @author: Marcelo Juchem <marcelo@fb.com>
   */
  LENIENT
};

/**
 * Converts an object to its `folly::dynamic` representation using Thrift's
 * reflection support.
 *
 * All Thrift types are required to be generated using the 'fatal' cpp2 flag,
 * otherwise compile-time reflection metadata won't be available.
 *
 * The root object is output to the given `folly::dynamic` output parameter.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename TC, typename T>
[[deprecated(
    "Use facebook::thrift::to_dynamic from <thrift/lib/cpp2/folly_dynamic/folly_dynamic.h>"
    "supporting adapters and not requiring old reflection")]] void
to_dynamic(folly::dynamic& out, T&& input, dynamic_format format) {
  using impl = apache::thrift::detail::dynamic_converter_impl<TC>;

  static_assert(
      fatal::is_complete<impl>::value, "to_dynamic: unsupported type");

  impl::to(out, std::forward<T>(input), format);
}
template <typename T>
[[deprecated(
    "Use facebook::thrift::to_dynamic from <thrift/lib/cpp2/folly_dynamic/folly_dynamic.h>"
    "supporting adapters and not requiring old reflection")]] void
to_dynamic(folly::dynamic& out, T&& input, dynamic_format format) {
  using TC = reflect_type_class_of_thrift_class<folly::remove_cvref_t<T>>;
  return to_dynamic<TC>(out, std::forward<T>(input), format);
}

/**
 * Converts an object to its `folly::dynamic` representation using Thrift's
 * reflection support.
 *
 * All Thrift types are required to be generated using the 'fatal' cpp2 flag,
 * otherwise compile-time reflection metadata won't be available.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename TC, typename T>
[[deprecated(
    "Use facebook::thrift::to_dynamic from <thrift/lib/cpp2/folly_dynamic/folly_dynamic.h>"
    "supporting adapters and not requiring old reflection")]] folly::dynamic
to_dynamic(T&& input, dynamic_format format) {
  folly::dynamic result(folly::dynamic::object);

  to_dynamic<TC>(result, std::forward<T>(input), format);

  return result;
}
template <typename T>
[[deprecated(
    "Use facebook::thrift::to_dynamic from <thrift/lib/cpp2/folly_dynamic/folly_dynamic.h>"
    "supporting adapters and not requiring old reflection")]] folly::dynamic
to_dynamic(T&& input, dynamic_format format) {
  folly::dynamic result(folly::dynamic::object);

  to_dynamic(result, std::forward<T>(input), format);

  return result;
}

/**
 * Converts an object from its `folly::dynamic` representation using Thrift's
 * reflection support.
 *
 * All Thrift types are required to be generated using the 'fatal' cpp2 flag,
 * otherwise compile-time reflection metadata won't be available.
 *
 * The decoded object is output to the given `out` parameter.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename TC, typename T>
[[deprecated(
    "Use facebook::thrift::from_dynamic from <thrift/lib/cpp2/folly_dynamic/folly_dynamic.h>"
    "supporting adapters and not requiring old reflection")]] void
from_dynamic(
    T& out,
    const folly::dynamic& input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) {
  using impl = apache::thrift::detail::dynamic_converter_impl<TC>;

  static_assert(
      fatal::is_complete<impl>::value, "from_dynamic: unsupported type");

  impl::from(out, input, format, adherence);
}
template <typename T>
[[deprecated(
    "Use facebook::thrift::from_dynamic from <thrift/lib/cpp2/folly_dynamic/folly_dynamic.h>"
    "supporting adapters and not requiring old reflection")]] void
from_dynamic(
    T& out,
    const folly::dynamic& input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) {
  using TC = reflect_type_class_of_thrift_class<folly::remove_cvref_t<T>>;
  from_dynamic<TC>(out, input, format, adherence);
}
template <typename TC, typename T>
void from_dynamic(
    T& out,
    folly::StringPiece input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) = delete;
template <typename T>
void from_dynamic(
    T& out,
    folly::StringPiece input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) = delete;

/**
 * Converts an object from its `folly::dynamic` representation using Thrift's
 * reflection support.
 *
 * All Thrift types are required to be generated using the 'fatal' cpp2 flag,
 * otherwise compile-time reflection metadata won't be available.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename TC, typename T>
[[deprecated(
    "Use facebook::thrift::from_dynamic from <thrift/lib/cpp2/folly_dynamic/folly_dynamic.h>"
    "supporting adapters and not requiring old reflection")]] T
from_dynamic(
    const folly::dynamic& input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) {
  T result;

  from_dynamic<TC>(result, input, format, adherence);

  return result;
}
template <typename T>
[[deprecated(
    "Use facebook::thrift::from_dynamic from <thrift/lib/cpp2/folly_dynamic/folly_dynamic.h>"
    "supporting adapters and not requiring old reflection")]] T
from_dynamic(
    const folly::dynamic& input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) {
  T result;

  from_dynamic(result, input, format, adherence);

  return result;
}
template <typename TC, typename T>
T from_dynamic(
    folly::StringPiece input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) = delete;
template <typename T>
T from_dynamic(
    folly::StringPiece input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) = delete;

} // namespace thrift
} // namespace apache

#include <thrift/lib/cpp2/reflection/internal/folly_dynamic-inl-post.h>
