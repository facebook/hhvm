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

#include <folly/Portability.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/detail/Compare.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>

namespace apache::thrift::type::detail {

// A base class the "curiously reoccurring template pattern" with helpers that
// propagate const&-ness.
template <typename D>
class BaseDerived {
 protected:
  constexpr D& derived() & noexcept { return static_cast<D&>(*this); }
  constexpr D&& derived() && noexcept { return static_cast<D&&>(*this); }
  constexpr const D& derived() const& noexcept {
    return static_cast<const D&>(*this);
  }
  constexpr const D&& derived() const&& noexcept {
    return static_cast<const D&&>(*this);
  }
};

// A base class for a wrapper around a Thrift value.
//
// Specifically designed to work with thrift::InlineAdapter.
//
// TODO(afuller): Consider expanding features and making public.
template <typename T, typename Tag = infer_tag<T>>
class Wrap {
 public:
  static_assert(std::is_same<native_type<Tag>, T>::value);

  using underlying_type = T;
  using underlying_tag = Tag;

  Wrap() = default;
  Wrap(const Wrap&) = default;
  Wrap(Wrap&&) noexcept = default;
  Wrap& operator=(const Wrap&) = default;
  Wrap& operator=(Wrap&&) noexcept = default;

  explicit Wrap(const underlying_type& data) : data_(data) {}
  explicit Wrap(underlying_type&& data) noexcept : data_(std::move(data)) {}

  FOLLY_NODISCARD underlying_type& toThrift() & { return data_; }
  FOLLY_NODISCARD const underlying_type& toThrift() const& { return data_; }
  FOLLY_NODISCARD underlying_type&& toThrift() && { return std::move(data_); }
  FOLLY_NODISCARD const underlying_type&& toThrift() const&& {
    return std::move(data_);
  }

  void reset() { op::clear<underlying_tag>(data_); }
  bool empty() const { return op::isEmpty<underlying_tag>(data_); }
  template <typename Protocol>
  std::uint32_t encode(Protocol& prot) const {
    return op::encode<underlying_tag>(prot, data_);
  }

 protected:
  T data_;

  // Gets the field reference, for the given id, in the underlying data.
  template <typename Id>
  constexpr decltype(auto) get() & {
    return op::get<Id>(data_);
  }
  template <typename Id>
  constexpr decltype(auto) get(Id) & {
    return get<Id>();
  }
  template <typename Id>
  constexpr decltype(auto) get() && {
    return op::get<Id>(std::move(data_));
  }
  template <typename Id>
  constexpr decltype(auto) get(Id) && {
    return get<Id>();
  }
  template <typename Id>
  constexpr decltype(auto) get() const& {
    return op::get<Id>(data_);
  }
  template <typename Id>
  constexpr decltype(auto) get(Id) const& {
    return get<Id>();
  }
  template <typename Id>
  constexpr decltype(auto) get() const&& {
    return op::get<Id>(std::move(data_));
  }
  template <typename Id>
  constexpr decltype(auto) get(Id) const&& {
    return get<Id>();
  }

  ~Wrap() = default; // abstract base class

  // A fluent version of 'reset()'.
  FOLLY_NODISCARD T& resetAnd() { return (reset(), data_); }
};

template <typename Derived, typename T, typename Tag = struct_t<T>>
class EqWrap : public Wrap<T, Tag>, public BaseDerived<Derived> {
  using Base = Wrap<T, Tag>;

 public:
  using Base::Base;

  EqWrap(const EqWrap&) = default;
  EqWrap(EqWrap&&) noexcept = default;
  EqWrap& operator=(const EqWrap&) = default;
  EqWrap& operator=(EqWrap&&) noexcept = default;

 protected:
  // Down-cast to access protected member.
  static const T& data(const Derived& val) {
    return static_cast<const EqWrap&>(val).data_;
  }
  static T& data(Derived& val) { return static_cast<EqWrap&>(val).data_; }
  static T&& data(Derived&& val) { return std::move(data(val)); }

  ~EqWrap() = default; // abstract base class

 private:
  friend constexpr bool operator==(const Derived& lhs, const Derived& rhs) {
    return op::equal<Tag>(data(lhs), data(rhs));
  }
  friend constexpr bool operator!=(const Derived& lhs, const Derived& rhs) {
    return !(lhs == rhs);
  }
};

// The custom specialization of std::hash injected into the std namespace.
#define FBTHRIFT_STD_HASH_WRAP_DATA(fullType)                        \
  namespace std {                                                    \
  template <>                                                        \
  struct hash<::fullType> {                                          \
    size_t operator()(const ::fullType& value) const noexcept {      \
      return ::apache::thrift::op::hash<::fullType::underlying_tag>( \
          value.toThrift());                                         \
    }                                                                \
  };                                                                 \
  } // namespace std

} // namespace apache::thrift::type::detail
