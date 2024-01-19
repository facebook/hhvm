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

#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

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
  using Base::bad_op;
  using Base::ref;
  using Base::unimplemented;

  // TODO(afuller): Make this implicit-conversion safe.
  template <typename L, typename R>
  static folly::ordering compare(const L& lhs, const R& rhs) {
    return static_cast<folly::ordering>((lhs > rhs) - (rhs > lhs));
  }

  // TODO(afuller): Use saturating add.
  static bool add(T& self, const T& val) { return (self += val, true); }
  static bool add(void* s, const Dyn& v) { return add(ref(s), v.as<Tag>()); }

  static folly::partial_ordering compare(const void* lhs, const Dyn& rhs) {
    switch (rhs.type().baseType()) {
      case type::BaseType::Bool:
        return compare(ref(lhs), rhs.as<type::bool_t>());
      case type::BaseType::Byte:
        return compare(ref(lhs), rhs.as<type::byte_t>());
      case type::BaseType::I16:
        return compare(ref(lhs), rhs.as<type::i16_t>());
      case type::BaseType::I32:
        return compare(ref(lhs), rhs.as<type::i32_t>());
      case type::BaseType::I64:
        return compare(ref(lhs), rhs.as<type::i64_t>());
      case type::BaseType::Float:
        return compare(ref(lhs), rhs.as<type::float_t>());
      case type::BaseType::Double:
        return compare(ref(lhs), rhs.as<type::double_t>());
      default:
        bad_op();
    }
  }

  // TODO(afuller): Detect and 'handle' bad conversions.
  static void assign(void* lhs, const Dyn& rhs) {
    switch (rhs.type().baseType()) {
      case type::BaseType::Bool:
        ref(lhs) = static_cast<T>(rhs.as<type::bool_t>());
        return;
      case type::BaseType::Byte:
        ref(lhs) = static_cast<T>(rhs.as<type::byte_t>());
        return;
      case type::BaseType::I16:
        ref(lhs) = static_cast<T>(rhs.as<type::i16_t>());
        return;
      case type::BaseType::I32:
        ref(lhs) = static_cast<T>(rhs.as<type::i32_t>());
        return;
      case type::BaseType::I64:
        ref(lhs) = static_cast<T>(rhs.as<type::i64_t>());
        return;
      case type::BaseType::Float:
        ref(lhs) = static_cast<T>(rhs.as<type::float_t>());
        return;
      case type::BaseType::Double:
        ref(lhs) = static_cast<T>(rhs.as<type::double_t>());
        return;
      default:
        bad_op();
    }
  }
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

using StdTag = type::cpp_type<std::string, type::string_c>;
using IOBufTag = type::cpp_type<folly::IOBuf, type::string_c>;
using IOBufPtr = std::unique_ptr<folly::IOBuf>;
using IOBufPtrTag = type::cpp_type<IOBufPtr, type::string_c>;

struct StringCompare : folly::IOBufCompare {
  using folly::IOBufCompare::operator();
  folly::ordering operator()(fmt::string_view lhs, fmt::string_view rhs) const {
    return folly::to_ordering(lhs.compare(rhs));
  }
  folly::ordering operator()(
      fmt::string_view lhs, const folly::IOBuf& rhs) const {
    return operator()(
        folly::IOBuf::wrapBufferAsValue(lhs.data(), lhs.size()), rhs);
  }
  folly::ordering operator()(fmt::string_view lhs, const IOBufPtr& rhs) const {
    return operator()(
        folly::IOBuf::wrapBufferAsValue(lhs.data(), lhs.size()), *rhs);
  }
  folly::ordering operator()(
      const folly::IOBuf& lhs, fmt::string_view rhs) const {
    return operator()(
        lhs, folly::IOBuf::wrapBufferAsValue(rhs.data(), rhs.size()));
  }
  folly::ordering operator()(
      const folly::IOBuf& lhs, const IOBufPtr& rhs) const {
    return operator()(lhs, *rhs);
  }
  folly::ordering operator()(const IOBufPtr& lhs, fmt::string_view rhs) const {
    return operator()(
        *lhs, folly::IOBuf::wrapBufferAsValue(rhs.data(), rhs.size()));
  }
  folly::ordering operator()(
      const IOBufPtr& lhs, const folly::IOBuf& rhs) const {
    return operator()(*lhs, rhs);
  }
};

template <typename Tag>
struct StringOp : BaseOp<Tag> {
  using T = type::native_type<Tag>;
  using Base = BaseOp<Tag>;
  using Base::ref;
  using Base::unimplemented;

  static void assign(std::string& self, std::string val) {
    self = std::move(val);
  }
  static void assign(folly::IOBuf& self, const folly::IOBuf& val) {
    self = val;
  }
  static void assign(folly::IOBuf& self, const std::string& val) {
    // TODO(afuller): Use the existing buffer instead of a new heap allocation.
    self = std::move(*folly::IOBuf::copyBuffer(val.data(), val.length()));
  }
  static void assign(std::string& self, const folly::IOBuf& val) {
    assign(self, val.to<std::string>());
  }
  static void assign(std::string& self, const IOBufPtr& val) {
    assign(self, *val);
  }
  static void assign(folly::IOBuf& self, const IOBufPtr& val) {
    assign(self, *val);
  }
  template <typename T>
  static void assign(const IOBufPtr& self, T&& val) {
    assign(*self, std::forward<T>(val));
  }

  template <typename T1, typename T2>
  static std::unique_ptr<folly::IOBuf> concat(T1&& first, T2&& second) {
    folly::IOBufQueue builder;
    builder.append(std::forward<T1>(first));
    builder.append(std::forward<T2>(second));
    return builder.move();
  }

  static void append(std::string& self, const std::string& val) { self += val; }
  static void append(std::string& self, const folly::IOBuf& val) {
    val.appendTo(self);
  }
  static void append(std::string& self, const IOBufPtr& val) {
    append(self, *val);
  }
  template <typename T>
  static void append(folly::IOBuf& self, T&& val) {
    self = std::move(*concat(std::move(self), std::forward<T>(val)));
  }
  static void append(folly::IOBuf& self, const IOBufPtr& val) {
    append(self, *val);
  }
  template <typename T>
  static void append(IOBufPtr& self, T&& val) {
    self = concat(std::move(self), std::forward<T>(val));
  }
  static void append(IOBufPtr& self, const IOBufPtr& val) {
    append(self, *val);
  }

  static void prepend(std::string& self, std::string val) {
    self = std::move(val) + std::move(self);
  }
  static void prepend(std::string& self, const folly::IOBuf& val) {
    prepend(self, val.to<std::string>());
  }
  static void prepend(std::string& self, const IOBufPtr& val) {
    prepend(self, *val);
  }
  template <typename T>
  static void prepend(folly::IOBuf& self, T&& val) {
    self = std::move(*concat(std::forward<T>(val), std::move(self)));
  }
  static void prepend(folly::IOBuf& self, const IOBufPtr& val) {
    prepend(self, *val);
  }
  template <typename T>
  static void prepend(IOBufPtr& self, T&& val) {
    self = concat(std::forward<T>(val), std::move(self));
  }
  static void prepend(IOBufPtr& self, const IOBufPtr& val) {
    prepend(self, *val);
  }

#define TRY_OP(ACCUM_TO, TRY_ON, TAG, OP)           \
  if (const auto* ptr##TAG = TRY_ON.tryAs<TAG>()) { \
    return OP(ref(ACCUM_TO), *ptr##TAG);            \
  }

  static folly::partial_ordering compare(const void* lhs, const Dyn& rhs) {
    StringCompare cmp;
    // TODO(afuller): Consider using a ~map.
    TRY_OP(lhs, rhs, Tag, cmp);
    TRY_OP(lhs, rhs, StdTag, cmp);
    TRY_OP(lhs, rhs, IOBufTag, cmp);
    TRY_OP(lhs, rhs, IOBufPtrTag, cmp);
    // TODO(afuller): Implement compatibility with any type convertable to
    // fmt::string_view.
    unimplemented();
  }

  static void assign(void* s, const Dyn& val) {
    TRY_OP(s, val, Tag, assign);
    TRY_OP(s, val, StdTag, assign);
    TRY_OP(s, val, IOBufTag, assign);
    TRY_OP(s, val, IOBufPtrTag, assign);
    // TODO(afuller): Implement compatibility with any type convertable to
    // fmt::string_view.
    unimplemented();
  }

  static void prepend(void* s, const Dyn& val) {
    TRY_OP(s, val, Tag, prepend);
    TRY_OP(s, val, StdTag, prepend);
    TRY_OP(s, val, IOBufTag, prepend);
    TRY_OP(s, val, IOBufPtrTag, prepend);
    // TODO(afuller): Implement compatibility with any type convertable to
    // fmt::string_view.
    unimplemented();
  }

  static void append(void* s, const Dyn& val) {
    TRY_OP(s, val, Tag, append);
    TRY_OP(s, val, StdTag, append);
    TRY_OP(s, val, IOBufTag, append);
    TRY_OP(s, val, IOBufPtrTag, append);
    // TODO(afuller): Implement compatibility with any type convertable to
    // fmt::string_view.
    unimplemented();
  }
  static bool add(void* s, const Dyn& val) { return (append(s, val), true); }
};

template <>
struct AnyOp<type::string_t> : StringOp<type::string_t> {};
template <typename T>
struct AnyOp<type::cpp_type<T, type::string_t>>
    : StringOp<type::cpp_type<T, type::string_t>> {};
template <>
struct AnyOp<type::binary_t> : StringOp<type::binary_t> {};
template <typename T>
struct AnyOp<type::cpp_type<T, type::binary_t>>
    : StringOp<type::cpp_type<T, type::binary_t>> {};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
