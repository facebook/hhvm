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
  static partial_ordering compare(const L& lhs, const R& rhs) {
    return partial_ordering((lhs > rhs) - (rhs > lhs));
  }

  // TODO(afuller): Use saturating add.
  static bool add(T& self, const T& val) { return (self += val, true); }
  static bool add(void* s, const Dyn& v) { return add(ref(s), v.as<Tag>()); }

  static partial_ordering compare(const void* lhs, const Dyn& rhs) {
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
  folly::ordering operator()(
      fmt::string_view lhs, const std::unique_ptr<IOBuf>& rhs) const {
    return operator()(
        folly::IOBuf::wrapBufferAsValue(lhs.data(), lhs.size()), *rhs);
  }
  folly::ordering operator()(
      const folly::IOBuf& lhs, fmt::string_view rhs) const {
    return operator()(
        lhs, folly::IOBuf::wrapBufferAsValue(rhs.data(), rhs.size()));
  }
  folly::ordering operator()(
      const folly::IOBuf& lhs, const std::unique_ptr<IOBuf>& rhs) const {
    return operator()(lhs, *rhs);
  }
  folly::ordering operator()(
      const std::unique_ptr<IOBuf>& lhs, fmt::string_view rhs) const {
    return operator()(
        *lhs, folly::IOBuf::wrapBufferAsValue(rhs.data(), rhs.size()));
  }
  folly::ordering operator()(
      const std::unique_ptr<IOBuf>& lhs, const folly::IOBuf& rhs) const {
    return operator()(*lhs, rhs);
  }
};

using StdTag = type::cpp_type<std::string, type::string_c>;
using IOBufTag = type::cpp_type<folly::IOBuf, type::string_c>;
using IOBufPtrTag = type::cpp_type<std::unique_ptr<IOBuf>, type::string_c>;

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
    self = *folly::IOBuf::copyBuffer(val.data(), val.length());
  }
  static void assign(std::string& self, const folly::IOBuf& val) {
    assign(self, val.to<std::string>());
  }
  static void assign(
      std::string& self, const std::unique_ptr<folly::IOBuf>& val) {
    assign(self, *val);
  }
  static void assign(
      folly::IOBuf& self, const std::unique_ptr<folly::IOBuf>& val) {
    assign(self, *val);
  }
  template <typename T>
  static void assign(const std::unique_ptr<folly::IOBuf>& self, T&& val) {
    assign(*self, std::forward<T>(val));
  }

  static partial_ordering compare(const void* lhs, const Dyn& rhs) {
    StringCompare cmp;
    // TODO(afuller): Consider using a ~map.
    if (const T* ptr = rhs.tryAs<Tag>()) {
      return to_partial_ordering(cmp(ref(lhs), *ptr));
    } else if (const auto* ptr = rhs.tryAs<StdTag>()) {
      return to_partial_ordering(cmp(ref(lhs), *ptr));
    } else if (const auto* ptr = rhs.tryAs<IOBufTag>()) {
      return to_partial_ordering(cmp(ref(lhs), *ptr));
    } else if (const auto* ptr = rhs.tryAs<IOBufPtrTag>()) {
      return to_partial_ordering(cmp(ref(lhs), **ptr));
    }
    // TODO(afuller): Implement compatibility with any type convertable to
    // fmt::string_view.
    unimplemented();
  }

  static void assign(void* s, const Dyn& val) {
    if (const T* ptr = val.tryAs<Tag>()) {
      return assign(ref(s), *ptr);
    } else if (const auto* ptr = val.tryAs<StdTag>()) {
      return assign(ref(s), *ptr);
    } else if (const auto* ptr = val.tryAs<IOBufTag>()) {
      return assign(ref(s), *ptr);
    } else if (const auto* ptr = val.tryAs<IOBufPtrTag>()) {
      return assign(ref(s), *ptr);
    }
    // TODO(afuller): Implement compatibility with any type convertable to
    // fmt::string_view.
    unimplemented();
  }
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
