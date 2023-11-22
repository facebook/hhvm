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

#include <any>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <shared_mutex>
#include <type_traits>
#include <vector>

#include <folly/CPortability.h>
#include <folly/Indestructible.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/synchronization/Lock.h>
#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/Adapter.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/TypeClass.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/TableBasedForwardTypes.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/Tag.h>

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

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

template <class List, FieldOrdinal>
struct at_impl {
  static_assert(sizeof(List) < 0, "");
};

template <class... Args, FieldOrdinal Ord>
struct at_impl<folly::tag_t<Args...>, Ord> {
  using type =
      folly::type_pack_element_t<folly::to_underlying(Ord), void, Args...>;
};

// Similar to mp_at in boost mp11, but Ordinal based
template <class List, FieldOrdinal Ord>
using at = typename at_impl<List, Ord>::type;

// Similar to std::find, but returns ordinal
template <class T>
FOLLY_CONSTEVAL type::Ordinal findOrdinal(
    const T* first, const T* last, const T& value) {
  for (const T* iter = first; iter != last; ++iter) {
    if (*iter == value) {
      return static_cast<type::Ordinal>(iter - first + 1);
    }
  }

  return static_cast<type::Ordinal>(0);
}

template <class T, class List>
class FindOrdinal {
  static_assert(sizeof(T) < 0, "");
};

template <class T, class... Args>
class FindOrdinal<T, folly::tag_t<Args...>> {
 private:
  static constexpr bool matches[sizeof...(Args)] = {
      std::is_same<T, Args>::value...};

 public:
  static constexpr auto value = findOrdinal(matches, std::end(matches), true);
  static FOLLY_CONSTEVAL size_t count() {
    size_t count = 0;
    for (bool b : matches) {
      count += b;
    }
    return count;
  }
};

template <class T, class List>
inline constexpr type::Ordinal FindOrdinalInUniqueTypes =
    FindOrdinal<T, List>::value;

#if defined(__clang__) && \
    !defined(THRIFT_DISABLE_REFLECTION_MULTIWAY_LOOKUP_OPTIMIZATION)
// For now only enable for __clang__ due to bugs in MSVC/GCC

template <int>
struct IntTag {};

#define FBTHRIFT_LOOKUP_SIZE 63
#define FBTHRIFT_PARAMS_WITH_DEFAULT(Z, NUM, TEXT) \
  BOOST_PP_COMMA_IF(NUM) class A##NUM = IntTag<NUM>

//  The struct is defined like this
//
//    template<
//      class A0 = IntTag<0>,
//      class A1 = IntTag<1>,
//      ...,
//      class AN = IntTag<N>>
//    struct MultiWayLookup {
//      template <class> static const int value = 0;
//      template <> static const int value<A0> = 0 + 1;
//      template <> static const int value<A1> = 1 + 1;
//      ...;
//      template <> static const int value<AN> = N + 1;
//    }
//
// So that MultiWayLookup<Args...>::value<T> returns Ordinal of T in [Args...]
// list with O(1) lookup time (instead of traditionally scanning [Args...])
template <BOOST_PP_REPEAT(FBTHRIFT_LOOKUP_SIZE, FBTHRIFT_PARAMS_WITH_DEFAULT, )>
struct MultiWayLookup {
  template <class>
  static const int value = 0;

#define FBTHRIFT_DEFINE_VALUE_SPECIALIZATION(Z, NUM, TEXT) \
  template <>                                              \
  static const int value<A##NUM> = NUM + 1;

  BOOST_PP_REPEAT(FBTHRIFT_LOOKUP_SIZE, FBTHRIFT_DEFINE_VALUE_SPECIALIZATION, )

#undef FBTHRIFT_DEFINE_VALUE_SPECIALIZATION
};
#undef FBTHRIFT_TEMPLATE_PARAMS_WITH_DEFAULT

// Find type T in list [Args...].
// FindOrdinalInUniqueTypesImpl::value will be the ordinal of T in [Args...]
// If not found, value will be Ordinal{0}
//
// Limitation: [Args...] can not have duplicated type, otherwise build failure.
template <class T, class... Args>
struct FindOrdinalInUniqueTypesImpl;

// If Found is not 0, we have found T in Args...
// In which case value = Found
template <int Found, class... Args>
struct FoundOrdinalOrCheckTheRest : field_ordinal<Found> {};

// Otherwise check the rest
// If found, value = FindOrdinalInUniqueTypesImpl<Args...>::value + SIZE
// If not found, value = 0
template <type::Ordinal ord>
using Rest = field_ordinal<
    ord == static_cast<type::Ordinal>(0)
        ? 0
        : folly::to_underlying(ord) + FBTHRIFT_LOOKUP_SIZE>;

template <class... Args>
struct FoundOrdinalOrCheckTheRest<0, Args...>
    : Rest<FindOrdinalInUniqueTypesImpl<Args...>::value> {};

#define FBTHRIFT_TEMPLATE_PARAMS(Z, NUM, TEXT) \
  BOOST_PP_COMMA_IF(NUM) TEXT A##NUM

// This macro will be expanded to "A0, A1, A2, ..., AN"
#define FBTHRIFT_PARAMS \
  BOOST_PP_REPEAT(FBTHRIFT_LOOKUP_SIZE, FBTHRIFT_TEMPLATE_PARAMS, )

// This macro will be expanded to "class A0, class A1, class A2, ..., class AN"
#define FBTHRIFT_PARAMS_WITH_CLASS \
  BOOST_PP_REPEAT(FBTHRIFT_LOOKUP_SIZE, FBTHRIFT_TEMPLATE_PARAMS, class)

// If size of (Args...) < FBTHRIFT_LOOKUP_SIZE: just do a multiway lookup
template <class T, class... Args>
struct FindOrdinalInUniqueTypesImpl
    : field_ordinal<MultiWayLookup<Args...>::template value<T>> {};

// If size of (Args...) > FBTHRIFT_LOOKUP_SIZE: used batched multiway lookup
template <class T, FBTHRIFT_PARAMS_WITH_CLASS, class... Args>
struct FindOrdinalInUniqueTypesImpl<T, FBTHRIFT_PARAMS, Args...>
    : FoundOrdinalOrCheckTheRest<
          MultiWayLookup<FBTHRIFT_PARAMS>::template value<T>,
          T,
          Args...> {};

#undef FBTHRIFT_TEMPLATE_PARAMS
#undef FBTHRIFT_PARAMS
#undef FBTHRIFT_PARAMS_WITH_CLASS
#undef FBTHRIFT_LOOKUP_SIZE

template <class T, class... Args>
inline constexpr type::Ordinal
    FindOrdinalInUniqueTypes<T, folly::tag_t<Args...>> =
        FindOrdinalInUniqueTypesImpl<T, Args...>::value;
#endif

template <class Id, class Idents, class TypeTags, class IdList>
FOLLY_CONSTEVAL std::enable_if_t<std::is_same<Id, void>::value, FieldOrdinal>
getFieldOrdinal(IdList&&) {
  return static_cast<FieldOrdinal>(0);
}

template <class Id, class Idents, class TypeTags, class IdList>
FOLLY_CONSTEVAL std::enable_if_t<type::is_field_id_v<Id>, FieldOrdinal>
getFieldOrdinal(IdList&& ids) {
  return findOrdinal(ids + 1, std::end(ids), folly::to_underlying(Id::value));
}

template <class Id, class Idents, class TypeTags, class IdList>
FOLLY_CONSTEVAL std::enable_if_t<type::is_ident_v<Id>, FieldOrdinal>
getFieldOrdinal(IdList&&) {
  return FindOrdinalInUniqueTypes<Id, Idents>;
}

template <class Id, class Idents, class TypeTags, class IdList>
FOLLY_CONSTEVAL std::enable_if_t<type::detail::is_type_tag_v<Id>, FieldOrdinal>
getFieldOrdinal(IdList&&) {
  static_assert(
      FindOrdinal<Id, TypeTags>::count() <= 1, "Type Tag is not unique");
  return FindOrdinal<Id, TypeTags>::value;
}

} // namespace detail
namespace ident {
template <class T>
void __fbthrift_check_whether_type_is_ident_via_adl(T&&);
}

} // namespace thrift
} // namespace apache

namespace apache::thrift::detail::annotation {

inline const std::vector<std::any>& empty_annotations() {
  static const folly::Indestructible<std::vector<std::any>> ret;
  return *ret;
}

template <class Struct>
const std::vector<std::any>& field_annotation_values(FieldId) {
  // @lint-ignore CLANGTIDY bugprone-sizeof-expression
  static_assert(sizeof(Struct) >= 0, "Struct must be a complete type");
  return empty_annotations();
}

template <class T>
inline constexpr bool is_runtime_annotation =
    decltype(detail::st::struct_private_access::
                 __fbthrift_cpp2_is_runtime_annotation<T>())::value;
} // namespace apache::thrift::detail::annotation

namespace apache::thrift {

template <class Annotation, class Struct, class Ident>
const Annotation* get_field_annotation() {
  using detail::annotation::field_annotation_values;
  using detail::annotation::is_runtime_annotation;
  static_assert(
      is_runtime_annotation<Annotation>,
      "Annotation is not annotated with @cpp.RuntimeAnnotation.");
  static_assert(
      op::get_ordinal<Struct, Ident>::value != static_cast<FieldOrdinal>(0),
      "Ident not found in Struct.");

  static const Annotation* ret = []() -> const Annotation* {
    for (const std::any& v : field_annotation_values<Struct>(
             op::get_field_id<Struct, Ident>::value)) {
      if (auto* p = std::any_cast<Annotation>(&v)) {
        return p;
      }
    }
    return nullptr;
  }();

  return ret;
}
} // namespace apache::thrift
