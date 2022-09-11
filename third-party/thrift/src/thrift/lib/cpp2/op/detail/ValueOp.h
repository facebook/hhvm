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

#include <iterator>
#include <stdexcept>

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
  using Base::ref;

  static bool add(T& self, const T& val) { return (self += val, true); }
  static bool add(void* s, const Dyn& v) { return add(ref(s), v.as<Tag>()); }
};

template <typename Tag>
struct FloatOp : NumberOp<Tag> {
  using T = type::native_type<Tag>;
  using Base = BaseOp<Tag>;
  using Base::ref;
  using Base::unimplemented;

  static partial_ordering compare(const void* lhs, const Dyn& rhs) {
    if (const T* ptr = rhs.tryAs<Tag>()) {
      return to_partial_ordering(op::compare<Tag>(ref(lhs), *ptr));
    }

    // Widen the type for inter op with other floats.
    if (const auto* ptr = rhs.tryAs<type::float_t>()) {
      return to_partial_ordering(op::compare<type::double_t>(ref(lhs), *ptr));
    } else if (const auto* ptr = rhs.tryAs<type::double_t>()) {
      return to_partial_ordering(op::compare<type::double_t>(ref(lhs), *ptr));
    }

    // TODO(afuller): Implement compatibility with integer types.
    unimplemented();
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
struct AnyOp<type::float_t> : FloatOp<type::float_t> {};
template <>
struct AnyOp<type::double_t> : FloatOp<type::double_t> {};

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
