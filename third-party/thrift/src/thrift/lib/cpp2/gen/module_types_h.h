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
#include <initializer_list>
#include <memory>
#include <shared_mutex>
#include <type_traits>

#include <folly/CPortability.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/synchronization/Lock.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/TypeClass.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/TableBasedForwardTypes.h>

#if !FOLLY_MOBILE
#include <folly/SharedMutex.h>
#else
#endif

#ifdef SWIG
#error SWIG
#endif

//  all members are logically private to fbthrift; external use is deprecated
#define APACHE_THRIFT_DEFINE_ACCESSOR(name)                  \
  template <>                                                \
  struct invoke_reffer<::apache::thrift::ident::name> {      \
    template <typename T>                                    \
    FOLLY_ERASE constexpr auto operator()(T&& t) const       \
        noexcept(noexcept(static_cast<T&&>(t).name##_ref())) \
            -> decltype(static_cast<T&&>(t).name##_ref()) {  \
      return static_cast<T&&>(t).name##_ref();               \
    }                                                        \
  }

namespace apache {
namespace thrift {
namespace detail {

template <typename T>
constexpr ptrdiff_t fieldOffset(std::int16_t fieldIndex);
template <typename T>
constexpr ptrdiff_t issetOffset(std::int16_t fieldIndex);
template <typename T>
constexpr ptrdiff_t unionTypeOffset();

template <typename Ident, typename Adapter, FieldId Id, typename Ref>
struct wrapped_struct_argument {
  static_assert(std::is_reference<Ref>::value, "not a reference");
  Ref ref;
  FOLLY_ERASE explicit wrapped_struct_argument(Ref ref_)
      : ref(static_cast<Ref>(ref_)) {}
};

template <typename Ident, typename Adapter, FieldId Id, typename T>
struct wrapped_struct_argument<Ident, Adapter, Id, std::initializer_list<T>> {
  std::initializer_list<T> ref;
  FOLLY_ERASE explicit wrapped_struct_argument(std::initializer_list<T> list)
      : ref(list) {}
};

template <
    typename Ident,
    typename Adapter = void,
    FieldId Id = static_cast<FieldId>(0),
    typename T>
FOLLY_ERASE
    wrapped_struct_argument<Ident, Adapter, Id, std::initializer_list<T>>
    wrap_struct_argument(std::initializer_list<T> value) {
  return wrapped_struct_argument<Ident, Adapter, Id, std::initializer_list<T>>(
      value);
}

template <
    typename Ident,
    typename Adapter = void,
    FieldId Id = static_cast<FieldId>(0),
    typename T>
FOLLY_ERASE wrapped_struct_argument<Ident, Adapter, Id, T&&>
wrap_struct_argument(T&& value) {
  return wrapped_struct_argument<Ident, Adapter, Id, T&&>(
      static_cast<T&&>(value));
}

template <typename Adapter, FieldId Id, typename F, typename T, typename S>
FOLLY_ERASE typename std::enable_if<std::is_void<Adapter>::value>::type
assign_struct_field(F f, T&& t, S&) {
  f = static_cast<T&&>(t);
}
template <typename Adapter, FieldId Id, typename F, typename T, typename S>
FOLLY_ERASE void assign_struct_field(std::unique_ptr<F>& f, T&& t, S&) {
  f = std::make_unique<folly::remove_cvref_t<T>>(static_cast<T&&>(t));
}
template <typename Adapter, FieldId Id, typename F, typename T, typename S>
FOLLY_ERASE void assign_struct_field(std::shared_ptr<F>& f, T&& t, S&) {
  f = std::make_shared<folly::remove_cvref_t<T>>(static_cast<T&&>(t));
}
template <typename Adapter, FieldId Id, typename F, typename T, typename S>
FOLLY_ERASE typename std::enable_if<!std::is_void<Adapter>::value>::type
assign_struct_field(F f, T&& t, S& s) {
  f = ::apache::thrift::adapt_detail::
      fromThriftField<Adapter, folly::to_underlying(Id)>(
          static_cast<T&&>(t), s);
}

template <
    typename Struct,
    typename... Ident,
    typename... Adapter,
    FieldId... Id,
    typename... T>
FOLLY_ERASE constexpr Struct make_structured_constant(
    wrapped_struct_argument<Ident, Adapter, Id, T>... arg) {
  using _ = int[];
  Struct s;
  void(
      _{0,
        (void(assign_struct_field<Adapter, Id>(
             access_field<Ident>(s), static_cast<T>(arg.ref), s)),
         0)...});
  return s;
}

// TODO(ytj): Remove after migrating existing callsites
template <typename S, typename... A, typename... T, typename TypeClass>
FOLLY_ERASE constexpr S make_constant(
    TypeClass,
    wrapped_struct_argument<A, void, static_cast<FieldId>(0), T>... arg) {
  return make_structured_constant<S>(arg...);
}

template <typename T, std::enable_if_t<st::IsThriftClass<T>{}, int> = 0>
constexpr bool operator!=(const T& lhs, const T& rhs) {
  return !(lhs == rhs);
}
template <typename T, std::enable_if_t<st::IsThriftClass<T>{}, int> = 0>
constexpr bool operator>(const T& lhs, const T& rhs) {
  return rhs < lhs;
}
template <typename T, std::enable_if_t<st::IsThriftClass<T>{}, int> = 0>
constexpr bool operator<=(const T& lhs, const T& rhs) {
  return !(rhs < lhs);
}
template <typename T, std::enable_if_t<st::IsThriftClass<T>{}, int> = 0>
constexpr bool operator>=(const T& lhs, const T& rhs) {
  return !(lhs < rhs);
}

enum class IssetBitsetOption {
  Unpacked,
  Packed,
  PackedWithAtomic,
};

template <
    size_t NumBits,
    IssetBitsetOption kOption = IssetBitsetOption::Unpacked>
class isset_bitset {
 private:
  using IntType = std::conditional_t<
      kOption == IssetBitsetOption::PackedWithAtomic,
      std::atomic<uint8_t>,
      uint8_t>;

 public:
  bool get(size_t field_index) const {
    check(field_index);
    return array_isset[field_index / kBits][field_index % kBits];
  }

  void set(size_t field_index, bool isset_flag) {
    check(field_index);
    array_isset[field_index / kBits][field_index % kBits] = isset_flag;
  }

  const IntType& at(size_t field_index) const {
    check(field_index);
    return array_isset[field_index / kBits].value();
  }

  IntType& at(size_t field_index) {
    check(field_index);
    return array_isset[field_index / kBits].value();
  }

  uint8_t bit(size_t field_index) const {
    check(field_index);
    return field_index % kBits;
  }

  static constexpr ptrdiff_t get_offset() {
    return offsetof(isset_bitset, array_isset);
  }

 private:
  static void check(size_t field_index) {
    DCHECK(field_index / kBits < NumBits);
  }

  static constexpr size_t kBits =
      kOption == IssetBitsetOption::Unpacked ? 1 : 8;
  std::array<
      apache::thrift::detail::BitSet<IntType>,
      (NumBits + kBits - 1) / kBits>
      array_isset;
};

namespace st {

#if !FOLLY_MOBILE
using DeserializationMutex = folly::SharedMutex;
#else
using DeserializationMutex = std::shared_timed_mutex; // C++14
#endif

} // namespace st

} // namespace detail
namespace ident {
template <class T>
void __fbthrift_check_whether_type_is_ident_via_adl(T&&);
}
} // namespace thrift
} // namespace apache
