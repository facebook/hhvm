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

#include <folly/dynamic.h>
#include <thrift/lib/cpp2/type/NativeType.h>

#include <thrift/lib/cpp2/folly_dynamic/internal/folly_dynamic-inl-pre.h>

namespace facebook {
namespace thrift {

/**
 * Describes the format of the data contained in the `folly::dynamic`.
 */
enum class dynamic_format {
  /**
   * A data format that's aimed at being more portable and easier to read by
   * humans.
   */
  PORTABLE,

  /**
   * A data format that's compatible with `readFromJson()` and
   * `TSimpleJSONProtocol` from Thrift 1.
   */
  JSON_1
};

/**
 * Tells how much a decoder should adhere to the data format specification.
 */
enum class format_adherence {
  /**
   * Demands the data to strictly follow the given format. Any deviation from
   * the format will be rejected.
   */
  STRICT,

  /**
   * Accepts data that deviates from the format, as long as the deviation is not
   * ambiguous and can be safely interpreted by the decoder.
   */
  LENIENT
};

/**
 * Converts an object to its `folly::dynamic` representation using Thrift's
 * always-on reflection support.
 *
 * The root object is output to the given `folly::dynamic` output parameter.
 */
template <typename Tag, typename T>
void to_dynamic(folly::dynamic& out, T&& input, dynamic_format format) {
  detail::dynamic_converter_impl<Tag>::to(out, std::forward<T>(input), format);
}
template <typename T>
void to_dynamic(folly::dynamic& out, T&& input, dynamic_format format) {
  using Tag = apache::thrift::type::infer_tag<T>;
  return to_dynamic<Tag>(out, std::forward<T>(input), format);
}

/**
 * Converts an object to its `folly::dynamic` representation using Thrift's
 * always-on reflection support.
 */
template <typename Tag, typename T>
folly::dynamic to_dynamic(T&& input, dynamic_format format) {
  folly::dynamic result(folly::dynamic::object);

  to_dynamic<Tag>(result, std::forward<T>(input), format);

  return result;
}
template <typename T>
folly::dynamic to_dynamic(T&& input, dynamic_format format) {
  folly::dynamic result(folly::dynamic::object);

  to_dynamic(result, std::forward<T>(input), format);

  return result;
}

/**
 * Converts an object from its `folly::dynamic` representation using Thrift's
 * always-on reflection support.

 * NOTE: this function doesn't clear the output parameter prior decoding and
 requires a cleared object to be passed in
 *
 * The decoded object is output to the given `out` parameter.
 */
template <typename Tag, typename T>
void from_dynamic(
    T& out,
    const folly::dynamic& input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) {
  detail::dynamic_converter_impl<Tag>::from(out, input, format, adherence);
}
template <typename T>
void from_dynamic(
    T& out,
    const folly::dynamic& input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) {
  using Tag = apache::thrift::type::infer_tag<T>;
  from_dynamic<Tag, T>(out, input, format, adherence);
}
template <typename Tag, typename T>
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
 * always-on reflection support.
 */
template <typename Tag, typename T>
T from_dynamic(
    const folly::dynamic& input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) {
  T result;

  detail::dynamic_converter_impl<Tag>::from(result, input, format, adherence);

  return result;
}
template <typename T>
T from_dynamic(
    const folly::dynamic& input,
    dynamic_format format,
    format_adherence adherence = format_adherence::STRICT) {
  using Tag = apache::thrift::type::infer_tag<T>;
  return from_dynamic<Tag, T>(input, format, adherence);
}
template <typename Tag, typename T>
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
} // namespace facebook

#include <thrift/lib/cpp2/folly_dynamic/internal/folly_dynamic-inl-post.h>
