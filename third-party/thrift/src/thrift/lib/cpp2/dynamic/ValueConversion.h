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

#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/dynamic/Any.h>
#include <thrift/lib/cpp2/dynamic/Binary.h>
#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/List.h>
#include <thrift/lib/cpp2/dynamic/Map.h>
#include <thrift/lib/cpp2/dynamic/Set.h>
#include <thrift/lib/cpp2/dynamic/Struct.h>
#include <thrift/lib/cpp2/dynamic/Union.h>
#include <thrift/lib/cpp2/op/Get.h>

#include <folly/container/Reserve.h>
#include <folly/io/IOBuf.h>

#include <concepts>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace apache::thrift::dynamic {
namespace detail {

inline type_system::TypeRef underlyingType(type_system::TypeRef type) {
  while (type.isOpaqueAlias()) {
    type = type.asOpaqueAlias().targetType();
  }
  return type;
}

[[noreturn]] inline void throwIncompatible() {
  throw std::invalid_argument("Dynamic value cannot be converted to target");
}

using Kind = type_system::TypeRef::Kind;

template <typename Tag>
struct KindForTag;

template <>
struct KindForTag<type::bool_t> : std::integral_constant<Kind, Kind::BOOL> {};
template <>
struct KindForTag<type::byte_t> : std::integral_constant<Kind, Kind::BYTE> {};
template <>
struct KindForTag<type::i16_t> : std::integral_constant<Kind, Kind::I16> {};
template <>
struct KindForTag<type::i32_t> : std::integral_constant<Kind, Kind::I32> {};
template <>
struct KindForTag<type::i64_t> : std::integral_constant<Kind, Kind::I64> {};
template <>
struct KindForTag<type::float_t> : std::integral_constant<Kind, Kind::FLOAT> {};
template <>
struct KindForTag<type::double_t> : std::integral_constant<Kind, Kind::DOUBLE> {
};
template <typename T>
struct KindForTag<type::enum_t<T>> : std::integral_constant<Kind, Kind::ENUM> {
};

template <typename Tag, typename T>
DynamicValue toScalar(const T& value, type_system::TypeRef expectedType) {
  constexpr auto expectedKind = KindForTag<Tag>::value;
  return expectedType.matchKind(
      [&]<Kind actualKind>(
          type_system::TypeRef::KindConstant<actualKind>) -> DynamicValue {
        if constexpr (actualKind != expectedKind) {
          throwIncompatible();
        } else {
          auto result = DynamicValue::makeDefault(expectedType);
          result.as<actualKind>() =
              static_cast<type_of_type_kind<actualKind>>(value);
          return result;
        }
      });
}

template <typename Tag, typename T>
void fromScalar(T& result, const DynamicConstRef& value) {
  constexpr auto expectedKind = KindForTag<Tag>::value;
  value.type().matchKind(
      [&]<Kind actualKind>(type_system::TypeRef::KindConstant<actualKind>) {
        if constexpr (actualKind != expectedKind) {
          throwIncompatible();
        } else {
          result = static_cast<T>(value.as<actualKind>());
        }
      });
}

template <typename Tag>
struct ToDynamic;
template <typename Tag>
struct FromDynamic;

template <typename Tag>
  requires type::is_a_v<Tag, type::number_c>
struct ToDynamic<Tag> {
  template <typename T>
  DynamicValue operator()(
      const T& value, type_system::TypeRef expectedType) const {
    return toScalar<Tag>(value, expectedType);
  }
};

template <typename Tag>
  requires type::is_a_v<Tag, type::number_c>
struct FromDynamic<Tag> {
  template <typename T>
  void operator()(T& result, const DynamicConstRef& value) const {
    fromScalar<Tag>(result, value);
  }
};

template <>
struct ToDynamic<type::bool_t> {
  DynamicValue operator()(bool value, type_system::TypeRef expectedType) const {
    return toScalar<type::bool_t>(value, expectedType);
  }
};

template <>
struct FromDynamic<type::bool_t> {
  void operator()(bool& result, const DynamicConstRef& value) const {
    fromScalar<type::bool_t>(result, value);
  }
};

template <>
struct ToDynamic<type::string_t> {
  template <typename T>
  DynamicValue operator()(
      const T& value, type_system::TypeRef expectedType) const {
    const std::string_view view(value);
    const auto underlying = underlyingType(expectedType);
    auto result = DynamicValue::makeDefault(expectedType);
    if (underlying.isString()) {
      result.asString() = view;
    } else if (underlying.isBinary()) {
      result.asBinary() = Binary(folly::IOBuf::copyBuffer(view));
    } else {
      throwIncompatible();
    }
    return result;
  }
};

inline std::string copyBinaryToString(const Binary& value) {
  std::string result(value.computeChainDataLength(), '\0');
  if (!result.empty()) {
    auto cursor = value.cursor();
    cursor.pull(result.data(), result.size());
  }
  return result;
}

template <>
struct FromDynamic<type::string_t> {
  template <typename T>
  void operator()(T& result, const DynamicConstRef& value) const {
    const auto type = underlyingType(value.type());
    if (type.isString()) {
      result = value.asString().view();
    } else if constexpr (std::same_as<T, std::string>) {
      if (!type.isBinary()) {
        throwIncompatible();
      }
      result = copyBinaryToString(value.asBinary());
    } else {
      throwIncompatible();
    }
  }
};

template <>
struct ToDynamic<type::binary_t> {
  template <typename T>
  DynamicValue operator()(
      const T& value, type_system::TypeRef expectedType) const {
    if (!underlyingType(expectedType).isBinary()) {
      throwIncompatible();
    }
    auto result = DynamicValue::makeDefault(expectedType);
    if constexpr (std::convertible_to<T, std::string_view>) {
      result.asBinary() =
          Binary(folly::IOBuf::copyBuffer(std::string_view(value)));
    } else if constexpr (std::same_as<std::remove_cvref_t<T>, folly::IOBuf>) {
      result.asBinary() = Binary(value.clone());
    } else {
      result.asBinary() =
          Binary(value ? value->clone() : folly::IOBuf::create(0));
    }
    return result;
  }
};

template <>
struct FromDynamic<type::binary_t> {
  template <typename T>
  void operator()(T& result, const DynamicConstRef& value) const {
    if (!underlyingType(value.type()).isBinary()) {
      throwIncompatible();
    }
    if constexpr (std::same_as<T, std::string>) {
      result = copyBinaryToString(value.asBinary());
    } else if constexpr (std::same_as<T, folly::IOBuf>) {
      if (value.asBinary().empty()) {
        result = folly::IOBuf{};
      } else {
        auto cursor = value.asBinary().cursor();
        cursor.clone(result, value.asBinary().computeChainDataLength());
      }
    } else {
      auto data = folly::IOBuf::create(0);
      if (!value.asBinary().empty()) {
        auto cursor = value.asBinary().cursor();
        cursor.clone(*data, value.asBinary().computeChainDataLength());
      }
      result = std::move(data);
    }
  }
};

template <typename T>
struct ToDynamic<type::enum_t<T>> {
  DynamicValue operator()(T value, type_system::TypeRef expectedType) const {
    return toScalar<type::enum_t<T>>(value, expectedType);
  }
};

template <typename T>
struct FromDynamic<type::enum_t<T>> {
  void operator()(T& result, const DynamicConstRef& value) const {
    fromScalar<type::enum_t<T>>(result, value);
  }
};

template <typename Tag>
struct ToDynamic<type::list<Tag>> {
  template <typename T>
  DynamicValue operator()(
      const T& value, type_system::TypeRef expectedType) const {
    const auto underlying = underlyingType(expectedType);
    if (!underlying.isList()) {
      throwIncompatible();
    }
    auto result = DynamicValue::makeDefault(expectedType);
    auto& list = result.asList();
    list.reserve(value.size());
    for (const auto& element : value) {
      list.push_back(
          ToDynamic<Tag>{}(element, underlying.asList().elementType()));
    }
    return result;
  }
};

template <typename Tag>
struct FromDynamic<type::list<Tag>> {
  template <typename T>
  void operator()(T& result, const DynamicConstRef& value) const {
    if (!underlyingType(value.type()).isList()) {
      throwIncompatible();
    }
    result.clear();
    folly::reserve_if_available(result, value.asList().size());
    for (const auto element : value.asList()) {
      type::native_type<Tag> converted;
      FromDynamic<Tag>{}(converted, element);
      result.push_back(std::move(converted));
    }
  }
};

template <typename Tag>
struct ToDynamic<type::set<Tag>> {
  template <typename T>
  DynamicValue operator()(
      const T& value, type_system::TypeRef expectedType) const {
    const auto underlying = underlyingType(expectedType);
    if (!underlying.isSet()) {
      throwIncompatible();
    }
    auto result = DynamicValue::makeDefault(expectedType);
    auto& set = result.asSet();
    set.reserve(value.size());
    for (const auto& element : value) {
      set.insert(ToDynamic<Tag>{}(element, underlying.asSet().elementType()));
    }
    return result;
  }
};

template <typename Tag>
struct FromDynamic<type::set<Tag>> {
  template <typename T>
  void operator()(T& result, const DynamicConstRef& value) const {
    if (!underlyingType(value.type()).isSet()) {
      throwIncompatible();
    }
    result.clear();
    folly::reserve_if_available(result, value.asSet().size());
    for (const auto element : value.asSet()) {
      type::native_type<Tag> converted;
      FromDynamic<Tag>{}(converted, element);
      result.insert(std::move(converted));
    }
  }
};

template <typename KeyTag, typename ValueTag>
struct ToDynamic<type::map<KeyTag, ValueTag>> {
  template <typename T>
  DynamicValue operator()(
      const T& value, type_system::TypeRef expectedType) const {
    const auto underlying = underlyingType(expectedType);
    if (!underlying.isMap()) {
      throwIncompatible();
    }
    auto result = DynamicValue::makeDefault(expectedType);
    auto& map = result.asMap();
    map.reserve(value.size());
    for (const auto& [key, mapped] : value) {
      map.insert(
          ToDynamic<KeyTag>{}(key, underlying.asMap().keyType()),
          ToDynamic<ValueTag>{}(mapped, underlying.asMap().valueType()));
    }
    return result;
  }
};

template <typename KeyTag, typename ValueTag>
struct FromDynamic<type::map<KeyTag, ValueTag>> {
  template <typename T>
  void operator()(T& result, const DynamicConstRef& value) const {
    if (!underlyingType(value.type()).isMap()) {
      throwIncompatible();
    }
    result.clear();
    folly::reserve_if_available(result, value.asMap().size());
    for (const auto [key, mapped] : value.asMap()) {
      type::native_type<KeyTag> convertedKey;
      FromDynamic<KeyTag>{}(convertedKey, key);
      type::native_type<ValueTag> convertedValue;
      FromDynamic<ValueTag>{}(convertedValue, mapped);
      result.emplace(std::move(convertedKey), std::move(convertedValue));
    }
  }
};

template <typename T>
struct ToDynamicStructure {
  DynamicValue operator()(
      const T& value, type_system::TypeRef expectedType) const {
    const auto underlying = underlyingType(expectedType);
    if (!underlying.isStruct()) {
      throwIncompatible();
    }
    auto result = DynamicValue::makeDefault(expectedType);
    auto& structValue = result.asStruct();
    op::for_each_field_id<T>([&](auto id) {
      using Id = decltype(id);
      const auto handle =
          underlying.asStruct().fieldHandleFor(FieldId{Id::value});
      if (!handle.valid()) {
        return;
      }
      if (const auto* field = op::get_value_or_null(op::get<Id>(value))) {
        using FieldTag = op::get_type_tag<T, Id>;
        structValue.setField(
            handle,
            ToDynamic<FieldTag>{}(
                *field, underlying.asStruct().at(handle).type()));
      }
    });
    return result;
  }
};

template <typename T>
struct FromDynamicStructure {
  void operator()(T& result, const DynamicConstRef& value) const {
    const auto underlying = underlyingType(value.type());
    if (!underlying.isStruct()) {
      throwIncompatible();
    }
    for (const auto& field : underlying.asStruct().fields()) {
      const auto id = field.identity().id();
      const auto fieldValue = value.asStruct().getField(id);
      if (!fieldValue) {
        continue;
      }
      op::invoke_by_field_id<T>(
          id,
          [&](auto staticId) {
            using Id = decltype(staticId);
            using FieldTag = op::get_field_tag<T, Id>;
            FromDynamic<FieldTag>{}(result, *fieldValue);
          },
          [] {});
    }
  }
};

template <typename T>
struct ToDynamic<type::struct_t<T>> : ToDynamicStructure<T> {};
template <typename T>
struct FromDynamic<type::struct_t<T>> : FromDynamicStructure<T> {};
template <typename T>
struct ToDynamic<type::exception_t<T>> : ToDynamicStructure<T> {};
template <typename T>
struct FromDynamic<type::exception_t<T>> : FromDynamicStructure<T> {};

template <typename T>
struct ToDynamic<type::union_t<T>> {
  DynamicValue operator()(
      const T& value, type_system::TypeRef expectedType) const {
    const auto underlying = underlyingType(expectedType);
    if (!underlying.isUnion()) {
      throwIncompatible();
    }
    auto result = DynamicValue::makeDefault(expectedType);
    op::invoke_by_field_id<T>(
        static_cast<FieldId>(value.getType()),
        [&](auto id) {
          using Id = decltype(id);
          const auto handle =
              underlying.asUnion().fieldHandleFor(FieldId{Id::value});
          if (!handle.valid()) {
            return;
          }
          using FieldTag = op::get_type_tag<T, Id>;
          result.asUnion().setField(
              handle,
              ToDynamic<FieldTag>{}(
                  *op::get<Id>(value), underlying.asUnion().at(handle).type()));
        },
        [] {});
    return result;
  }
};

template <typename T>
struct FromDynamic<type::union_t<T>> {
  void operator()(T& result, const DynamicConstRef& value) const {
    const auto underlying = underlyingType(value.type());
    if (!underlying.isUnion()) {
      throwIncompatible();
    }
    const auto& unionValue = value.asUnion();
    if (unionValue.isEmpty()) {
      return;
    }
    const auto handle = unionValue.activeField();
    const auto id = underlying.asUnion().at(handle).identity().id();
    op::invoke_by_field_id<T>(
        id,
        [&](auto staticId) {
          using Id = decltype(staticId);
          using FieldTag = op::get_type_tag<T, Id>;
          using FieldType = op::get_native_type<T, Id>;
          FieldType converted;
          FromDynamic<FieldTag>{}(converted, unionValue.getField(handle));
          op::get<Id>(result) = std::move(converted);
        },
        [] {});
  }
};

template <typename T, typename Tag>
struct ToDynamic<type::cpp_type<T, Tag>> : ToDynamic<Tag> {};
template <typename T, typename Tag>
struct FromDynamic<type::cpp_type<T, Tag>> : FromDynamic<Tag> {};

template <typename Adapter, typename Tag>
struct ToDynamic<type::adapted<Adapter, Tag>> {
  template <typename T>
  DynamicValue operator()(
      const T& value, type_system::TypeRef expectedType) const {
    return ToDynamic<Tag>{}(Adapter::toThrift(value), expectedType);
  }
};

template <typename Adapter, typename Tag>
struct FromDynamic<type::adapted<Adapter, Tag>> {
  template <typename T>
  void operator()(T& result, const DynamicConstRef& value) const {
    type::native_type<Tag> original;
    FromDynamic<Tag>{}(original, value);
    result = Adapter::fromThrift(std::move(original));
  }
};

template <typename Tag, typename Struct, int16_t FieldId>
struct FromDynamic<type::field<Tag, FieldContext<Struct, FieldId>>> {
  void operator()(Struct& result, const DynamicConstRef& value) const {
    using Id = field_id<FieldId>;
    using FieldType = op::get_native_type<Struct, Id>;
    FieldType converted;
    FromDynamic<Tag>{}(converted, value);
    using Ref = op::get_field_ref<Struct, Id>;
    if constexpr (apache::thrift::detail::is_shared_or_unique_ptr_v<Ref>) {
      op::get<Id>(result) = std::make_unique<FieldType>(std::move(converted));
    } else {
      op::get<Id>(result) = std::move(converted);
    }
  }
};

template <typename Adapter, typename Tag, typename Struct, int16_t FieldId>
struct FromDynamic<
    type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>> {
  void operator()(Struct& result, const DynamicConstRef& value) const {
    using Id = field_id<FieldId>;
    using FieldType = op::get_native_type<Struct, Id>;
    std::remove_cvref_t<adapt_detail::thrift_t<Adapter, FieldType>> original;
    FromDynamic<Tag>{}(original, value);
    op::get<Id>(result) = adapt_detail::fromThriftField<Adapter, FieldId>(
        std::move(original), result);
  }
};

} // namespace detail

/** Converts a concrete Thrift value to the requested runtime type. */
template <typename T>
DynamicValue toDynamicValue(const T& value, type_system::TypeRef expectedType) {
  using Value = std::remove_cvref_t<T>;
  if constexpr (std::same_as<Value, DynamicValue>) {
    if (!value.type().isEqualIdentityTo(expectedType)) {
      throw std::invalid_argument("DynamicValue type does not match target");
    }
    return value;
  } else if constexpr (std::same_as<Value, type::AnyStruct>) {
    if (!detail::underlyingType(expectedType).isAny()) {
      detail::throwIncompatible();
    }
    auto result = DynamicValue::makeDefault(expectedType);
    result.asAny() = Any(type::AnyData(value));
    return result;
  } else if constexpr (std::same_as<Value, type::AnyData>) {
    if (!detail::underlyingType(expectedType).isAny()) {
      detail::throwIncompatible();
    }
    auto result = DynamicValue::makeDefault(expectedType);
    result.asAny() = Any(value);
    return result;
  } else if constexpr (std::convertible_to<T, std::string_view>) {
    return detail::ToDynamic<type::string_t>{}(
        std::string_view(value), expectedType);
  } else {
    return detail::ToDynamic<type::infer_tag<Value>>{}(value, expectedType);
  }
}

/** Converts a dynamic value to the requested concrete Thrift type. */
template <typename T>
T fromDynamicValue(const DynamicConstRef& value) {
  if constexpr (std::same_as<T, type::AnyStruct>) {
    if (!detail::underlyingType(value.type()).isAny()) {
      detail::throwIncompatible();
    }
    return value.asAny().toThrift();
  } else if constexpr (std::same_as<T, type::AnyData>) {
    if (!detail::underlyingType(value.type()).isAny()) {
      detail::throwIncompatible();
    }
    return type::AnyData(value.asAny().toThrift());
  } else if constexpr (std::same_as<T, std::string>) {
    const auto type = detail::underlyingType(value.type());
    if (type.isBinary()) {
      return detail::copyBinaryToString(value.asBinary());
    }
    std::string result;
    detail::FromDynamic<type::string_t>{}(result, value);
    return result;
  } else {
    T result;
    detail::FromDynamic<type::infer_tag<T>>{}(result, value);
    return result;
  }
}

} // namespace apache::thrift::dynamic
