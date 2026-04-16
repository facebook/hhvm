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

#include <memory>
#include <string_view>
#include <utility>

#include <folly/Memory.h>
#include <folly/Portability.h>
#include <folly/Traits.h>
#include <folly/container/F14Map.h>
#include <folly/container/Reserve.h>
#include <folly/detail/StaticSingletonManager.h>

#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/detail/Enum.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>

namespace apache::thrift::detail {

namespace st {

//  copy_field_fn
//  copy_field
//
//  Returns a copy of a field. Used by structure copy-cosntructors.
//
//  Transitively copies through unique-ptr's, which are not copy-constructible.
template <typename TypeClass>
struct copy_field_fn;
template <typename TypeClass>
inline constexpr copy_field_fn<TypeClass> copy_field{};

template <typename>
struct copy_field_rec {
  template <typename T>
  T operator()(T const& t) const {
    return t;
  }
};

template <typename ValueTypeClass>
struct copy_field_rec<type_class::list<ValueTypeClass>> {
  template <typename T>
  T operator()(T const& t) const {
    T result;
    folly::reserve_if_available(result, t.size());
    for (const auto& e : t) {
      result.push_back(copy_field<ValueTypeClass>(e));
    }
    return result;
  }
};

template <typename ValueTypeClass>
struct copy_field_rec<type_class::set<ValueTypeClass>> {
  template <typename T>
  T operator()(T const& t) const {
    T result;
    folly::reserve_if_available(result, t.size());
    for (const auto& e : t) {
      result.emplace_hint(result.end(), copy_field<ValueTypeClass>(e));
    }
    return result;
  }
};

template <typename KeyTypeClass, typename MappedTypeClass>
struct copy_field_rec<type_class::map<KeyTypeClass, MappedTypeClass>> {
  template <typename T>
  T operator()(T const& t) const {
    T result;
    folly::reserve_if_available(result, t.size());
    for (const auto& pair : t) {
      result.emplace_hint(
          result.end(),
          copy_field<KeyTypeClass>(pair.first),
          copy_field<MappedTypeClass>(pair.second));
    }
    return result;
  }
};

template <typename TypeClass>
struct copy_field_fn : copy_field_rec<TypeClass> {
  using rec = copy_field_rec<TypeClass>;

  using rec::operator();
  template <typename T>
  std::unique_ptr<T> operator()(const std::unique_ptr<T>& t) const {
    return !t ? nullptr : std::make_unique<T>((*this)(*t));
  }

  template <typename T, typename Alloc>
  std::unique_ptr<T, folly::allocator_delete<Alloc>> operator()(
      const std::unique_ptr<T, folly::allocator_delete<Alloc>>& t) const {
    return !t ? nullptr
              : folly::allocate_unique<T>(
                    t.get_deleter().get_allocator(), (*this)(*t));
  }
};

struct translate_field_name_table {
  size_t size;
  const std::string_view* names;
  const int16_t* ids;
  const protocol::TType* types;
};

struct translate_field_name_hash_table {
  translate_field_name_hash_table(
      size_t size,
      const std::string_view* names,
      const int16_t* ids,
      const protocol::TType* types);

  folly::F14FastMap<std::string_view, std::pair<int16_t, protocol::TType>> map;
  folly::F14FastMap<int16_t, protocol::TType> idMap;
};

void checkFieldIdConflict(int16_t expected, int16_t actual);

void translate_field_name_or_id(
    std::string_view fname,
    int16_t& fid,
    protocol::TType& ftype,
    const translate_field_name_table& table);

void translate_field_name_or_id(
    std::string_view fname,
    int16_t& fid,
    protocol::TType& ftype,
    const translate_field_name_hash_table& table);

namespace {

//  gen_check_get_json
//
//  Metafunctions for getting the member types named, respectively,
//    * __fbthrift_cpp2_gen_json
struct gen_check_get_json {
  template <typename Type>
  using apply = decltype(private_access::__fbthrift_cpp2_gen_json<Type>());
};

//  gen_check_get
//
//  Metafunction for applying Get over Type and for handling the case where
//  Get fails to apply.
//
//  Get is one of the getters above:
//    * gen_check_get_json
//
//  When Get::apply<Type>:
//    * fails to apply (because cpp.type is in use), treat as true
//    * returns signed (extern template instances are generated), treat as true
//    * returns unsigned (otherwise), treat as false
//
//  The tag types signed and unsigned are used in the generated code to minimize
//  the overhead of parsing the class body, shifting all overhead to the code
//  which inspects these tags.
template <typename Void, typename Get, typename Type>
constexpr bool gen_check_get_ = true;
template <typename Get, typename Type>
constexpr bool gen_check_get_<
    folly::void_t<typename Get::template apply<Type>>,
    Get,
    Type> = Get::template apply<Type>::value;
template <typename Get, typename Type>
constexpr bool gen_check_get = gen_check_get_<void, Get, Type>;

//  gen_check_rec
//
//  Metafunction for recursing through container types to apply the metafunction
//  gen_check_get over struct/union types.
//
//  Get is one of the getters above:
//    * gen_check_get_json
template <typename TypeClass>
struct gen_check_rec {
  template <typename Get, typename Type>
  static constexpr bool apply = true;
};
template <typename ValueTypeClass>
struct gen_check_rec_list_set {
  using ValueTraits = gen_check_rec<ValueTypeClass>;
  template <typename Get, typename Type>
  static constexpr bool apply =
      ValueTraits::template apply<Get, typename Type::value_type>;
};
template <typename ValueTypeClass>
struct gen_check_rec<type_class::list<ValueTypeClass>>
    : gen_check_rec_list_set<ValueTypeClass> {};
template <typename ValueTypeClass>
struct gen_check_rec<type_class::set<ValueTypeClass>>
    : gen_check_rec_list_set<ValueTypeClass> {};
template <typename KeyTypeClass, typename MappedTypeClass>
struct gen_check_rec<type_class::map<KeyTypeClass, MappedTypeClass>> {
  using KeyTraits = gen_check_rec<KeyTypeClass>;
  using MappedTraits = gen_check_rec<MappedTypeClass>;
  template <typename Get, typename Type>
  static constexpr bool apply =
      KeyTraits::template apply<Get, typename Type::key_type> &&
      MappedTraits::template apply<Get, typename Type::mapped_type>;
};
struct gen_check_rec_structure_variant {
  template <typename Get, typename Type>
  static constexpr bool apply = gen_check_get<Get, Type>;
};
template <>
struct gen_check_rec<type_class::structure> : gen_check_rec_structure_variant {
};
template <>
struct gen_check_rec<type_class::variant> : gen_check_rec_structure_variant {};

//  gen_check
//
//  Returns whether, if the property Get holds for the outer structure Type,
//  that it also holds for each structure-typed field FieldType of the outer
//  type, peering through containers.
//
//  Get is one of the getters above:
//    * gen_check_get_json
template <
    typename Get,
    typename Type,
    typename FieldTypeClass,
    typename FieldType>
constexpr bool gen_check = !gen_check_get<Get, Type> ||
    gen_check_rec<FieldTypeClass>::template apply<Get, FieldType>;

//  gen_check_json
//
//  Aliases to gen_check partially instantiated with one of the getters above:
//    * gen_check_get_json
//
//  Used by a generated static_assert to enforce consistency over transitive
//  dependencies in the use of extern-template instantiations over json.
template <typename Type, typename FieldTypeClass, typename FieldType>
constexpr bool gen_check_json =
    gen_check<gen_check_get_json, Type, FieldTypeClass, FieldType>;

} // namespace

} // namespace st

template <class T>
bool pointer_equal(const T& lhs, const T& rhs) {
  return lhs && rhs ? *lhs == *rhs : lhs == rhs;
}

template <class T>
bool pointer_less(const T& lhs, const T& rhs) {
  return lhs && rhs ? *lhs < *rhs : lhs < rhs;
}

} // namespace apache::thrift::detail

// __fbthrift_static_init_* are referenced using extern prototypes to keep them
// private. This triggers the following warning, which is suppressed.
// Helpfully, clang triggers this warning for C++ but gcc not only does not but
// also errors if you try to suppress it, so we only suppress for clang.
FOLLY_CLANG_DISABLE_WARNING("-Wmissing-prototypes")
