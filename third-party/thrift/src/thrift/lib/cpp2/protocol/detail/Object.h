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

#include <stack>
#include <type_traits>

#include <fatal/type/same_reference_as.h>
#include <folly/CPortability.h>
#include <folly/Utility.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/GetStandardProtocol.h>
#include <thrift/lib/cpp2/type/Any.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>
#include <thrift/lib/cpp2/type/Traits.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/thrift/gen-cpp2/id_types.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol {

// This is the return value of parseObject with mask.
// Masked fields are deserialized to included Object, and the other fields are
// are stored in excluded MaskedProtocolData.
struct MaskedDecodeResult {
  Object included;
  MaskedProtocolData excluded;
};

template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeValue(const Value& val);

namespace detail {

template <typename C, typename T>
decltype(auto) forward_elem(T& elem) {
  return std::forward<typename fatal::same_reference_as<T, C>::type>(elem);
}

template <typename TT, typename = void>
struct ValueHelper {
  template <typename T>
  static void set(Value& result, T&& value) {
    if constexpr (false) {
    } else if constexpr (type::base_type_v<TT> == type::BaseType::Bool) {
      result.boolValue_ref() = value;
    } else if constexpr (type::base_type_v<TT> == type::BaseType::Byte) {
      result.byteValue_ref() = value;
    } else if constexpr (type::base_type_v<TT> == type::BaseType::I16) {
      result.i16Value_ref() = value;
    } else if constexpr (type::base_type_v<TT> == type::BaseType::I32) {
      result.i32Value_ref() = value;
    } else if constexpr (type::base_type_v<TT> == type::BaseType::I64) {
      result.i64Value_ref() = value;
    } else if constexpr (type::base_type_v<TT> == type::BaseType::Enum) {
      result.i32Value_ref() = static_cast<int32_t>(value);
    } else if constexpr (type::base_type_v<TT> == type::BaseType::Float) {
      result.floatValue_ref() = value;
    } else if constexpr (type::base_type_v<TT> == type::BaseType::Double) {
      result.doubleValue_ref() = value;
    } else if constexpr (type::base_type_v<TT> == type::BaseType::String) {
      result.stringValue_ref() = std::forward<T>(value);
    }
  }
};

template <>
struct ValueHelper<type::binary_t> {
  static void set(Value& result, folly::IOBuf value) {
    result.binaryValue_ref() = std::move(value);
  }
  static void set(Value& result, std::string_view value) {
    result.binaryValue_ref() =
        folly::IOBuf{folly::IOBuf::COPY_BUFFER, value.data(), value.size()};
  }
  static void set(Value& result, folly::ByteRange value) {
    result.binaryValue_ref() =
        folly::IOBuf{folly::IOBuf::COPY_BUFFER, value.data(), value.size()};
  }
};

template <typename V>
struct ValueHelper<type::list<V>> {
  template <typename C>
  static void set(Value& result, C&& value) {
    auto& result_list = result.listValue_ref().ensure();
    for (auto& elem : value) {
      ValueHelper<V>::set(result_list.emplace_back(), forward_elem<C>(elem));
    }
  }
};

template <typename V>
struct ValueHelper<type::set<V>> {
  template <typename C>
  static void set(Value& result, C&& value) {
    auto& result_set = result.setValue_ref().ensure();
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
    auto& result_map = result.mapValue_ref().ensure();
    for (auto& entry : value) {
      Value key;
      ValueHelper<K>::set(key, entry.first);
      ValueHelper<V>::set(result_map[key], forward_elem<C>(entry.second));
    }
  }
};

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
    beginValue().objectValue_ref().ensure();
    return 0;
  }
  uint32_t writeStructEnd() { return endValue(Value::Type::objectValue); }

  uint32_t writeFieldBegin(
      const char* /*name*/, TType /*fieldType*/, int16_t fieldId) {
    auto result = cur(Value::Type::objectValue)
                      .mutable_objectValue()
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
    std::map<Value, Value>& mapVal = cur().mapValue_ref().ensure();
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
    std::set<Value>& setVal = cur().setValue_ref().ensure();
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
    assert(str != nullptr);
    if (!str) {
      return 0;
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
    std::vector<Value>& listVal = beginValue().listValue_ref().ensure();
    listVal.resize(n);
    for (auto itr = listVal.rbegin(); itr != listVal.rend(); ++itr) {
      cur_.push(&*itr);
    }
  }

  // Get temporary buffer from cur()
  std::vector<Value> getBufferFromStack() {
    return std::move(*cur(Value::Type::listValue).listValue_ref());
  }
};

// Specialization for all structured types.
template <typename TT>
struct ValueHelper<TT, type::if_structured<TT>> {
  template <typename T>
  static void set(Value& result, T&& value) {
    ObjectWriter writer(&result);
    op::detail::RecursiveEncode<type::infer_tag<folly::remove_cvref_t<T>>>{}(
        writer, value);
  }
};

// Schemaless deserialization of thrift serialized data of specified
// thrift type into conformance::Value
// Protocol: protocol to use eg. apache::thrift::BinaryProtocolReader
// TODO: handle jsonprotocol
template <class Protocol>
Value parseValue(Protocol& prot, TType arg_type, bool string_to_binary = true) {
  Value result;
  switch (arg_type) {
    case protocol::T_BOOL: {
      bool boolv;
      prot.readBool(boolv);
      result.boolValue_ref() = boolv;
      return result;
    }
    case protocol::T_BYTE: {
      int8_t bytev = 0;
      prot.readByte(bytev);
      result.byteValue_ref() = bytev;
      return result;
    }
    case protocol::T_I16: {
      int16_t i16;
      prot.readI16(i16);
      result.i16Value_ref() = i16;
      return result;
    }
    case protocol::T_I32: {
      int32_t i32;
      prot.readI32(i32);
      result.i32Value_ref() = i32;
      return result;
    }
    case protocol::T_I64: {
      int64_t i64;
      prot.readI64(i64);
      result.i64Value_ref() = i64;
      return result;
    }
    case protocol::T_DOUBLE: {
      double dub;
      prot.readDouble(dub);
      result.doubleValue_ref() = dub;
      return result;
    }
    case protocol::T_FLOAT: {
      float flt;
      prot.readFloat(flt);
      result.floatValue_ref() = flt;
      return result;
    }
    case protocol::T_STRING: {
      if (string_to_binary) {
        auto& binaryValue = result.binaryValue_ref().ensure();
        prot.readBinary(binaryValue);
        return result;
      }
      std::string str;
      prot.readString(str);
      result.stringValue_ref() = str;
      return result;
    }
    case protocol::T_STRUCT: {
      std::string name;
      int16_t fid;
      TType ftype;
      auto& objectValue = result.objectValue_ref().ensure();
      prot.readStructBegin(name);
      while (true) {
        prot.readFieldBegin(name, ftype, fid);
        if (ftype == protocol::T_STOP) {
          break;
        }
        objectValue[FieldId{fid}] = parseValue(prot, ftype, string_to_binary);
        prot.readFieldEnd();
      }
      prot.readStructEnd();
      return result;
    }
    case protocol::T_MAP: {
      TType keyType;
      TType valType;
      uint32_t size;
      auto& mapValue = result.mapValue_ref().ensure();
      prot.readMapBegin(keyType, valType, size);
      for (uint32_t i = 0; i < size; i++) {
        auto key = parseValue(prot, keyType, string_to_binary);
        mapValue[std::move(key)] = parseValue(prot, valType, string_to_binary);
      }
      prot.readMapEnd();
      return result;
    }
    case protocol::T_SET: {
      TType elemType;
      uint32_t size;
      auto& setValue = result.setValue_ref().ensure();
      prot.readSetBegin(elemType, size);
      for (uint32_t i = 0; i < size; i++) {
        setValue.insert(parseValue(prot, elemType, string_to_binary));
      }
      prot.readSetEnd();
      return result;
    }
    case protocol::T_LIST: {
      TType elemType;
      uint32_t size;
      prot.readListBegin(elemType, size);
      auto& listValue = result.listValue_ref().ensure();
      listValue.reserve(size);
      for (uint32_t i = 0; i < size; i++) {
        listValue.push_back(parseValue(prot, elemType, string_to_binary));
      }
      prot.readListEnd();
      return result;
    }
    default: {
      TProtocolException::throwInvalidSkipType(arg_type);
    }
  }
}

struct MaskedDecodeResultValue {
  Value included;
  MaskedData excluded;
};

// Returns an element in the list by ValueId.
template <typename T>
const T& getByValueId(const std::vector<T>& values, type::ValueId id) {
  return values[apache::thrift::util::zigzagToI64(static_cast<int64_t>(id))];
}

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
  maskedData.full_ref() =
      type::ValueId{apache::thrift::util::i32ToZigzag(values.size() - 1)};
}

// parseValue with readMask and writeMask
template <bool KeepExcludedData, typename Protocol>
MaskedDecodeResultValue parseValueWithMask(
    Protocol& prot,
    TType arg_type,
    MaskRef readMask,
    MaskRef writeMask,
    MaskedProtocolData& protocolData,
    bool string_to_binary = true) {
  MaskedDecodeResultValue result;
  if (readMask.isAllMask()) { // serialize all
    result.included = parseValue(prot, arg_type, string_to_binary);
    return result;
  }
  if (readMask.isNoneMask()) { // do not deserialize
    if (!KeepExcludedData) { // no need to store
      apache::thrift::skip(prot, arg_type);
      return result;
    }
    if (writeMask.isNoneMask()) { // store the serialized data
      setMaskedDataFull(prot, arg_type, result.excluded, protocolData);
      return result;
    }
    if (writeMask.isAllMask()) { // no need to store
      apache::thrift::skip(prot, arg_type);
      return result;
    }
    // Need to recursively store the result not in writeMask.
  }
  switch (arg_type) {
    case protocol::T_STRUCT: {
      auto& object = result.included.objectValue_ref().ensure();
      std::string name;
      int16_t fid;
      TType ftype;
      prot.readStructBegin(name);
      while (true) {
        prot.readFieldBegin(name, ftype, fid);
        if (ftype == protocol::T_STOP) {
          break;
        }
        MaskRef nextRead = readMask.get(FieldId{fid});
        MaskRef nextWrite = writeMask.get(FieldId{fid});
        MaskedDecodeResultValue nestedResult =
            parseValueWithMask<KeepExcludedData>(
                prot,
                ftype,
                nextRead,
                nextWrite,
                protocolData,
                string_to_binary);
        // Set nested MaskedDecodeResult if not empty.
        if (!apache::thrift::empty(nestedResult.included)) {
          object[FieldId{fid}] = std::move(nestedResult.included);
        }
        if (KeepExcludedData && !apache::thrift::empty(nestedResult.excluded)) {
          result.excluded.fields_ref().ensure()[FieldId{fid}] =
              std::move(nestedResult.excluded);
        }
        prot.readFieldEnd();
      }
      prot.readStructEnd();
      return result;
    }
    case protocol::T_MAP: {
      auto& map = result.included.mapValue_ref().ensure();
      TType keyType;
      TType valType;
      uint32_t size;
      prot.readMapBegin(keyType, valType, size);
      for (uint32_t i = 0; i < size; i++) {
        auto keyValue = parseValue(prot, keyType, string_to_binary);
        MaskRef nextRead =
            readMask.get(findMapIdByValueAddress(readMask.mask, keyValue));
        MaskRef nextWrite =
            writeMask.get(findMapIdByValueAddress(writeMask.mask, keyValue));
        MaskedDecodeResultValue nestedResult =
            parseValueWithMask<KeepExcludedData>(
                prot,
                valType,
                nextRead,
                nextWrite,
                protocolData,
                string_to_binary);
        // Set nested MaskedDecodeResult if not empty.
        if (!apache::thrift::empty(nestedResult.included)) {
          map[keyValue] = std::move(nestedResult.included);
        }
        if (KeepExcludedData && !apache::thrift::empty(nestedResult.excluded)) {
          auto& keys = protocolData.keys().ensure();
          keys.push_back(keyValue);
          type::ValueId id =
              type::ValueId{apache::thrift::util::i32ToZigzag(keys.size() - 1)};
          result.excluded.values_ref().ensure()[id] =
              std::move(nestedResult.excluded);
        }
      }
      prot.readMapEnd();
      return result;
    }
    default: {
      result.included = parseValue(prot, arg_type, string_to_binary);
      return result;
    }
  }
}

template <typename Protocol, bool KeepExcludedData>
MaskedDecodeResult parseObject(
    const folly::IOBuf& buf,
    Mask readMask,
    Mask writeMask,
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
  result.included =
      std::move(parseValueResult.included.objectValue_ref().ensure());
  return result;
}

inline TType getTType(const Value& val) {
  auto type = toTType(static_cast<type::BaseType>(val.getType()));
  if (type == protocol::T_UTF7 || type == protocol::T_UTF8 ||
      type == protocol::T_UTF16) {
    return protocol::T_STRING;
  }
  return type;
}

inline void ensureSameType(const Value& a, TType b) {
  if (getTType(a) != b) {
    TProtocolException::throwInvalidFieldData();
  }
}

template <class Protocol>
void serializeValue(Protocol& prot, const Value& value) {
  switch (value.getType()) {
    case Value::Type::boolValue:
      prot.writeBool(*value.boolValue_ref());
      return;
    case Value::Type::byteValue:
      prot.writeByte(*value.byteValue_ref());
      return;
    case Value::Type::i16Value:
      prot.writeI16(*value.i16Value_ref());
      return;
    case Value::Type::i32Value:
      prot.writeI32(*value.i32Value_ref());
      return;
    case Value::Type::i64Value:
      prot.writeI64(*value.i64Value_ref());
      return;
    case Value::Type::floatValue:
      prot.writeFloat(*value.floatValue_ref());
      return;
    case Value::Type::doubleValue:
      prot.writeDouble(*value.doubleValue_ref());
      return;
    case Value::Type::stringValue:
      prot.writeString(*value.stringValue_ref());
      return;
    case Value::Type::binaryValue:
      prot.writeBinary(*value.binaryValue_ref());
      return;
    case Value::Type::listValue: {
      TType elemType = protocol::T_I64;
      uint32_t size = value.listValue_ref()->size();
      if (size > 0) {
        elemType = getTType(value.listValue_ref()->at(0));
      }
      prot.writeListBegin(elemType, size);
      for (const auto& val : *value.listValue_ref()) {
        ensureSameType(val, elemType);
        serializeValue(prot, val);
      }
      prot.writeListEnd();
      return;
    }
    case Value::Type::mapValue: {
      TType keyType = protocol::T_STRING;
      TType valueType = protocol::T_I64;
      uint32_t size = value.mapValue_ref()->size();
      if (size > 0) {
        keyType = getTType(value.mapValue_ref()->begin()->first);
        valueType = getTType(value.mapValue_ref()->begin()->second);
      }
      prot.writeMapBegin(keyType, valueType, size);
      for (const auto& [key, val] : *value.mapValue_ref()) {
        ensureSameType(key, keyType);
        ensureSameType(val, valueType);
        serializeValue(prot, key);
        serializeValue(prot, val);
      }
      prot.writeMapEnd();
      return;
    }
    case Value::Type::setValue: {
      TType elemType = protocol::T_I64;
      uint32_t size = value.setValue_ref()->size();
      if (size > 0) {
        elemType = getTType(*value.setValue_ref()->begin());
      }
      prot.writeSetBegin(elemType, size);
      for (const auto& val : *value.setValue_ref()) {
        ensureSameType(val, elemType);
        serializeValue(prot, val);
      }
      prot.writeSetEnd();
      return;
    }
    case Value::Type::objectValue: {
      serializeObject(prot, *value.objectValue_ref());
      return;
    }
    default: {
      TProtocolException::throwInvalidFieldData();
    }
  }
}

template <class Protocol>
void serializeObject(Protocol& prot, const Object& obj) {
  prot.writeStructBegin("");
  for (const auto& [fieldID, fieldVal] : *obj.members()) {
    auto fieldType = getTType(fieldVal);
    prot.writeFieldBegin("", fieldType, fieldID);
    serializeValue(prot, fieldVal);
    prot.writeFieldEnd();
  }
  prot.writeFieldStop();
  prot.writeStructEnd();
}

// Writes the field from raw data in MaskedData.
template <class Protocol>
void writeRawField(
    Protocol& prot,
    FieldId fieldId,
    MaskedProtocolData& protocolData,
    MaskedData& maskedData) {
  auto& nestedMaskedData = maskedData.fields_ref().value()[fieldId];
  // When value doesn't exist in the object, maskedData should have full field.
  if (!nestedMaskedData.full_ref()) {
    throw std::runtime_error("incompatible value and maskedData");
  }
  type::ValueId valueId = nestedMaskedData.full_ref().value();
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
    MaskedProtocolData& protocolData,
    MaskedData& maskedData) {
  // When value doesn't exist in the object, maskedData should have full field.
  if (!maskedData.full_ref()) {
    throw std::runtime_error("incompatible value and maskedData");
  }
  type::ValueId valueId = maskedData.full_ref().value();
  const EncodedValue& value = getByValueId(*protocolData.values(), valueId);
  if (toTType(*value.wireType()) != valueType) {
    TProtocolException::throwInvalidFieldData();
  }
  prot.writeRaw(*value.data());
}

// We can assume that if value type is a struct, fields in maskedData is
// active, and if value type is a map, values in maskedData is active.
// It throws a runtime error if value and maskedData are incompatible.
template <class Protocol>
void serializeValue(
    Protocol& prot,
    const Value& value,
    MaskedProtocolData& protocolData,
    MaskedData& maskedData) {
  switch (value.getType()) {
    case Value::Type::objectValue: {
      if (!maskedData.fields_ref()) {
        throw std::runtime_error("incompatible value and maskedData");
      }
      prot.writeStructBegin("");
      // It is more efficient to serialize with sorted field ids.
      std::set<FieldId> fieldIds{};
      for (const auto& [fieldId, _] : value.as_object()) {
        fieldIds.insert(FieldId{fieldId});
      }
      for (const auto& [fieldId, _] : *maskedData.fields_ref()) {
        fieldIds.insert(fieldId);
      }

      for (auto fieldId : fieldIds) {
        if (!value.as_object().contains(fieldId)) {
          // no need to serialize the value
          writeRawField(prot, fieldId, protocolData, maskedData);
          continue;
        }
        // get type from value
        const auto& fieldVal = value.as_object().at(fieldId);
        auto fieldType = getTType(fieldVal);
        prot.writeFieldBegin("", fieldType, folly::to_underlying(fieldId));
        // just serialize the value
        if (folly::get_ptr(*maskedData.fields_ref(), fieldId) == nullptr) {
          serializeValue(prot, fieldVal);
        } else { // recursively serialize value with maskedData
          auto& nextMaskedData = maskedData.fields_ref().value()[fieldId];
          serializeValue(prot, fieldVal, protocolData, nextMaskedData);
        }
        prot.writeFieldEnd();
      }
      prot.writeFieldStop();
      prot.writeStructEnd();
      return;
    }
    case Value::Type::mapValue: {
      if (!maskedData.values_ref()) {
        throw std::runtime_error("incompatible value and maskedData");
      }
      TType keyType = protocol::T_STRING;
      TType valueType = protocol::T_I64;

      // compute size, keyType, and valueType
      uint32_t size = value.mapValue_ref()->size();
      if (size > 0) {
        keyType = getTType(value.mapValue_ref()->begin()->first);
        valueType = getTType(value.mapValue_ref()->begin()->second);
      }
      for (auto& [keyValueId, nestedMaskedData] : *maskedData.values_ref()) {
        const Value& key = getByValueId(*protocolData.keys(), keyValueId);
        if (size == 0) { // need to set keyType and valueType
          keyType = getTType(key);
          type::ValueId valueId = nestedMaskedData.full_ref().value();
          valueType = toTType(
              *getByValueId(*protocolData.values(), valueId).wireType());
        }
        if (folly::get_ptr(*value.mapValue_ref(), key) == nullptr) {
          ++size;
        }
      }

      // Remember which keys are in the maskedData. Cannot use unordered_map
      // as Value doesn't have a hash function.
      std::set<Value> keys{};
      prot.writeMapBegin(keyType, valueType, size);
      for (auto& [keyValueId, nestedMaskedData] : *maskedData.values_ref()) {
        const Value& key = getByValueId(*protocolData.keys(), keyValueId);
        keys.insert(key);
        ensureSameType(key, keyType);
        serializeValue(prot, key);
        // no need to serialize the value
        if (folly::get_ptr(*value.mapValue_ref(), key) == nullptr) {
          writeRawMapValue(prot, valueType, protocolData, nestedMaskedData);
          continue;
        }
        // recursively serialize value with maskedData
        const Value& val = value.mapValue_ref()->at(key);
        ensureSameType(val, valueType);
        serializeValue(prot, val, protocolData, nestedMaskedData);
      }
      for (const auto& [key, val] : *value.mapValue_ref()) {
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
    default: {
      serializeValue(prot, value);
    }
  }
}

type::Type toType(const protocol::Value& value);

template <class ProtocolWriter>
type::AnyData toAny(
    const Value& value,
    type::Protocol protocol = get_standard_protocol<ProtocolWriter>) {
  type::SemiAny data;
  data.type() = toType(value);
  data.protocol() = protocol;
  data.data() = std::move(
      *::apache::thrift::protocol::serializeValue<ProtocolWriter>(value));
  return type::AnyData{data};
}

} // namespace detail
} // namespace apache::thrift::protocol
