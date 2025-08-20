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
#include <functional>
#include <stack>
#include <type_traits>
#include <unordered_set>

#include <folly/CPortability.h>
#include <folly/Conv.h>
#include <folly/Utility.h>
#include <folly/container/Reserve.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/GetStandardProtocol.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>
#include <thrift/lib/cpp2/type/Any.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_constants.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_types.h>
#include <thrift/lib/thrift/gen-cpp2/id_types.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol::detail {

template <typename Tag>
struct HasStructuredTag : std::false_type {};
template <typename Tag>
struct HasStructuredTag<type::list<Tag>> : HasStructuredTag<Tag> {};
template <typename Tag>
struct HasStructuredTag<type::set<Tag>> : HasStructuredTag<Tag> {};
template <typename KTag, typename VTag>
struct HasStructuredTag<type::map<KTag, VTag>> {
  static constexpr bool value =
      HasStructuredTag<KTag>::value || HasStructuredTag<VTag>::value;
};
template <typename T, typename Tag>
struct HasStructuredTag<type::cpp_type<T, Tag>> : HasStructuredTag<Tag> {};
template <typename Adapter, typename Tag>
struct HasStructuredTag<type::adapted<Adapter, Tag>> : HasStructuredTag<Tag> {};
template <typename Tag, typename FieldContext>
struct HasStructuredTag<type::field<Tag, FieldContext>>
    : HasStructuredTag<Tag> {};
template <typename T>
struct HasStructuredTag<type::struct_t<T>> : std::true_type {};
template <typename T>
struct HasStructuredTag<type::union_t<T>> : std::true_type {};
template <typename T>
struct HasStructuredTag<type::exception_t<T>> : std::true_type {};

template <typename Tag>
inline constexpr bool has_structured_tag_v = HasStructuredTag<Tag>::value;

template <typename C, typename T>
decltype(auto) forward_elem(T& elem) {
  if constexpr (std::is_lvalue_reference_v<C>) {
    return elem;
  } else {
    return static_cast<T&&>(elem);
  }
}

template <typename TT, typename = void>
struct ValueHelper {
  template <typename T>
  static void set(Value& result, T&& value) {
    if constexpr (false) {
    } else if constexpr (type::base_type_v<TT> == type::BaseType::Bool) {
      result.emplace_bool(value);
    } else if constexpr (type::base_type_v<TT> == type::BaseType::Byte) {
      result.emplace_byte(value);
    } else if constexpr (type::base_type_v<TT> == type::BaseType::I16) {
      result.emplace_i16(value);
    } else if constexpr (type::base_type_v<TT> == type::BaseType::I32) {
      result.emplace_i32(value);
    } else if constexpr (type::base_type_v<TT> == type::BaseType::I64) {
      result.emplace_i64(value);
    } else if constexpr (type::base_type_v<TT> == type::BaseType::Enum) {
      result.emplace_i32(static_cast<int32_t>(value));
    } else if constexpr (type::base_type_v<TT> == type::BaseType::Float) {
      result.emplace_float(value);
    } else if constexpr (type::base_type_v<TT> == type::BaseType::Double) {
      result.emplace_double(value);
    } else if constexpr (type::base_type_v<TT> == type::BaseType::String) {
      result.emplace_string(std::forward<T>(value));
    } else {
      static_assert(folly::always_false<T>, "Unknown Type Tag.");
    }
  }
};

template <>
struct ValueHelper<type::binary_t> {
  static void set(Value& result, folly::IOBuf value) {
    result.emplace_binary(std::move(value));
  }
  static void set(Value& result, std::string_view value) {
    result.emplace_binary(
        folly::IOBuf{folly::IOBuf::COPY_BUFFER, value.data(), value.size()});
  }
  static void set(Value& result, folly::ByteRange value) {
    result.emplace_binary(
        folly::IOBuf{folly::IOBuf::COPY_BUFFER, value.data(), value.size()});
  }
};

template <typename V>
struct ValueHelper<type::list<V>> {
  template <typename C>
  static void set(Value& result, C&& value) {
    auto& result_list = result.emplace_list();
    for (auto& elem : value) {
      ValueHelper<V>::set(result_list.emplace_back(), forward_elem<C>(elem));
    }
  }
};

template <typename V>
struct ValueHelper<type::set<V>> {
  template <typename C>
  static void set(Value& result, C&& value) {
    auto& result_set = result.emplace_set();
    for (auto& elem : value) {
      Value elem_val;
      ValueHelper<V>::set(elem_val, forward_elem<C>(elem));
      result_set.emplace(std::move(elem_val));
    }
  }
};

template <typename K, typename V>
struct ValueHelper<type::map<K, V>> {
  template <typename C>
  static void set(Value& result, C&& value) {
    auto& result_map = result.emplace_map();
    for (auto& entry : value) {
      Value key;
      ValueHelper<K>::set(key, entry.first);
      ValueHelper<V>::set(result_map[key], forward_elem<C>(entry.second));
    }
  }
};

template <typename T, typename Tag>
struct ValueHelper<type::cpp_type<T, Tag>> : ValueHelper<Tag> {};

template <typename TT, typename T = type::native_type<TT>>
Value asValueStruct(T&& value) {
  Value result;
  ValueHelper<TT>::set(result, std::forward<T>(value));
  return result;
}

// Clears whatever value-variant is set (instead of clearing the union itself)
// Eg. if value is a string, this will clear the string instead of setting value
// to empty-union
void clearValueInner(Value& value);

class BaseObjectAdapter {
 public:
  static constexpr ProtocolType protocolType() { return {}; }
  static constexpr bool kUsesFieldNames() { return true; }
  static constexpr bool kOmitsContainerSizes() { return false; }
  static constexpr bool kSortKeys() { return false; }
  static constexpr bool kHasIndexSupport() { return false; }
};

class ObjectWriter : public BaseObjectAdapter {
 public:
  explicit ObjectWriter(Value* target) {
    assert(target != nullptr);
    cur_.emplace(target);
  }

  uint32_t writeStructBegin(const char* /*name*/) {
    beginValue().emplace_object();
    return 0;
  }
  uint32_t writeStructEnd() { return endValue(Value::Type::objectValue); }

  uint32_t writeFieldBegin(
      const char* /*name*/, TType /*fieldType*/, int16_t fieldId) {
    auto result = cur(Value::Type::objectValue)
                      .as_object()
                      .members()
                      ->emplace(fieldId, Value());
    assert(result.second);
    cur_.push(&result.first->second);
    return 0;
  }

  uint32_t writeFieldEnd() { return 0; }

  uint32_t writeFieldStop() { return 0; }

  uint32_t writeMapBegin(
      const TType /*keyType*/, TType /*valType*/, uint32_t size) {
    // We cannot push reference to map elements on stack without first inserting
    // map elements. So push reference to temporary buffer on stack instead.
    allocBufferPushOnStack((size_t)size * 2);
    return 0;
  }

  uint32_t writeMapEnd() {
    // insert elements from buffer into mapValue
    std::vector<Value> mapKeyAndValues = getBufferFromStack();
    assert(mapKeyAndValues.size() % 2 == 0);
    auto& mapVal = cur().emplace_map();
    mapVal.reserve(mapKeyAndValues.size() / 2);
    for (size_t i = 0; i < mapKeyAndValues.size(); i += 2) {
      mapVal.emplace(
          std::move(mapKeyAndValues[i]), std::move(mapKeyAndValues[i + 1]));
    }
    return endValue(Value::Type::mapValue);
  }

  uint32_t writeListBegin(TType /*elemType*/, uint32_t size) {
    allocBufferPushOnStack(size);
    return 0;
  }

  uint32_t writeListEnd() { return endValue(Value::Type::listValue); }

  uint32_t writeSetBegin(TType /*elemType*/, uint32_t size) {
    // We cannot push reference to set elements on stack without first inserting
    // set elements. So push reference to temporary buffer on stack instead.
    allocBufferPushOnStack(size);
    return 0;
  }

  uint32_t writeSetEnd() {
    // insert elements from buffer into setValue
    std::vector<Value> setValues = getBufferFromStack();
    auto& setVal = cur().emplace_set();
    setVal.reserve(setValues.size());
    for (size_t i = 0; i < setValues.size(); i++) {
      setVal.emplace(std::move(setValues[i]));
    }
    return endValue(Value::Type::setValue);
  }

  uint32_t writeBool(bool value) {
    ValueHelper<type::bool_t>::set(beginValue(), value);
    return endValue(Value::Type::boolValue);
  }

  uint32_t writeByte(int8_t value) {
    ValueHelper<type::byte_t>::set(beginValue(), value);
    return endValue(Value::Type::byteValue);
  }

  uint32_t writeI16(int16_t value) {
    ValueHelper<type::i16_t>::set(beginValue(), value);
    return endValue(Value::Type::i16Value);
  }

  uint32_t writeI32(int32_t value) {
    ValueHelper<type::i32_t>::set(beginValue(), value);
    return endValue(Value::Type::i32Value);
  }

  uint32_t writeI64(int64_t value) {
    ValueHelper<type::i64_t>::set(beginValue(), value);
    return endValue(Value::Type::i64Value);
  }

  uint32_t writeFloat(float value) {
    ValueHelper<type::float_t>::set(beginValue(), value);
    return endValue(Value::Type::floatValue);
  }

  int32_t writeDouble(double value) {
    ValueHelper<type::double_t>::set(beginValue(), value);
    return endValue(Value::Type::doubleValue);
  }

  uint32_t writeString(folly::StringPiece value) {
    // TODO: set in stringValue if UTF8
    return writeBinary(value);
  }

  uint32_t writeBinary(folly::ByteRange value) {
    ValueHelper<type::binary_t>::set(beginValue(), value);
    return endValue(Value::Type::binaryValue);
  }

  uint32_t writeBinary(const folly::IOBuf& value) {
    ValueHelper<type::binary_t>::set(beginValue(), value);
    return endValue(Value::Type::binaryValue);
  }

  uint32_t writeBinary(const std::unique_ptr<folly::IOBuf>& str) {
    if (!str) {
      return writeBinary(folly::ByteRange{});
    }
    return writeBinary(*str);
  }

  uint32_t writeBinary(folly::StringPiece value) {
    return writeBinary(folly::ByteRange(value));
  }

 protected:
  std::stack<Value*> cur_;

  void checkCur(Value::Type required) {
    (void)required;
    assert(cur().getType() == required);
  }

  Value& cur(Value::Type required) {
    checkCur(required);
    return *cur_.top();
  }

  Value& cur() {
    assert(!cur_.empty());
    return *cur_.top();
  }

  Value& beginValue() {
    checkCur(Value::Type::__EMPTY__);
    return cur();
  }

  uint32_t endValue(Value::Type required) {
    checkCur(required);
    cur_.pop();
    return 0;
  }

  // Allocated temporary buffer in cur() and pushes buffer references on stack
  void allocBufferPushOnStack(size_t n) {
    // using listVal as temporary buffer
    std::vector<Value>& listVal = beginValue().emplace_list();
    listVal.resize(n);
    for (auto itr = listVal.rbegin(); itr != listVal.rend(); ++itr) {
      cur_.push(&*itr);
    }
  }

  // Get temporary buffer from cur()
  std::vector<Value> getBufferFromStack() {
    return std::move(cur(Value::Type::listValue).as_list());
  }
};

template <typename Adapter, typename Tag>
struct ValueHelper<type::adapted<Adapter, Tag>> {
  template <typename T>
  static void set(Value& result, T&& value) {
    ObjectWriter writer(&result);
    op::encode<type::adapted<Adapter, Tag>>(writer, std::forward<T>(value));
  }
};
template <class T>
inline constexpr bool kIsStructured = false;
template <class T>
inline constexpr bool kIsStructured<type::struct_t<T>> = true;
template <class T>
inline constexpr bool kIsStructured<type::union_t<T>> = true;
template <class T>
inline constexpr bool kIsStructured<type::exception_t<T>> = true;
template <>
inline constexpr bool kIsStructured<type::struct_c> = true;
template <>
inline constexpr bool kIsStructured<type::union_c> = true;
template <>
inline constexpr bool kIsStructured<type::exception_c> = true;

// Specialization for all structured types.
template <typename TT>
struct ValueHelper<TT, std::enable_if_t<kIsStructured<TT>>> {
  template <typename T>
  static void set(Value& result, T&& value) {
    ObjectWriter writer(&result);
    op::encode<type::infer_tag<folly::remove_cvref_t<T>>>(
        writer, std::forward<T>(value));
  }
};

inline TType unifyStringType(TType type) {
  if (type == protocol::T_UTF7 || type == protocol::T_UTF8 ||
      type == protocol::T_UTF16) {
    return protocol::T_STRING;
  }
  return type;
}

template <class Protocol>
Value parseValue(Protocol& prot, TType arg_type, bool string_to_binary = true);

template <class Protocol>
void parseObjectInplace(
    Protocol& prot, Object& objectValue, bool string_to_binary = true) {
  std::string name;
  int16_t fid;
  TType ftype;
  prot.readStructBegin(name);
  while (true) {
    prot.readFieldBegin(name, ftype, fid);
    if (ftype == protocol::T_STOP) {
      break;
    }
    parseValueInplace(prot, ftype, objectValue[FieldId{fid}], string_to_binary);
    prot.readFieldEnd();
  }
  prot.readStructEnd();
}

// Schemaless deserialization of thrift serialized data of specified
// thrift type into conformance::Value
// Protocol: protocol to use eg. apache::thrift::BinaryProtocolReader
// TODO: handle jsonprotocol
template <class Protocol>
void parseValueInplace(
    Protocol& prot,
    TType arg_type,
    Value& result,
    bool string_to_binary = true) {
  switch (unifyStringType(arg_type)) {
    case protocol::T_BOOL: {
      bool boolv;
      prot.readBool(boolv);
      result.emplace_bool(boolv);
      break;
    }
    case protocol::T_BYTE: {
      int8_t bytev = 0;
      prot.readByte(bytev);
      result.emplace_byte(bytev);
      break;
    }
    case protocol::T_I16: {
      int16_t i16;
      prot.readI16(i16);
      result.emplace_i16(i16);
      break;
    }
    case protocol::T_I32: {
      int32_t i32;
      prot.readI32(i32);
      result.emplace_i32(i32);
      break;
    }
    case protocol::T_I64: {
      int64_t i64;
      prot.readI64(i64);
      result.emplace_i64(i64);
      break;
    }
    case protocol::T_DOUBLE: {
      double dub;
      prot.readDouble(dub);
      result.emplace_double(dub);
      break;
    }
    case protocol::T_FLOAT: {
      float flt;
      prot.readFloat(flt);
      result.emplace_float(flt);
      break;
    }
    case protocol::T_STRING: {
      if (string_to_binary) {
        auto& binaryValue = result.emplace_binary();
        prot.readBinary(binaryValue);
        break;
      }
      auto& stringValue = result.emplace_string();
      prot.readString(stringValue);
      break;
    }
    case protocol::T_STRUCT: {
      parseObjectInplace(prot, result.emplace_object(), string_to_binary);
      break;
    }
    case protocol::T_MAP: {
      TType keyType;
      TType valType;
      uint32_t size;
      auto& mapValue = result.emplace_map();
      prot.readMapBegin(keyType, valType, size);
      if (!canReadNElements(prot, size, {keyType, valType})) {
        TProtocolException::throwTruncatedData();
      }
      mapValue.reserve(size);
      for (uint32_t i = 0; i < size; i++) {
        parseValueInplace(
            prot,
            valType,
            mapValue[parseValue(prot, keyType, string_to_binary)],
            string_to_binary);
      }
      prot.readMapEnd();
      break;
    }
    case protocol::T_SET: {
      TType elemType;
      uint32_t size;
      auto& setValue = result.emplace_set();
      prot.readSetBegin(elemType, size);
      if (!canReadNElements(prot, size, {elemType})) {
        TProtocolException::throwTruncatedData();
      }
      setValue.reserve(size);
      for (uint32_t i = 0; i < size; i++) {
        setValue.insert(parseValue(prot, elemType, string_to_binary));
      }
      prot.readSetEnd();
      break;
    }
    case protocol::T_LIST: {
      TType elemType;
      uint32_t size;
      prot.readListBegin(elemType, size);
      auto& listValue = result.emplace_list();
      if (!canReadNElements(prot, size, {elemType})) {
        TProtocolException::throwTruncatedData();
      }
      listValue.resize(size);
      for (auto& v : listValue) {
        parseValueInplace(prot, elemType, v, string_to_binary);
      }
      prot.readListEnd();
      break;
    }
    case T_STOP:
    case T_VOID:
    case T_U64:
    case T_UTF8:
    case T_UTF16:
    case T_STREAM:
    default: {
      TProtocolException::throwInvalidSkipType(arg_type);
    }
  }
}

template <class Protocol>
Value parseValue(Protocol& prot, TType arg_type, bool string_to_binary) {
  Value result;
  parseValueInplace(prot, arg_type, result, string_to_binary);
  return result;
}

// Returns an element in the list by ValueId.
template <typename T>
const T& getByValueId(const std::vector<T>& values, type::ValueId id) {
  return values[apache::thrift::util::zigzagToI64(static_cast<int64_t>(id))];
}

inline TType getTType(const Value& val) {
  auto type = toTType(static_cast<type::BaseType>(val.getType()));
  return unifyStringType(type);
}

inline void ensureSameType(const Value& a, TType b) {
  if (getTType(a) != b) {
    TProtocolException::throwInvalidFieldData();
  }
}

template <class Protocol>
uint32_t serializeValue(Protocol& prot, const Value& value) {
  switch (value.getType()) {
    case Value::Type::boolValue:
      return prot.writeBool(value.as_bool());
    case Value::Type::byteValue:
      return prot.writeByte(value.as_byte());
    case Value::Type::i16Value:
      return prot.writeI16(value.as_i16());
    case Value::Type::i32Value:
      return prot.writeI32(value.as_i32());
    case Value::Type::i64Value:
      return prot.writeI64(value.as_i64());
    case Value::Type::floatValue:
      return prot.writeFloat(value.as_float());
    case Value::Type::doubleValue:
      return prot.writeDouble(value.as_double());
    case Value::Type::stringValue:
      return prot.writeString(value.as_string());
    case Value::Type::binaryValue:
      return prot.writeBinary(value.as_binary());
    case Value::Type::listValue: {
      TType elemType = protocol::T_I64;
      const auto& listVal = value.as_list();
      const auto size = listVal.size();
      if (size > 0) {
        elemType = getTType(listVal.at(0));
      }
      auto serializedSize = prot.writeListBegin(elemType, size);
      for (const auto& val : listVal) {
        ensureSameType(val, elemType);
        serializedSize += serializeValue(prot, val);
      }
      serializedSize += prot.writeListEnd();
      return serializedSize;
    }
    case Value::Type::mapValue: {
      TType keyType = protocol::T_STRING;
      TType valueType = protocol::T_I64;
      const auto& mapVal = value.as_map();
      const auto size = mapVal.size();
      if (size > 0) {
        keyType = getTType(mapVal.begin()->first);
        valueType = getTType(mapVal.begin()->second);
      }
      auto serializedSize = prot.writeMapBegin(keyType, valueType, size);
      for (const auto& [key, val] : mapVal) {
        ensureSameType(key, keyType);
        ensureSameType(val, valueType);
        serializedSize += serializeValue(prot, key);
        serializedSize += serializeValue(prot, val);
      }
      serializedSize += prot.writeMapEnd();
      return serializedSize;
    }
    case Value::Type::setValue: {
      TType elemType = protocol::T_I64;
      const auto& setVal = value.as_set();
      const auto size = setVal.size();
      if (size > 0) {
        elemType = getTType(*setVal.begin());
      }
      auto serializedSize = prot.writeSetBegin(elemType, size);
      for (const auto& val : setVal) {
        ensureSameType(val, elemType);
        serializedSize += serializeValue(prot, val);
      }
      serializedSize += prot.writeSetEnd();
      return serializedSize;
    }
    case Value::Type::objectValue: {
      return serializeObject(prot, value.as_object());
    }
    case Value::Type::__EMPTY__:
    default: {
      TProtocolException::throwInvalidFieldData();
    }
  }
  return 0;
}

template <class Protocol>
uint32_t serializeObject(Protocol& prot, const Object& obj) {
  uint32_t serializedSize = 0;
  serializedSize += prot.writeStructBegin("");
  if (obj.members()) {
    for (const auto& [fieldID, fieldVal] : *obj.members()) {
      auto fieldType = getTType(fieldVal);
      serializedSize += prot.writeFieldBegin("", fieldType, fieldID);
      serializedSize += serializeValue(prot, fieldVal);
      serializedSize += prot.writeFieldEnd();
    }
  }
  serializedSize += prot.writeFieldStop();
  serializedSize += prot.writeStructEnd();
  return serializedSize;
}

// Writes the field from raw data in MaskedData.
template <class Protocol>
void writeRawField(
    Protocol& prot,
    FieldId fieldId,
    const MaskedProtocolData& protocolData,
    const MaskedData& maskedData) {
  const auto& nestedMaskedData = maskedData.fields().value().at(fieldId);
  // When value doesn't exist in the object, maskedData should have full field.
  if (!nestedMaskedData.full()) {
    throw std::runtime_error("incompatible value and maskedData");
  }
  type::ValueId valueId = nestedMaskedData.full().value();
  const EncodedValue& value = getByValueId(*protocolData.values(), valueId);
  prot.writeFieldBegin(
      "", toTType(*value.wireType()), folly::to_underlying(fieldId));
  prot.writeRaw(*value.data());
  prot.writeFieldEnd();
}

// Writes the map value from raw data in MaskedData.
template <class Protocol>
void writeRawMapValue(
    Protocol& prot,
    TType valueType,
    const MaskedProtocolData& protocolData,
    const MaskedData& maskedData) {
  // When value doesn't exist in the object, maskedData should have full field.
  if (!maskedData.full()) {
    throw std::runtime_error("incompatible value and maskedData");
  }
  type::ValueId valueId = maskedData.full().value();
  const EncodedValue& value = getByValueId(*protocolData.values(), valueId);
  if (toTType(*value.wireType()) != valueType) {
    TProtocolException::throwInvalidFieldData();
  }
  prot.writeRaw(*value.data());
}

template <class Protocol>
void serializeObject(
    Protocol& prot,
    const Object& obj,
    const MaskedProtocolData& protocolData,
    const MaskedData& maskedData) {
  if (!maskedData.fields()) {
    throw std::runtime_error("incompatible value and maskedData");
  }
  prot.writeStructBegin("");
  // It is more efficient to serialize with sorted field ids.
  std::set<FieldId> fieldIds{};
  for (const auto& [fieldId, _] : obj) {
    fieldIds.insert(FieldId{fieldId});
  }
  for (const auto& [fieldId, _] : *maskedData.fields()) {
    fieldIds.insert(fieldId);
  }

  for (auto fieldId : fieldIds) {
    if (!obj.contains(fieldId)) {
      // no need to serialize the value
      writeRawField(prot, fieldId, protocolData, maskedData);
      continue;
    }
    // get type from value
    const auto& fieldVal = obj.at(fieldId);
    auto fieldType = getTType(fieldVal);
    prot.writeFieldBegin("", fieldType, folly::to_underlying(fieldId));
    // just serialize the value
    if (folly::get_ptr(*maskedData.fields(), fieldId) == nullptr) {
      serializeValue(prot, fieldVal);
    } else { // recursively serialize value with maskedData
      const auto& nextMaskedData = maskedData.fields().value().at(fieldId);
      serializeValue(prot, fieldVal, protocolData, nextMaskedData);
    }
    prot.writeFieldEnd();
  }
  prot.writeFieldStop();
  prot.writeStructEnd();
  return;
}

// We can assume that if value type is a struct, fields in maskedData is
// active, and if value type is a map, values in maskedData is active.
// It throws a runtime error if value and maskedData are incompatible.
template <class Protocol>
void serializeValue(
    Protocol& prot,
    const Value& value,
    const MaskedProtocolData& protocolData,
    const MaskedData& maskedData) {
  if (value.getType() == Value::Type::objectValue) {
    return serializeObject(prot, value.as_object(), protocolData, maskedData);
  }

  if (value.getType() == Value::Type::mapValue) {
    if (!maskedData.values()) {
      throw std::runtime_error("incompatible value and maskedData");
    }
    TType keyType = protocol::T_STRING;
    TType valueType = protocol::T_I64;

    // compute size, keyType, and valueType
    const auto& mapVal = value.as_map();
    auto size = mapVal.size();
    if (size > 0) {
      keyType = getTType(mapVal.begin()->first);
      valueType = getTType(mapVal.begin()->second);
    }
    for (auto& [keyValueId, nestedMaskedData] : *maskedData.values()) {
      const Value& key = getByValueId(*protocolData.keys(), keyValueId);
      if (size == 0) { // need to set keyType and valueType
        keyType = getTType(key);
        type::ValueId valueId = nestedMaskedData.full().value();
        valueType =
            toTType(*getByValueId(*protocolData.values(), valueId).wireType());
      }
      if (folly::get_ptr(mapVal, key) == nullptr) {
        ++size;
      }
    }

    // Remember which keys are in the maskedData. Note that the ownership of
    // keys are managed by the maskedData.
    std::unordered_set<
        std::reference_wrapper<const Value>,
        std::hash<Value>,
        std::equal_to<Value>>
        keys;
    prot.writeMapBegin(keyType, valueType, size);
    for (auto& [keyValueId, nestedMaskedData] : *maskedData.values()) {
      const Value& key = getByValueId(*protocolData.keys(), keyValueId);
      keys.insert(key);
      ensureSameType(key, keyType);
      serializeValue(prot, key);
      // no need to serialize the value
      if (folly::get_ptr(mapVal, key) == nullptr) {
        writeRawMapValue(prot, valueType, protocolData, nestedMaskedData);
        continue;
      }
      // recursively serialize value with maskedData
      const Value& val = mapVal.at(key);
      ensureSameType(val, valueType);
      serializeValue(prot, val, protocolData, nestedMaskedData);
    }
    for (const auto& [key, val] : mapVal) {
      if (keys.find(key) != keys.end()) { // already serailized
        continue;
      }
      ensureSameType(key, keyType);
      ensureSameType(val, valueType);
      serializeValue(prot, key);
      serializeValue(prot, val);
    }
    prot.writeMapEnd();
    return;
  }

  serializeValue(prot, value);
}

template <class Protocol>
void serializeValue(const Value& val, folly::IOBufQueue& queue) {
  Protocol prot;
  prot.setOutput(&queue);
  serializeValue(prot, val);
}

template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeValue(const Value& val) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  serializeValue<Protocol>(val, queue);
  return queue.move();
}

type::Type toType(const protocol::Value& value);

type::AnyData toAny(
    const Value& value, type::Type type, type::Protocol protocol);

/**
 * Schemaless conversion from any -> value contained within
 * Currently supports only compact/binary protocols
 */
Value parseValueFromAny(const type::AnyStruct& any);
Value parseValueFromAny(const type::AnyData& any);
// anyObject's schema much match that of AnyStruct
Value parseValueFromAnyObject(const Object& anyObject);

template <typename Tag>
struct ProtocolValueToThriftValue;

template <>
struct ProtocolValueToThriftValue<type::bool_t> {
  // return whether conversion succeed
  template <typename T>
  bool operator()(const Value& value, T& b) const {
    if (auto p = value.if_bool()) {
      b = *p;
      return true;
    }

    return false;
  }
};

template <>
struct ProtocolValueToThriftValue<type::byte_t> {
  template <typename T>
  bool operator()(const Value& value, T& i) const {
    if (auto p = value.if_byte()) {
      i = *p;
      return true;
    }

    return false;
  }
};

template <>
struct ProtocolValueToThriftValue<type::i16_t> {
  template <typename T>
  bool operator()(const Value& value, T& i) const {
    if (auto p = value.if_i16()) {
      i = *p;
      return true;
    }

    return false;
  }
};

template <>
struct ProtocolValueToThriftValue<type::i32_t> {
  template <typename T>
  bool operator()(const Value& value, T& i) const {
    if (auto p = value.if_i32()) {
      i = *p;
      return true;
    }
    return false;
  }
};

template <>
struct ProtocolValueToThriftValue<type::i64_t> {
  template <typename T>
  bool operator()(const Value& value, T& i) const {
    if (auto p = value.if_i64()) {
      i = *p;
      return true;
    }

    return false;
  }
};

template <>
struct ProtocolValueToThriftValue<type::float_t> {
  template <typename T>
  bool operator()(const Value& value, T& f) const {
    if (auto p = value.if_float()) {
      f = *p;
      return true;
    }
    return false;
  }
};

template <>
struct ProtocolValueToThriftValue<type::double_t> {
  template <typename T>
  bool operator()(const Value& value, T& d) const {
    if (auto p = value.if_double()) {
      d = *p;
      return true;
    }
    return false;
  }
};

template <>
struct ProtocolValueToThriftValue<type::string_t> {
  template <typename StrType>
  bool operator()(const Value& value, StrType& s) const {
    if (auto p = value.if_string()) {
      s = *p;
      return true;
    }
    if (auto p = value.if_binary()) {
      s.clear();
      folly::io::Cursor cursor{&*p};
      while (!cursor.isAtEnd()) {
        const auto buf = cursor.peekBytes();
        s.append((const char*)buf.data(), buf.size());
        cursor += buf.size();
      }
      return true;
    }

    return false;
  }

  bool operator()(const Value& value, std::unique_ptr<folly::IOBuf>& s) const {
    if (auto p = value.if_string()) {
      s = folly::IOBuf::copyBuffer(p->data(), p->size());
      return true;
    }
    if (auto p = value.if_binary()) {
      s = p->clone();
      return true;
    }

    return false;
  }

  bool operator()(const Value& value, folly::IOBuf& s) const {
    std::unique_ptr<folly::IOBuf> buf;
    if (operator()(value, buf)) {
      s = *buf;
      return true;
    }
    return false;
  }
};

// We can't distinguish string/binary type in binary/compact protocol,
// Thus we need to handle them in the same way.
template <>
struct ProtocolValueToThriftValue<type::binary_t>
    : ProtocolValueToThriftValue<type::string_t> {};

template <typename T>
struct ProtocolValueToThriftValue<type::enum_t<T>> {
  template <typename U>
  bool operator()(const Value& value, U& t) const {
    if (auto p = value.if_i32()) {
      t = static_cast<T>(*p);
      return true;
    }
    return false;
  }
};

template <typename Tag>
struct ProtocolValueToThriftValue<type::list<Tag>> {
  template <typename ListType>
  bool operator()(const Value& value, ListType& list) const {
    auto p = value.if_list();
    if (!p) {
      return false;
    }
    bool ret = true;
    list.clear();
    folly::reserve_if_available(list, p->size());
    for (auto&& v : *p) {
      if (!ProtocolValueToThriftValue<Tag>{}(v, list.emplace_back())) {
        if constexpr (!has_structured_tag_v<Tag>) {
          return false;
        }
        ret = false;
      }
    }

    return ret;
  }
};

template <typename Tag>
struct ProtocolValueToThriftValue<type::set<Tag>> {
  template <typename SetType>
  bool operator()(const Value& value, SetType& set) const {
    type::native_type<Tag> elem;
    auto p = value.if_set();

    if (!p) {
      return false;
    }

    set.clear();
    folly::reserve_if_available(set, p->size());
    bool ret = true;
    for (auto&& v : *p) {
      apache::thrift::op::clear<Tag>(elem);
      if (!ProtocolValueToThriftValue<Tag>{}(v, elem)) {
        if constexpr (!has_structured_tag_v<Tag>) {
          return false;
        }
        ret = false;
      }
      set.emplace_hint(set.end(), std::move(elem));
    }

    return ret;
  }
};

template <typename KeyTag, typename ValueTag>
struct ProtocolValueToThriftValue<type::map<KeyTag, ValueTag>> {
  template <typename MapType>
  bool operator()(const Value& value, MapType& map) const {
    type::native_type<KeyTag> key;
    type::native_type<ValueTag> val;
    auto p = value.if_map();
    if (!p) {
      return false;
    }
    map.clear();
    folly::reserve_if_available(map, p->size());
    bool ret = true;
    for (auto&& [k, v] : *p) {
      apache::thrift::op::clear<KeyTag>(key);
      apache::thrift::op::clear<ValueTag>(val);
      if (!ProtocolValueToThriftValue<KeyTag>{}(k, key)) {
        if constexpr (!has_structured_tag_v<KeyTag>) {
          return false;
        }
        ret = false;
      }
      if (!ProtocolValueToThriftValue<ValueTag>{}(v, val)) {
        if constexpr (!has_structured_tag_v<ValueTag>) {
          return false;
        }
        ret = false;
      }
      map.emplace_hint(map.end(), std::move(key), std::move(val));
    }
    return ret;
  }
};

template <typename T, typename Tag>
struct ProtocolValueToThriftValue<type::cpp_type<T, Tag>>
    : ProtocolValueToThriftValue<Tag> {};

template <typename Adapter, typename Tag>
struct ProtocolValueToThriftValue<type::adapted<Adapter, Tag>> {
  template <typename ObjectOrValue, typename U>
  bool operator()(const ObjectOrValue& value, U& m) const {
    // TODO: Optimize in-place adapter
    type::native_type<Tag> orig;
    auto ret = ProtocolValueToThriftValue<Tag>{}(value, orig);
    m = Adapter::fromThrift(std::move(orig));
    return ret;
  }
};

template <typename Tag, typename Struct, int16_t FieldId>
struct ProtocolValueToThriftValue<
    type::field<Tag, FieldContext<Struct, FieldId>>> {
  template <typename ObjectOrValue, typename U>
  bool operator()(const ObjectOrValue& value, U& m, Struct&) const {
    return ProtocolValueToThriftValue<Tag>{}(value, m);
  }
};

template <typename Adapter, typename Tag, typename Struct, int16_t FieldId>
struct ProtocolValueToThriftValue<
    type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>> {
  using field_adapted_tag =
      type::field<type::adapted<Adapter, Tag>, FieldContext<Struct, FieldId>>;
  static_assert(type::is_concrete_v<field_adapted_tag>);

  template <typename ObjectOrValue, typename U, typename AdapterT = Adapter>
  constexpr adapt_detail::
      if_not_field_adapter<AdapterT, type::native_type<Tag>, Struct, bool>
      operator()(const ObjectOrValue& value, U& m, Struct&) const {
    return ProtocolValueToThriftValue<type::adapted<Adapter, Tag>>{}(value, m);
  }

  template <typename ObjectOrValue, typename U, typename AdapterT = Adapter>
  constexpr adapt_detail::
      if_field_adapter<AdapterT, FieldId, type::native_type<Tag>, Struct, bool>
      operator()(const ObjectOrValue& value, U& m, Struct& strct) const {
    // TODO: Optimize in-place adapter
    type::native_type<Tag> orig;
    bool ret = ProtocolValueToThriftValue<Tag>{}(value, orig);
    m = adapt_detail::fromThriftField<Adapter, FieldId>(std::move(orig), strct);
    return ret;
  }
};

template <class T>
struct ProtocolValueToThriftValueStructure {
  // Returns true if all fields are successfully converted.
  bool operator()(const Object& obj, T& s) const {
    bool ret = true;
    for (auto&& kv : obj) {
      op::invoke_by_field_id<T>(
          static_cast<FieldId>(kv.first),
          [&](auto id) {
            using Id = decltype(id);
            using FieldTag = op::get_field_tag<T, Id>;
            using FieldType = op::get_native_type<T, Id>;
            FieldType t;
            if (!ProtocolValueToThriftValue<FieldTag>{}(kv.second, t, s)) {
              ret = false;
              if constexpr (!has_structured_tag_v<FieldTag>) {
                return;
              }
            }

            using Ref = op::get_field_ref<T, Id>;
            if constexpr (apache::thrift::detail::is_shared_or_unique_ptr_v<
                              Ref>) {
              op::get<Id>(s) =
                  std::make_unique<op::get_native_type<T, Id>>(std::move(t));
            } else {
              op::get<Id>(s) = std::move(t);
            }
          },
          [&] {
            // Missing field in T.
            ret = false;
          });
    }
    return ret;
  }
  bool operator()(const Value& value, T& s) const {
    if (auto p = value.if_object()) {
      return operator()(*p, s);
    }
    return false;
  }
};

template <typename T>
struct ProtocolValueToThriftValue<type::struct_t<T>>
    : ProtocolValueToThriftValueStructure<T> {};
template <typename T>
struct ProtocolValueToThriftValue<type::union_t<T>>
    : ProtocolValueToThriftValueStructure<T> {};
template <typename T>
struct ProtocolValueToThriftValue<type::exception_t<T>>
    : ProtocolValueToThriftValueStructure<T> {};

} // namespace apache::thrift::protocol::detail
