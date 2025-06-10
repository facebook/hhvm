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

namespace apache::thrift::protocol::experimental {

// ---- NativeList ---- //

template <typename T>
NativeList::NativeList(ListOf<T>&& l) : kind_(std::move(l)) {}
template <typename T>
NativeList::NativeList(const ListOf<T>& l) : kind_(l) {}
template <typename T>
bool NativeList::has_element() const noexcept {
  return std::holds_alternative<
      ::apache::thrift::protocol::experimental::detail::list_t<T>>(kind_);
}
template <typename T>
bool NativeList::is_type() const noexcept {
  static_assert(
      detail::is_list_v<T> && !std::is_same_v<T, NativeList>,
      "NativeList is always a list type");
  using ElemTy = std::remove_cv_t<typename T::value_type>;
  return std::holds_alternative<detail::list_t<ElemTy>>(kind_);
}
template <typename T>
const NativeList::Specialized<T>& NativeList::as_type() const {
  static_assert(
      detail::is_list_v<T> && !std::is_same_v<T, NativeList>,
      "NativeList is always a list type");
  return std::get<Specialized<T>>(kind_);
}
template <typename T>
NativeList::Specialized<T>& NativeList::as_type() {
  static_assert(
      detail::is_list_v<T> && !std::is_same_v<T, NativeList>,
      "NativeList is always a list type");
  return std::get<Specialized<T>>(kind_);
}
template <typename T>
const NativeList::Specialized<T>* NativeList::if_type() const noexcept {
  static_assert(
      detail::is_list_v<T> && !std::is_same_v<T, NativeList>,
      "NativeList is always a list type");
  return std::get_if<Specialized<T>>(&kind_);
}
template <typename T>
NativeList::Specialized<T>* NativeList::if_type() noexcept {
  static_assert(
      detail::is_list_v<T> && !std::is_same_v<T, NativeList>,
      "NativeList is always a list type");
  return std::get_if<Specialized<T>>(&kind_);
}

// ---- NativeSet ---- //

template <typename T>
NativeSet::NativeSet(SetOf<T>&& s) : kind_(std::move(s)) {}

template <typename T>
bool NativeSet::has_element() const noexcept {
  return std::holds_alternative<detail::set_t<T>>(kind_);
}

template <typename T>
bool NativeSet::is_type() const noexcept {
  static_assert(
      detail::is_set_v<T> && !std::is_same_v<T, NativeSet>,
      "NativeSet is always a set type");
  return std::holds_alternative<Specialized<T>>(kind_);
}

template <typename T>
const NativeSet::Specialized<T>& NativeSet::as_type() const {
  static_assert(
      detail::is_set_v<T> && !std::is_same_v<T, NativeSet>,
      "NativeSet is always a set type");
  return std::get<Specialized<T>>(kind_);
}

template <typename T>
NativeSet::Specialized<T>& NativeSet::as_type() {
  static_assert(
      detail::is_set_v<T> && !std::is_same_v<T, NativeSet>,
      "NativeSet is always a set type");
  return std::get<Specialized<T>>(kind_);
}

template <typename T>
const NativeSet::Specialized<T>* NativeSet::if_type() const noexcept {
  static_assert(
      detail::is_set_v<T> && !std::is_same_v<T, NativeSet>,
      "NativeSet is always a set type");
  return std::get_if<Specialized<T>>(&kind_);
}

template <typename T>
NativeSet::Specialized<T>* NativeSet::if_type() noexcept {
  static_assert(
      detail::is_set_v<T> && !std::is_same_v<T, NativeSet>,
      "NativeSet is always a set type");
  return std::get_if<Specialized<T>>(&kind_);
}

// ---- NativeMap ---- //

template <typename... Args>
NativeMap::NativeMap(MapOf<Args...>&& m) noexcept : kind_(std::move(m)) {}

template <typename T>
bool NativeMap::is_type() const noexcept {
  static_assert(
      detail::is_map_v<T> && !std::is_same_v<T, NativeMap>,
      "NativeMap is always a map type");
  return std::holds_alternative<Specialized<T>>(kind_);
}

template <typename T>
const NativeMap::Specialized<T>& NativeMap::as_type() const {
  static_assert(
      detail::is_map_v<T> && !std::is_same_v<T, NativeMap>,
      "NativeMap is always a map type");
  return std::get<Specialized<T>>(kind_);
}

template <typename T>
NativeMap::Specialized<T>& NativeMap::as_type() {
  static_assert(
      detail::is_map_v<T> && !std::is_same_v<T, NativeMap>,
      "NativeMap is always a map type");
  return std::get<Specialized<T>>(kind_);
}

template <typename T>
const NativeMap::Specialized<T>* NativeMap::if_type() const noexcept {
  static_assert(
      detail::is_map_v<T> && !std::is_same_v<T, NativeMap>,
      "NativeMap is always a map type");
  return std::get_if<Specialized<T>>(&kind_);
}

template <typename T>
NativeMap::Specialized<T>* NativeMap::if_type() noexcept {
  static_assert(
      detail::is_map_v<T> && !std::is_same_v<T, NativeMap>,
      "NativeMap is always a map type");
  return std::get_if<Specialized<T>>(&kind_);
}

template <typename K, typename V>
bool detail::native_map_emplace(NativeMap& map, K&& key, V&& val) {
  return map.visit(
      [&](std::monostate&) {
        // Create a new map with the specialized type
        static_assert(!std::is_same_v<std::remove_cvref_t<K>, NativeValue>);
        static_assert(!std::is_same_v<std::remove_cvref_t<V>, NativeValue>);
        map = make_map_of<K, V>(std::forward<K>(key), std::forward<V>(val));
        return true;
      },
      [&](auto&& m) -> bool {
        // If the map has a matching key/value, emplace it directly, otherwise
        // it's an error
        using MapTy = std::remove_cvref_t<decltype(m)>;
        using MapKey = typename MapTy::key_type;

        if constexpr (std::is_same_v<MapTy, NativeMap::Generic>) {
          return m.emplace(std::forward<K>(key), std::forward<V>(val)).second;
        } else if constexpr (std::is_same_v<std::remove_cvref_t<K>, MapKey>) {
          if constexpr (
              std::is_same_v<std::remove_cvref_t<V>, ValueHolder> ||
              std::is_same_v<std::remove_cvref_t<V>, NativeValue>) {
            return m.emplace(std::forward<K>(key), std::forward<V>(val)).second;
          } else {
            return m
                .emplace(
                    std::forward<K>(key), NativeValue{std::forward<V>(val)})
                .second;
          }
        } else {
          throw std::runtime_error(fmt::format(
              "Cannot emplace key={} value={} pair into a map with a different type: {}",
              folly::pretty_name<K>(),
              folly::pretty_name<V>(),
              folly::pretty_name<MapTy>()));
        }
      });
}

template <typename K, typename V>
bool detail::native_map_insert_or_assign(NativeMap& map, K&& key, V&& val) {
  return map.visit(
      [&](std::monostate&) {
        // Create a new map with the specialized type
        static_assert(!std::is_same_v<std::remove_cvref_t<K>, NativeValue>);
        static_assert(!std::is_same_v<std::remove_cvref_t<V>, NativeValue>);
        map = make_map_of<K, V>(std::forward<K>(key), std::forward<V>(val));
        return true;
      },
      [&](auto&& m) -> bool {
        // If the map has a matching key/value, insert/assign it directly,
        // otherwise it's an error
        using MapTy = std::remove_cvref_t<decltype(m)>;
        using MapKey = typename MapTy::key_type;

        if constexpr (std::is_same_v<MapTy, NativeMap::Generic>) {
          return m.emplace(std::forward<K>(key), std::forward<V>(val)).second;
        } else if constexpr (std::is_same_v<std::remove_cvref_t<K>, MapKey>) {
          if constexpr (
              std::is_same_v<std::remove_cvref_t<V>, ValueHolder> ||
              std::is_same_v<std::remove_cvref_t<V>, NativeValue>) {
            return m
                .insert_or_assign(std::forward<K>(key), std::forward<V>(val))
                .second;
          } else {
            return m
                .insert_or_assign(
                    std::forward<K>(key), NativeValue{std::forward<V>(val)})
                .second;
          }
        } else {
          throw std::runtime_error(fmt::format(
              "Cannot insert_or_assign key={} value={} pair into a map with a different type: {}",
              folly::pretty_name<K>(),
              folly::pretty_name<V>(),
              folly::pretty_name<MapTy>()));
        }
      });
}

// ---- Object ---- //

template <typename... Args>
NativeValue& NativeObject::emplace(FieldId id, Args... args) {
  auto it = fields.emplace(id, std::forward<Args...>(args)...).first;
  return it->second;
}

// ---- ValueAPI ---- //

template <typename T>
ValueType ValueAccess<T>::get_type() const {
  return folly::variant_match(
      value().inner(),
      [](const std::monostate&) { return ValueType::Empty; },
      [](const auto& val) {
        using V = std::remove_cvref_t<decltype(val)>;
        return detail::native_value_type_mapping_v<V>;
      });
}

template <typename T>
ValueAccess<T>::operator NativeValue&() noexcept {
  return value();
}

template <typename T>
ValueAccess<T>::operator const NativeValue&() const noexcept {
  return value();
}

template <typename T>
template <typename Ty>
bool ValueAccess<T>::is_type() const noexcept {
  return std::holds_alternative<detail::native_value_type_t<Ty>>(
      value().inner());
}

template <typename T>
template <typename Ty>
const detail::native_value_type_t<Ty>& ValueAccess<T>::as_type() const {
  return std::get<detail::native_value_type_t<Ty>>(value().inner());
}

template <typename T>
template <typename Ty>
detail::native_value_type_t<Ty>& ValueAccess<T>::as_type() {
  return std::get<detail::native_value_type_t<Ty>>(value().inner());
}

template <typename T>
template <typename Ty>
const detail::native_value_type_t<Ty>* ValueAccess<T>::if_type()
    const noexcept {
  return std::get_if<detail::native_value_type_t<Ty>>(&value().inner());
}

template <typename T>
template <typename Ty>
detail::native_value_type_t<Ty>* ValueAccess<T>::if_type() noexcept {
  return std::get_if<detail::native_value_type_t<Ty>>(&value().inner());
}

template <typename T>
bool ValueAccess<T>::operator==(const NativeValue& other) const {
  return value().inner() == other.inner();
}

template <typename T>
bool ValueAccess<T>::operator!=(const NativeValue& other) const {
  return value().inner() != other.inner();
}

#define FB_THRIFT_VALUE_ACCESS_IMPL(TYPE, NAME)           \
  template <typename T>                                   \
  bool ValueAccess<T>::is_##NAME() const {                \
    return std::holds_alternative<TYPE>(value().inner()); \
  }                                                       \
  template <typename T>                                   \
  const TYPE& ValueAccess<T>::as_##NAME() const {         \
    return std::get<TYPE>(value().inner());               \
  }                                                       \
  template <typename T>                                   \
  TYPE& ValueAccess<T>::as_##NAME() {                     \
    return std::get<TYPE>(value().inner());               \
  }                                                       \
  template <typename T>                                   \
  const TYPE* ValueAccess<T>::if_##NAME() const {         \
    return std::get_if<TYPE>(&value().inner());           \
  }                                                       \
  template <typename T>                                   \
  TYPE* ValueAccess<T>::if_##NAME() {                     \
    return std::get_if<TYPE>(&value().inner());           \
  }                                                       \
  template <typename T>                                   \
  TYPE& ValueAccess<T>::ensure_##NAME() {                 \
    if (!std::holds_alternative<TYPE>(value().inner())) { \
      return value().inner().template emplace<TYPE>();    \
    }                                                     \
    return std::get<TYPE>(value().inner());               \
  }                                                       \
  template <typename T>                                   \
  template <typename... Args>                             \
  TYPE& ValueAccess<T>::emplace_##NAME(Args&&... args) {  \
    return value().inner().template emplace<TYPE>(        \
        std::forward<Args>(args)...);                     \
  }

FB_THRIFT_VALUE_ACCESS_IMPL(PrimitiveTypes::Bool, bool)
FB_THRIFT_VALUE_ACCESS_IMPL(PrimitiveTypes::I8, byte)
FB_THRIFT_VALUE_ACCESS_IMPL(PrimitiveTypes::I16, i16)
FB_THRIFT_VALUE_ACCESS_IMPL(PrimitiveTypes::I32, i32)
FB_THRIFT_VALUE_ACCESS_IMPL(PrimitiveTypes::I64, i64)
FB_THRIFT_VALUE_ACCESS_IMPL(PrimitiveTypes::Float, float)
FB_THRIFT_VALUE_ACCESS_IMPL(PrimitiveTypes::Double, double)
FB_THRIFT_VALUE_ACCESS_IMPL(PrimitiveTypes::Bytes, binary)
FB_THRIFT_VALUE_ACCESS_IMPL(NativeList, list)
FB_THRIFT_VALUE_ACCESS_IMPL(NativeSet, set)
FB_THRIFT_VALUE_ACCESS_IMPL(NativeMap, map)
FB_THRIFT_VALUE_ACCESS_IMPL(NativeObject, object)

// ---- ValueHelper ---- //

template <typename TT>
struct ValueHelper<TT> {
  template <typename T>
  static NativeValue into(T&& value) {
    if constexpr (std::is_same_v<TT, type::bool_t>) {
      return NativeValue(static_cast<PrimitiveTypes::Bool>(value));
    } else if constexpr (std::is_same_v<TT, type::byte_t>) {
      return NativeValue(static_cast<PrimitiveTypes::I8>(value));
    } else if constexpr (std::is_same_v<TT, type::i16_t>) {
      return NativeValue(static_cast<PrimitiveTypes::I16>(value));
    } else if constexpr (
        std::is_same_v<TT, type::i32_t> ||
        type::base_type_v<TT> == type::BaseType::Enum) {
      return NativeValue(static_cast<PrimitiveTypes::I32>(value));
    } else if constexpr (std::is_same_v<TT, type::i64_t>) {
      return NativeValue(static_cast<PrimitiveTypes::I64>(value));
    } else if constexpr (std::is_same_v<TT, type::float_t>) {
      return NativeValue(static_cast<PrimitiveTypes::Float>(value));
    } else if constexpr (std::is_same_v<TT, type::double_t>) {
      return NativeValue(static_cast<PrimitiveTypes::Double>(value));
    } else if constexpr (
        std::is_same_v<TT, type::string_t> ||
        std::is_same_v<TT, type::binary_t>) {
      return NativeValue(PrimitiveTypes::Bytes{std::forward<T>(value)});
    } else {
      static_assert(folly::always_false<T>, "Unknown Type Tag.");
    }
  }
};

template <>
struct ValueHelper<type::binary_t> {
  static NativeValue into(folly::IOBuf value) {
    return NativeValue{PrimitiveTypes::Bytes::fromIOBuf(std::move(value))};
  }
  static NativeValue into(std::string_view value) {
    return NativeValue{PrimitiveTypes::Bytes::fromIOBuf(
        folly::IOBuf{folly::IOBuf::COPY_BUFFER, value.data(), value.size()})};
  }
  static NativeValue into(folly::ByteRange value) {
    return NativeValue{PrimitiveTypes::Bytes::fromIOBuf(
        folly::IOBuf{folly::IOBuf::COPY_BUFFER, value.data(), value.size()})};
  }
};

template <typename V>
struct ValueHelper<type::list<V>> {
  template <typename T>
  static NativeValue into(T&& value) {
    using InputListElem = typename std::remove_cvref_t<T>::value_type;
    using ResultList = typename detail::list_t<InputListElem>;
    ResultList native_list{};
    native_list.reserve(value.size());
    for (auto&& elem : value) {
      if constexpr (::apache::thrift::type::
                        is_a_v<V, ::apache::thrift::type::primitive_c>) {
        native_list.emplace_back(elem);
      } else if constexpr (std::is_same_v<
                               ValueHolder,
                               typename ResultList::value_type>) {
        native_list.emplace_back(ValueHelper<V>::into(elem));
      } else if constexpr (std::is_same_v<
                               NativeObject,
                               typename ResultList::value_type>) {
        static_assert(
            false,
            "::asValueStruct does not support NativeObject conversion (yet)");
      } else {
        static_assert(
            false, "Missing NativeList specialization for ValueHelper");
      }
    }
    return NativeList{std::move(native_list)};
  }
};

template <typename V>
struct ValueHelper<type::set<V>> {
  template <typename T>
  static NativeValue into(T&& value) {
    using InputSetElem = typename std::remove_cvref_t<T>::value_type;
    using ResultSet = typename detail::set_t<InputSetElem>;
    ResultSet native_set{};
    native_set.reserve(value.size());
    for (auto&& elem : value) {
      if constexpr (::apache::thrift::type::
                        is_a_v<V, ::apache::thrift::type::primitive_c>) {
        native_set.emplace(elem);
      } else if constexpr (std::is_same_v<
                               ValueHolder,
                               typename ResultSet::value_type>) {
        native_set.emplace(ValueHelper<V>::into(elem));
      } else if constexpr (std::is_same_v<
                               NativeObject,
                               typename ResultSet::value_type>) {
        static_assert(
            false,
            "::asValueStruct does not support NativeObject conversion (yet)");
      } else {
        static_assert(
            false, "Missing NativeSet specialization for ValueHelper");
      }
    }
    return NativeSet{std::move(native_set)};
  }
};

template <typename T, typename Tag>
struct ValueHelper<type::cpp_type<T, Tag>> : ValueHelper<Tag> {};

} // namespace apache::thrift::protocol::experimental
