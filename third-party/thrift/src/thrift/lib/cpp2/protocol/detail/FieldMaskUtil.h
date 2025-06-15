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
#include <unordered_set>

#include <folly/Range.h>
#include <folly/lang/Exception.h>

#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Copy.h>
#include <thrift/lib/cpp2/op/Create.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/FieldMaskRef.h>
#include <thrift/lib/cpp2/protocol/detail/FieldMask.h>
#include <thrift/lib/cpp2/protocol/detail/Object.h>

namespace apache::thrift::protocol {

// This is the return value of parseObject with mask.
// Masked fields are deserialized to included Object, and the other fields are
// are stored in excluded MaskedProtocolData.
struct MaskedDecodeResult {
  Object included;
  MaskedProtocolData excluded;
};

namespace detail {

// TODO(dokwon): Use Path once avaialble.
// Path of a field in a Thrift struct as a list of field ids.
using FieldPath = std::vector<FieldId>;

// Validates the mask with the given Struct. Ensures that mask doesn't contain
// fields not in the Struct.
template <typename T>
bool validate_mask(MaskRef ref) {
  // Get the field ids in the thrift struct type.
  std::unordered_set<FieldId> ids;
  ids.reserve(op::num_fields<T>);
  op::for_each_ordinal<T>(
      [&](auto ord) { ids.insert(op::get_field_id<T, decltype(ord)>()); });
  const FieldIdToMask& map = ref.mask.includes() ? ref.mask.includes().value()
                                                 : ref.mask.excludes().value();
  for (auto& [id, _] : map) {
    // Mask contains a field not in the struct.
    if (ids.find(FieldId{id}) == ids.end()) {
      return false;
    }
  }
  return true;
}

template <typename Tag>
bool is_compatible_with(const Mask&);

template <typename Tag>
bool is_compatible_with_impl(Tag, const Mask&) {
  return false;
}
template <typename T>
bool is_compatible_with_structured(const Mask& mask) {
  MaskRef ref{mask};
  if (!validate_mask<T>(ref)) {
    return false;
  }
  // Validates each field in the struct/union
  bool isValid = true;
  op::for_each_ordinal<T>([&](auto ord) {
    if (!isValid) { // short circuit
      return;
    }
    using Ord = decltype(ord);
    MaskRef next = ref.get(op::get_field_id<T, Ord>());
    if (next.isAllMask() || next.isNoneMask()) {
      return;
    }
    // Recurse
    isValid &= is_compatible_with<op::get_type_tag<T, Ord>>(next.mask);
  });
  return isValid;
}

inline bool is_compatible_with_any(const Mask& mask) {
  MaskRef ref{mask};
  if (ref.isTypeMask()) {
    return true;
  } else {
    // Backwards compatibility
    return is_compatible_with_structured<type::AnyStruct>(mask);
  }
}

template <typename T>
bool is_compatible_with_impl(type::struct_t<T>, const Mask& mask) {
  return is_compatible_with_structured<T>(mask);
}

inline bool is_compatible_with_impl(
    type::struct_t<type::AnyStruct>, const Mask& mask) {
  return is_compatible_with_any(mask);
}

inline bool is_compatible_with_impl(
    type::adapted<
        apache::thrift::InlineAdapter<::apache::thrift::type::AnyData>,
        type::struct_t<type::AnyStruct>>,
    const Mask& mask) {
  return is_compatible_with_any(mask);
}

template <typename T>
bool is_compatible_with_impl(type::union_t<T>, const Mask& mask) {
  return is_compatible_with_structured<T>(mask);
}

template <typename Key, typename Value>
bool is_compatible_with_impl(type::map<Key, Value>, const Mask& mask) {
  // Map mask is compatible only if all nested masks are compatible with
  // `Value`.
  if (const auto* m = getIntegerMapMask(mask)) {
    return std::all_of(m->begin(), m->end(), [](const auto& pair) {
      return is_compatible_with<Value>(pair.second);
    });
  }
  if (const auto* m = getStringMapMask(mask)) {
    return std::all_of(m->begin(), m->end(), [](const auto& pair) {
      return is_compatible_with<Value>(pair.second);
    });
  }
  return true;
}

template <typename Tag>
bool is_compatible_with(const Mask& mask) {
  if (isAllMask(mask) || isNoneMask(mask)) {
    return true;
  }
  return is_compatible_with_impl(Tag{}, mask);
}

// Throws an error if a thrift struct type is not compatible with the mask.
template <typename Tag>
void errorIfNotCompatible(const Mask& mask) {
  if (!::apache::thrift::protocol::detail::is_compatible_with<Tag>(mask)) {
    folly::throw_exception<std::runtime_error>(
        "The mask and struct are incompatible.");
  }
}

// This converts id list to a field mask with a single field.
template <typename Tag>
Mask path(const Mask& other) {
  // This is the base case as there is no more id.
  errorIfNotCompatible<Tag>(other);
  return other;
}

template <typename Tag, typename Id, typename... Ids>
Mask path(const Mask& other) {
  using T = type::native_type<Tag>;
  static_assert(is_thrift_class_v<T>);
  Mask mask;
  using fieldId = op::get_field_id<T, Id>;
  static_assert(fieldId::value != FieldId{});
  mask.includes().emplace()[static_cast<int16_t>(fieldId::value)] =
      path<op::get_type_tag<T, Id>, Ids...>(other);
  return mask;
}

// This converts field name list from the given index to a field mask with a
// single field.
template <typename Tag>
Mask path(
    const std::vector<folly::StringPiece>& fieldNames,
    size_t index,
    const Mask& other) {
  if (index == fieldNames.size()) {
    errorIfNotCompatible<Tag>(other);
    return other;
  }
  // static_assert doesn't work as it compiles this code for every field.
  using T = type::native_type<Tag>;
  if constexpr (is_thrift_class_v<T>) {
    Mask mask;
    op::for_each_field_id<T>([&](auto id) {
      using Id = decltype(id);
      if (mask.includes()) { // already set
        return;
      }
      if (op::get_name_v<T, Id> == fieldNames[index]) {
        mask.includes().emplace()[folly::to_underlying(id())] =
            path<op::get_type_tag<T, Id>>(fieldNames, index + 1, other);
      }
    });
    if (!mask.includes()) { // field not found
      folly::throw_exception<std::runtime_error>("field doesn't exist");
    }
    return mask;
  }
  folly::throw_exception<std::runtime_error>(
      "Path contains a non thrift struct/union field.");
}

void ensure_fields(MaskRef ref, type::AnyStruct&);

// Ensures the masked fields in the given thrift struct.
template <typename T>
void ensure_fields(MaskRef ref, T& t) {
  if (!validate_mask<T>(ref)) {
    folly::throw_exception<std::runtime_error>(
        "The mask and struct are incompatible.");
  }
  if (is_thrift_union_v<T> && ref.numFieldsSet<T>() > 1) {
    folly::throw_exception<std::runtime_error>(
        "Ensuring more than one field in union");
  }
  if constexpr (!std::is_const_v<std::remove_reference_t<T>>) {
    op::for_each_ordinal<T>([&](auto ord) {
      using Ord = decltype(ord);
      MaskRef next = ref.get(op::get_field_id<T, Ord>());
      if (next.isNoneMask()) {
        return;
      }
      using FieldTag = op::get_field_tag<T, Ord>;
      auto&& field_ref = op::get<Ord>(t);
      op::ensure<FieldTag>(field_ref, t);
      // Need to ensure the struct object.
      using FieldType = op::get_native_type<T, Ord>;
      if constexpr (is_thrift_class_v<FieldType>) {
        auto& value = *op::getValueOrNull(field_ref);
        ensure_fields(next, value);
        return;
      }
      if (!next.isAllMask()) {
        folly::throw_exception<std::runtime_error>(
            "The mask and struct are incompatible.");
      }
    });
  } else {
    folly::throw_exception<std::runtime_error>("Cannot ensure a const object");
  }
}

void clear_fields(MaskRef ref, type::AnyStruct& t);

inline void clear_fields(MaskRef ref, type::AnyData& t) {
  static_assert(std::is_same_v<type::AnyStruct&, decltype(t.toThrift())>);
  clear_fields(std::move(ref), t.toThrift());
}

// Clears the masked fields in the given thrift struct.
template <typename T>
void clear_fields(MaskRef ref, T& t) {
  if (!validate_mask<T>(ref)) {
    folly::throw_exception<std::runtime_error>(
        "The mask and struct are incompatible.");
  }
  if constexpr (!std::is_const_v<std::remove_reference_t<T>>) {
    op::for_each_ordinal<T>([&](auto ord) {
      using Ord = decltype(ord);
      MaskRef next = ref.get(op::get_field_id<T, Ord>());
      if (next.isNoneMask()) {
        return;
      }
      using FieldTag = op::get_field_tag<T, Ord>;
      auto&& field_ref = op::get<Ord>(t);
      if (next.isAllMask()) {
        op::clear_field<FieldTag>(field_ref, t);
        return;
      }
      using FieldType = op::get_native_type<T, Ord>;
      auto* field_value = op::getValueOrNull(field_ref);
      if (!field_value) {
        errorIfNotCompatible<op::get_type_tag<T, Ord>>(next.mask);
        return;
      }
      // Need to clear the struct/union object.
      if constexpr (is_thrift_class_v<FieldType>) {
        clear_fields(next, *field_value);
        return;
      }
      folly::throw_exception<std::runtime_error>(
          "The mask and struct are incompatible.");
    });
  } else {
    folly::throw_exception<std::runtime_error>("Cannot clear a const object");
  }
}

bool filter_fields(MaskRef ref, const type::AnyStruct& t, type::AnyStruct& ret);

inline bool filter_fields(
    MaskRef ref, const type::AnyData& t, type::AnyData& ret) {
  static_assert(std::is_same_v<type::AnyStruct&, decltype(ret.toThrift())>);
  return filter_fields(ref, t.toThrift(), ret.toThrift());
}

// Writes masked fields from src (as specified by ref) into ret (ret must be
// empty). Returns true if any masked field was written into ret.
template <typename T>
bool filter_fields(MaskRef ref, const T& src, T& ret) {
  if (!validate_mask<T>(ref)) {
    folly::throw_exception<std::runtime_error>(
        "The mask and struct are incompatible.");
  }
  bool retained = false;
  op::for_each_ordinal<T>([&](auto ord) {
    using Ord = decltype(ord);
    MaskRef next = ref.get(op::get_field_id<T, Ord>());
    // Id doesn't exist in field mask, skip.
    if (next.isNoneMask()) {
      return;
    }
    using FieldType = op::get_native_type<T, Ord>;
    auto&& src_ref = op::get<Ord>(src);
    auto&& ret_ref = op::get<Ord>(ret);
    bool srcHasValue = bool(op::getValueOrNull(src_ref));
    if (!srcHasValue) { // skip
      errorIfNotCompatible<op::get_type_tag<T, Ord>>(next.mask);
    } else if (next.isAllMask()) {
      if constexpr (is_thrift_union_v<T>) {
        // Simply copy the entire union over
        ret = src;
      } else {
        op::copy(src_ref, ret_ref);
      }
      retained = true;
    } else if constexpr (is_thrift_class_v<FieldType>) {
      FieldType nested;
      // If no masked fields are retained, leave this field unset (will leave
      // optional fields unset)
      if (filter_fields(next, *src_ref, nested)) {
        moveObject(ret_ref, std::move(nested));
        retained = true;
      }
    } else {
      folly::throw_exception<std::runtime_error>(
          "The mask and struct are incompatible.");
    }
  });
  return retained;
}

struct MaskedDecodeResultValue {
  Value included;
  MaskedData excluded;
};

// Stores the serialized data of the given type in maskedData and protocolData.
template <typename Protocol>
void setMaskedDataFull(
    Protocol& prot,
    TType arg_type,
    MaskedData& maskedData,
    MaskedProtocolData& protocolData) {
  auto& values = protocolData.values().ensure();
  auto& encodedValue = values.emplace_back();
  encodedValue.wireType() = type::toBaseType(arg_type);
  // get the serialized data from cursor
  auto cursor = prot.getCursor();
  apache::thrift::skip(prot, arg_type);
  cursor.clone(encodedValue.data().emplace(), prot.getCursor() - cursor);
  const auto pos = folly::to<int32_t>(values.size() - 1);
  maskedData.full() = type::ValueId{apache::thrift::util::i32ToZigzag(pos)};
}

inline MaskRef getKeyMaskRefByValue(MaskRef maskRef, const Value& value) {
  switch (value.getType()) {
    case Value::Type::byteValue:
      return maskRef.get(MapId{value.as_byte()});
    case Value::Type::i16Value:
      return maskRef.get(MapId{value.as_i16()});
    case Value::Type::i32Value:
      return maskRef.get(MapId{value.as_i32()});
    case Value::Type::i64Value:
      return maskRef.get(MapId{value.as_i64()});
    case Value::Type::stringValue:
      return maskRef.get(value.as_string());
    case Value::Type::binaryValue: {
      const auto& binaryValue = value.as_binary();

      if (binaryValue.isChained()) {
        std::string key = binaryValue.toString();
        return maskRef.get(key);
      }

      std::string_view key(
          reinterpret_cast<const char*>(binaryValue.data()),
          binaryValue.length());
      return maskRef.get(key);
    }
    case Value::Type::boolValue:
    case Value::Type::floatValue:
    case Value::Type::doubleValue:
    case Value::Type::objectValue:
    case Value::Type::listValue:
    case Value::Type::setValue:
    case Value::Type::mapValue:
    case Value::Type::__EMPTY__:
      folly::throw_exception<std::runtime_error>("Value is empty");
    default:
      folly::throw_exception<std::runtime_error>(folly::to<std::string>(
          "Unrecognized value type ",
          static_cast<std::int64_t>(value.getType())));
  }
}

// parseValue with readMaskRef and writeMaskRef
template <bool KeepExcludedData, typename Protocol>
MaskedDecodeResultValue parseValueWithMask(
    Protocol& prot,
    TType arg_type,
    MaskRef readMaskRef,
    MaskRef writeMaskRef,
    MaskedProtocolData& protocolData,
    bool string_to_binary = true) {
  MaskedDecodeResultValue result;
  if (arg_type == protocol::T_BOOL) {
    // For Compact protocol, Bool field values are encoded directly in the field
    // header, in which case the encoded value will be empty.
    //
    // https://github.com/apache/thrift/blob/master/doc/specs/thrift-compact-protocol.md#boolean-encoding
    //
    // Given that the bool field is cheap to decode, we can just decode it
    // unconditionally.
    prot.readBool(result.included.emplace_bool());
    return result;
  }
  if (readMaskRef.isAllMask()) { // serialize all
    parseValueInplace(prot, arg_type, result.included, string_to_binary);
    return result;
  }
  if (readMaskRef.isNoneMask()) { // do not deserialize
    if constexpr (!KeepExcludedData) { // no need to store
      apache::thrift::skip(prot, arg_type);
      return result;
    }
    if (writeMaskRef.isNoneMask()) { // store the serialized data
      setMaskedDataFull(prot, arg_type, result.excluded, protocolData);
      return result;
    }
    if (writeMaskRef.isAllMask()) { // no need to store
      apache::thrift::skip(prot, arg_type);
      return result;
    }
    // Need to recursively store the result not in writeMaskRef.
  }
  if (arg_type == protocol::T_STRUCT) {
    auto& object = result.included.ensure_object();
    std::string name;
    int16_t fid;
    TType ftype;
    prot.readStructBegin(name);
    while (true) {
      prot.readFieldBegin(name, ftype, fid);
      if (ftype == protocol::T_STOP) {
        break;
      }
      MaskRef nextRead = readMaskRef.get(FieldId{fid});
      MaskRef nextWrite = writeMaskRef.get(FieldId{fid});
      MaskedDecodeResultValue nestedResult =
          parseValueWithMask<KeepExcludedData>(
              prot, ftype, nextRead, nextWrite, protocolData, string_to_binary);
      // Set nested MaskedDecodeResult if not empty.
      if (!apache::thrift::empty(nestedResult.included.toThrift())) {
        object[FieldId{fid}] = std::move(nestedResult.included);
      }
      if constexpr (KeepExcludedData) {
        if (!apache::thrift::empty(nestedResult.excluded)) {
          result.excluded.fields().ensure()[FieldId{fid}] =
              std::move(nestedResult.excluded);
        }
      }
      prot.readFieldEnd();
    }
    prot.readStructEnd();
    return result;
  } else if (arg_type == protocol::T_MAP) {
    auto& map = result.included.ensure_map();
    TType keyType;
    TType valType;
    uint32_t size;
    prot.readMapBegin(keyType, valType, size);
    if (!size) {
      prot.readMapEnd();
      return result;
    }
    for (uint32_t i = 0; i < size; i++) {
      auto keyValue = parseValue(prot, keyType, string_to_binary);
      MaskRef nextRead = getKeyMaskRefByValue(readMaskRef, keyValue);
      MaskRef nextWrite = getKeyMaskRefByValue(writeMaskRef, keyValue);
      MaskedDecodeResultValue nestedResult =
          parseValueWithMask<KeepExcludedData>(
              prot,
              valType,
              nextRead,
              nextWrite,
              protocolData,
              string_to_binary);
      // Set nested MaskedDecodeResult if not empty.
      if (!apache::thrift::empty(nestedResult.included.toThrift())) {
        map[keyValue] = std::move(nestedResult.included);
      }
      if constexpr (KeepExcludedData) {
        if (!apache::thrift::empty(nestedResult.excluded)) {
          auto& keys = protocolData.keys().ensure();
          keys.push_back(keyValue);
          const auto pos = folly::to<int32_t>(keys.size() - 1);
          type::ValueId id =
              type::ValueId{apache::thrift::util::i32ToZigzag(pos)};
          result.excluded.values().ensure()[id] =
              std::move(nestedResult.excluded);
        }
      }
    }
    prot.readMapEnd();
    return result;
  } else {
    parseValueInplace(prot, arg_type, result.included, string_to_binary);
    return result;
  }
}

template <typename Protocol, bool KeepExcludedData>
MaskedDecodeResult parseObject(
    const folly::IOBuf& buf,
    const Mask& readMask,
    const Mask& writeMask,
    bool string_to_binary = true) {
  Protocol prot;
  prot.setInput(&buf);
  MaskedDecodeResult result;
  MaskedProtocolData& protocolData = result.excluded;
  protocolData.protocol() = get_standard_protocol<Protocol>;
  MaskedDecodeResultValue parseValueResult =
      parseValueWithMask<KeepExcludedData>(
          prot,
          T_STRUCT,
          MaskRef{readMask, false},
          MaskRef{writeMask, false},
          protocolData,
          string_to_binary);
  protocolData.data() = std::move(parseValueResult.excluded);
  // Calling ensure as it is possible that the value is not set.
  result.included = std::move(parseValueResult.included.ensure_object());
  return result;
}

Mask fieldPathToMask(const FieldPath& path, const Mask& other);

} // namespace detail
} // namespace apache::thrift::protocol
