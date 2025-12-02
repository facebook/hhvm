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

#include <memory>
#include <thrift/lib/cpp2/protocol/TableBasedSerializer.h>

#include <glog/logging.h>
#include <folly/CppAttributes.h>
#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/protocol/ProtocolReaderStructReadState.h>
#include <thrift/lib/cpp2/protocol/ProtocolReaderWireTypeInfo.h>

namespace apache::thrift::detail {

constexpr TypeInfo kStopType = {
    protocol::TType::T_STOP, nullptr, nullptr, nullptr};
constexpr FieldInfo kStopMarker = {
    0, FieldQualifier::Unqualified, nullptr, 0, 0, &kStopType};

template <class TProtocol>
void skip(
    TProtocol* iprot, ProtocolReaderStructReadState<TProtocol>& readState) {
  readState.skip(iprot);
  readState.readFieldEnd(iprot);
  readState.readFieldBeginNoInline(iprot);
}

void* getFieldValuesBasePtr(const StructInfo& structInfo, void* targetObject);

const void* getFieldValuesBasePtr(
    const StructInfo& structInfo, const void* targetObject);

/**
 * Returns a pointer to the memory holding the value of the field corresponding
 * to `fieldInfo`, in the given target `object`.
 */
inline const void* getFieldValuePtr(
    const FieldInfo& fieldInfo, const void* fieldValuesBasePtr) {
  return static_cast<const char*>(fieldValuesBasePtr) + fieldInfo.memberOffset;
}

/**
 * Returns a pointer to the memory holding the value of the field corresponding
 * to `fieldInfo`, in the given target `object`.
 */
inline void* getFieldValuePtr(
    const FieldInfo& fieldInfo, void* fieldValuesBasePtr) {
  return static_cast<char*>(fieldValuesBasePtr) + fieldInfo.memberOffset;
}

inline OptionalThriftValue getValue(
    const TypeInfo& typeInfo, const void* valuePtr) {
  if (typeInfo.get != nullptr) {
    return typeInfo.get(valuePtr, typeInfo);
  }
  return valuePtr != nullptr ? folly::make_optional<ThriftValue>(valuePtr)
                             : folly::none;
}

FOLLY_ERASE void* invokeSet(VoidPtrFuncPtr set, void* outValuePtr) {
  return reinterpret_cast<void* (*)(void*)>(set)(outValuePtr);
}

FOLLY_ERASE void* invokeStructSet(const TypeInfo& info, void* outValuePtr) {
  return reinterpret_cast<void* (*)(void*, const TypeInfo&)>(info.set)(
      outValuePtr, info);
}

template <class TProtocol>
const FieldInfo* findFieldInfo(
    TProtocol* iprot,
    ProtocolReaderStructReadState<TProtocol>& readState,
    const StructInfo& structInfo) {
  const FieldInfo* const end = structInfo.fieldInfos + structInfo.numFields;
  if (iprot->kUsesFieldNames()) {
    const FieldInfo* found =
        std::find_if(structInfo.fieldInfos, end, [&](const FieldInfo& val) {
          return val.name == readState.fieldName();
        });
    if (found != end) {
      readState.fieldId = found->id;
      readState.fieldType = found->typeInfo->type;
      if (readState.isCompatibleWithType(iprot, found->typeInfo->type)) {
        return found;
      }
    }
  } else {
    const FieldInfo* found = std::lower_bound(
        structInfo.fieldInfos,
        end,
        readState.fieldId,
        [](const FieldInfo& lhs, FieldID rhs) { return lhs.id < rhs; });
    if (found != end && found->id == readState.fieldId &&
        readState.isCompatibleWithType(iprot, found->typeInfo->type)) {
      return found;
    }
  }
  return nullptr;
}

// Returns active field id for a Thrift union object.
int getActiveId(const void* unionObject, const StructInfo& info);

// Sets the active field id for a Thrift union object.
void setActiveId(void* object, const StructInfo& info, int value);

/**
 * Checks if a field value is safe to retrieve.
 *
 * Optional fields are nullable, so it is only safe to retrieve their value if
 * they are explicitly set.
 *
 * All other fields (unqualified, terse) are always safe to retrieve.
 *
 * This should not be called for unions (i.e., `structInfo.unionExt` should be
 * `nullptr`)
 */
bool structFieldHasValue(
    const void* targetObject,
    const FieldInfo& fieldInfo,
    const StructInfo& structInfo);

// A helper function to set a field to its intrinsic default value.
void setToIntrinsicDefault(void* value, const FieldInfo& info);

void clearTerseField(void* value, const FieldInfo& info);

// For an unqualified and optional field, the semantic is identical to
// `structFieldHasValue`. An unqualified field is not emptiable, and an optional
// field is empty if it is not explicitly set. For a terse field, compare the
// value with the intrinsic default to check whether a field is empty or not.
bool isFieldNotEmpty(
    const void* object,
    const ThriftValue& value,
    const FieldInfo& info,
    const StructInfo& structInfo);

/**
 * A terse field skips serialization if it is equal to the intrinsic default.
 * Note, for a struct terse field, serialization is skipped if it is empty. If
 * it has an unqualified field, it is not eligible to be empty. A struct is
 * empty, if optional fields are not explicitly and terse fields are equal to
 * the intrinsic default.
 *
 * Cannot be called for Thrift unions.
 */
bool isTerseFieldSet(const ThriftValue& value, const FieldInfo& info);

/**
 * This should not be called for unions (i.e., `structInfo.unionExt` should be
 * `nullptr`)
 */
void markFieldAsSet(
    void* object, const FieldInfo& fieldInfo, const StructInfo& structInfo);

template <class TProtocol>
void readThriftValue(
    TProtocol* iprot,
    const TypeInfo& typeInfo,
    ProtocolReaderStructReadState<TProtocol>& readState,
    void* outValuePtr) {
  using WireTypeInfo = ProtocolReaderWireTypeInfo<TProtocol>;
  using WireType = typename WireTypeInfo::WireType;
  switch (typeInfo.type) {
    case protocol::TType::T_STRUCT:
      readState.beforeSubobject(iprot);
      read<TProtocol>(
          iprot,
          *static_cast<const StructInfo*>(typeInfo.typeExt),
          typeInfo.set ? invokeStructSet(typeInfo, outValuePtr) : outValuePtr);
      readState.afterSubobject(iprot);
      break;
    case protocol::TType::T_I64: {
      std::int64_t temp;
      iprot->readI64(temp);
      reinterpret_cast<void* (*)(void*, std::int64_t)>(typeInfo.set)(
          outValuePtr, temp);
      break;
    }
    case protocol::TType::T_I32: {
      std::int32_t temp;
      iprot->readI32(temp);
      reinterpret_cast<void* (*)(void*, std::int32_t)>(typeInfo.set)(
          outValuePtr, temp);
      break;
    }
    case protocol::TType::T_I16: {
      std::int16_t temp;
      iprot->readI16(temp);
      reinterpret_cast<void* (*)(void*, std::int16_t)>(typeInfo.set)(
          outValuePtr, temp);
      break;
    }
    case protocol::TType::T_BYTE: {
      std::int8_t temp;
      iprot->readByte(temp);
      reinterpret_cast<void* (*)(void*, std::int8_t)>(typeInfo.set)(
          outValuePtr, temp);
      break;
    }
    case protocol::TType::T_BOOL: {
      bool temp;
      iprot->readBool(temp);
      reinterpret_cast<void* (*)(void*, bool)>(typeInfo.set)(outValuePtr, temp);
      break;
    }
    case protocol::TType::T_DOUBLE: {
      double temp;
      iprot->readDouble(temp);
      reinterpret_cast<void* (*)(void*, double)>(typeInfo.set)(
          outValuePtr, temp);
      break;
    }
    case protocol::TType::T_FLOAT: {
      float temp;
      iprot->readFloat(temp);
      reinterpret_cast<void* (*)(void*, float)>(typeInfo.set)(
          outValuePtr, temp);
      break;
    }
    case protocol::TType::T_STRING: {
      switch (*static_cast<const StringFieldType*>(typeInfo.typeExt)) {
        case StringFieldType::String:
          iprot->readString(*static_cast<std::string*>(outValuePtr));
          break;
        case StringFieldType::StringView: {
          std::string temp;
          iprot->readString(temp);
          reinterpret_cast<void* (*)(void*, const std::string&)>(typeInfo.set)(
              outValuePtr, temp);
          break;
        }
        case StringFieldType::Binary:
          iprot->readBinary(*static_cast<std::string*>(outValuePtr));
          break;
        case StringFieldType::BinaryStringView: {
          std::string temp;
          iprot->readBinary(temp);
          reinterpret_cast<void* (*)(void*, const std::string&)>(typeInfo.set)(
              outValuePtr, temp);
          break;
        }
        case StringFieldType::IOBufObj: {
          const OptionalThriftValue value = getValue(typeInfo, outValuePtr);
          if (value.hasValue()) {
            iprot->readBinary(*value.value().iobuf);
          } else {
            folly::IOBuf temp;
            iprot->readBinary(temp);
            reinterpret_cast<void* (*)(void*, const folly::IOBuf&)>(
                typeInfo.set)(outValuePtr, temp);
          }
          break;
        }
        case StringFieldType::IOBuf:
          iprot->readBinary(*static_cast<folly::IOBuf*>(outValuePtr));
          break;
        case StringFieldType::IOBufPtr:
          iprot->readBinary(
              *static_cast<std::unique_ptr<folly::IOBuf>*>(outValuePtr));
          break;
      }
      break;
    }
    case protocol::TType::T_MAP: {
      readState.beforeSubobject(iprot);
      // Initialize the container to clear out current values.
      auto* actualObject = invokeSet(typeInfo.set, outValuePtr);
      const MapFieldExt& ext =
          *static_cast<const MapFieldExt*>(typeInfo.typeExt);
      std::uint32_t size = ~0;
      WireType reportedKeyType = WireTypeInfo::defaultValue();
      WireType reportedMappedType = WireTypeInfo::defaultValue();
      iprot->readMapBegin(reportedKeyType, reportedMappedType, size);
      struct Context {
        MapFieldTypeInfo typeInfo; // Must be first for safe casting
        TProtocol* iprot;
        ProtocolReaderStructReadState<TProtocol>* readState;
      };
      static_assert(
          std::is_standard_layout_v<Context>,
          "Context must be standard layout for safe casting through void*");
      const Context context = {
          {
              ext.keyInfo,
              ext.valInfo,
          },
          iprot,
          &readState,
      };
      const auto keyReader = [](const void* context, void* key) {
        const auto& typedContext = *static_cast<const Context*>(context);
        readThriftValue(
            typedContext.iprot,
            *typedContext.typeInfo.keyInfo,
            *typedContext.readState,
            key);
      };
      const auto valueReader = [](const void* context, void* val) {
        const auto& typedContext = *static_cast<const Context*>(context);
        readThriftValue(
            typedContext.iprot,
            *typedContext.typeInfo.valInfo,
            *typedContext.readState,
            val);
      };
      if (iprot->kOmitsContainerSizes()) {
        while (iprot->peekMap()) {
          ext.consumeElem(&context, actualObject, keyReader, valueReader);
        }
      } else {
        if (size > 0 &&
            (ext.keyInfo->type != reportedKeyType ||
             ext.valInfo->type != reportedMappedType)) {
          skip_n(*iprot, size, {reportedKeyType, reportedMappedType});
        } else {
          if (!canReadNElements(
                  *iprot, size, {reportedKeyType, reportedMappedType})) {
            protocol::TProtocolException::throwTruncatedData();
          }
          ext.readMap(&context, actualObject, size, keyReader, valueReader);
        }
      }
      iprot->readMapEnd();
      readState.afterSubobject(iprot);
      break;
    }
    case protocol::TType::T_SET: {
      readState.beforeSubobject(iprot);
      // Initialize the container to clear out current values.
      auto* actualObject = invokeSet(typeInfo.set, outValuePtr);
      const SetFieldExt& ext =
          *static_cast<const SetFieldExt*>(typeInfo.typeExt);
      std::uint32_t size = ~0;
      WireType reportedType = WireTypeInfo::defaultValue();
      iprot->readSetBegin(reportedType, size);
      struct Context {
        const TypeInfo* valInfo;
        TProtocol* iprot;
        ProtocolReaderStructReadState<TProtocol>& readState;
      };
      const Context context = {
          ext.valInfo,
          iprot,
          readState,
      };
      const auto reader = [](const void* context, void* value) {
        const auto& typedContext = *static_cast<const Context*>(context);
        readThriftValue(
            typedContext.iprot,
            *typedContext.valInfo,
            typedContext.readState,
            value);
      };
      if (iprot->kOmitsContainerSizes()) {
        while (iprot->peekSet()) {
          ext.consumeElem(&context, actualObject, reader);
        }
      } else {
        if (reportedType != ext.valInfo->type) {
          skip_n(*iprot, size, {reportedType});
        } else {
          if (!canReadNElements(*iprot, size, {reportedType})) {
            protocol::TProtocolException::throwTruncatedData();
          }
          ext.readSet(&context, actualObject, size, reader);
        }
      }
      iprot->readSetEnd();
      readState.afterSubobject(iprot);
      break;
    }
    case protocol::TType::T_LIST: {
      readState.beforeSubobject(iprot);
      // Initialize the container to clear out current values.
      auto* actualObject = invokeSet(typeInfo.set, outValuePtr);
      const ListFieldExt& ext =
          *static_cast<const ListFieldExt*>(typeInfo.typeExt);
      std::uint32_t size = ~0;
      WireType reportedType = WireTypeInfo::defaultValue();

      iprot->readListBegin(reportedType, size);
      struct Context {
        const TypeInfo* valInfo;
        TProtocol* iprot;
        ProtocolReaderStructReadState<TProtocol>& readState;
      };
      const Context context = {
          ext.valInfo,
          iprot,
          readState,
      };
      const auto reader = [](const void* context, void* value) {
        const auto& typedContext = *static_cast<const Context*>(context);
        readThriftValue(
            typedContext.iprot,
            *typedContext.valInfo,
            typedContext.readState,
            value);
      };
      if (iprot->kOmitsContainerSizes()) {
        while (iprot->peekList()) {
          ext.consumeElem(&context, actualObject, reader);
        }
      } else {
        if (reportedType != ext.valInfo->type) {
          skip_n(*iprot, size, {reportedType});
        } else {
          if (!canReadNElements(*iprot, size, {reportedType})) {
            protocol::TProtocolException::throwTruncatedData();
          }
          ext.readList(&context, actualObject, size, reader);
        }
      }
      iprot->readListEnd();
      readState.afterSubobject(iprot);
      break;
    }
    case protocol::TType::T_STOP:
    case protocol::TType::T_VOID:
    case protocol::TType::T_UTF8:
    case protocol::TType::T_U64:
    case protocol::TType::T_UTF16:
    case protocol::TType::T_STREAM:
      skip(iprot, readState);
  }
}

template <class TProtocol>
size_t writeThriftValue(
    TProtocol* iprot, const TypeInfo& typeInfo, ThriftValue value) {
  switch (typeInfo.type) {
    case protocol::TType::T_STRUCT:
      return write(
          iprot,
          *static_cast<const StructInfo*>(typeInfo.typeExt),
          value.object);
    case protocol::TType::T_I64:
      return iprot->writeI64(value.int64Value);
    case protocol::TType::T_I32:
      return iprot->writeI32(value.int32Value);
    case protocol::TType::T_I16:
      return iprot->writeI16(value.int16Value);
    case protocol::TType::T_BYTE:
      return iprot->writeByte(value.int8Value);
    case protocol::TType::T_BOOL:
      return iprot->writeBool(value.boolValue);
    case protocol::TType::T_DOUBLE:
      return iprot->writeDouble(value.doubleValue);
    case protocol::TType::T_FLOAT:
      return iprot->writeFloat(value.floatValue);
    case protocol::TType::T_STRING: {
      switch (*static_cast<const StringFieldType*>(typeInfo.typeExt)) {
        case StringFieldType::String:
          return iprot->writeString(
              *static_cast<const std::string*>(value.object));
        case StringFieldType::StringView:
          return iprot->writeString(
              static_cast<const folly::StringPiece>(value.stringViewValue));
        case StringFieldType::Binary:
          return iprot->writeBinary(
              *static_cast<const std::string*>(value.object));
        case StringFieldType::BinaryStringView:
          return iprot->writeBinary(
              static_cast<const folly::StringPiece>(value.stringViewValue));
        case StringFieldType::IOBufObj:
          return iprot->writeBinary(
              *static_cast<const folly::IOBuf*>(value.iobuf));
        case StringFieldType::IOBuf:
          return iprot->writeBinary(
              *static_cast<const folly::IOBuf*>(value.object));
        case StringFieldType::IOBufPtr:
          return iprot->writeBinary(
              *static_cast<const std::unique_ptr<folly::IOBuf>*>(value.object));
      }
    }
      // For container types, when recursively writing with lambdas we
      // intentionally skip checking OptionalThriftValue.hasValue and treat it
      // as a user error if the value is a nullptr.
    case protocol::TType::T_MAP: {
      const auto& ext = *static_cast<const MapFieldExt*>(typeInfo.typeExt);
      size_t written = iprot->writeMapBegin(
          ext.keyInfo->type, ext.valInfo->type, ext.size(value.object));

      struct Context {
        const TypeInfo* keyInfo;
        const TypeInfo* valInfo;
        TProtocol* iprot;
      };
      const Context context = {
          ext.keyInfo,
          ext.valInfo,
          iprot,
      };
      written += ext.writeMap(
          &context,
          value.object,
          iprot->kSortKeys(),
          [](const void* context, const void* key, const void* val) {
            const auto& typedContext = *static_cast<const Context*>(context);
            const TypeInfo& keyInfo = *typedContext.keyInfo;
            const TypeInfo& valInfo = *typedContext.valInfo;
            return writeThriftValue(
                       typedContext.iprot, keyInfo, *getValue(keyInfo, key)) +
                writeThriftValue(
                       typedContext.iprot,
                       *typedContext.valInfo,
                       *getValue(valInfo, val));
          });
      written += iprot->writeMapEnd();
      return written;
    }
    case protocol::TType::T_SET: {
      const auto& ext = *static_cast<const SetFieldExt*>(typeInfo.typeExt);
      size_t written =
          iprot->writeSetBegin(ext.valInfo->type, ext.size(value.object));

      struct Context {
        const TypeInfo* valInfo;
        TProtocol* iprot;
      };
      const Context context = {
          ext.valInfo,
          iprot,
      };
      written += ext.writeSet(
          &context,
          value.object,
          iprot->kSortKeys(),
          [](const void* context, const void* value) {
            const auto& typedContext = *static_cast<const Context*>(context);
            const TypeInfo& valInfo = *typedContext.valInfo;
            return writeThriftValue(
                typedContext.iprot, valInfo, *getValue(valInfo, value));
          });
      written += iprot->writeSetEnd();
      return written;
    }
    case protocol::TType::T_LIST: {
      const auto& ext = *static_cast<const ListFieldExt*>(typeInfo.typeExt);
      size_t written =
          iprot->writeListBegin(ext.valInfo->type, ext.size(value.object));

      struct Context {
        const TypeInfo* valInfo;
        TProtocol* iprot;
      };
      const Context context = {
          ext.valInfo,
          iprot,
      };
      written += ext.writeList(
          &context, value.object, [](const void* context, const void* value) {
            const auto& typedContext = *static_cast<const Context*>(context);
            const TypeInfo& valInfo = *typedContext.valInfo;
            return writeThriftValue(
                typedContext.iprot, valInfo, *getValue(valInfo, value));
          });
      written += iprot->writeListEnd();
      return written;
    }
    case protocol::TType::T_STOP:
    case protocol::TType::T_VOID:
    case protocol::TType::T_STREAM:
    case protocol::TType::T_UTF8:
    case protocol::TType::T_U64:
    case protocol::TType::T_UTF16:
      DCHECK(false);
      break;
  }
  return 0;
}

template <class TProtocol>
size_t writeField(
    TProtocol& iprot, const FieldInfo& fieldInfo, const ThriftValue& value) {
  size_t written = iprot.writeFieldBegin(
      fieldInfo.name, fieldInfo.typeInfo->type, fieldInfo.id);
  written += writeThriftValue(&iprot, *fieldInfo.typeInfo, value);
  written += iprot.writeFieldEnd();
  return written;
}

template <class TProtocol>
void readThriftUnion(
    TProtocol* iprot, const StructInfo& structInfo, void* unionObject) {
  DCHECK(structInfo.unionExt != nullptr);
  const UnionExt& unionExt = *structInfo.unionExt;
  ProtocolReaderStructReadState<TProtocol> readState;
  readState.readStructBegin(iprot);
  readState.fieldId = 0;
  readState.readFieldBegin(iprot);
  if (readState.atStop()) {
    unionExt.clear(unionObject);
    readState.readStructEnd(iprot);
    return;
  }

  void* const fieldValuesBasePtr =
      getFieldValuesBasePtr(structInfo, unionObject);
  if (const FieldInfo* fieldInfo =
          findFieldInfo(iprot, readState, structInfo)) {
    if (getActiveId(unionObject, structInfo) != 0) {
      unionExt.clear(unionObject);
    }
    void* fieldValuePtr = getFieldValuePtr(*fieldInfo, fieldValuesBasePtr);
    if (unionExt.initMember[0] != nullptr) {
      unionExt.initMember[fieldInfo - structInfo.fieldInfos](fieldValuePtr);
    }
    readThriftValue(iprot, *fieldInfo->typeInfo, readState, fieldValuePtr);
    setActiveId(unionObject, structInfo, fieldInfo->id);
    readState.readFieldEnd(iprot);
    readState.readFieldBegin(iprot);
  } else {
    skip(iprot, readState);
  }
  if (UNLIKELY(!readState.atStop())) {
    TProtocolException::throwUnionMissingStop();
  }
  readState.readStructEnd(iprot);
  return;
}

template <class TProtocol>
void read(TProtocol* iprot, const StructInfo& structInfo, void* targetObject) {
  DCHECK(targetObject);

  if (UNLIKELY(structInfo.unionExt != nullptr)) {
    readThriftUnion(iprot, structInfo, targetObject);
    return;
  }

  void* const fieldValuesBasePtr =
      getFieldValuesBasePtr(structInfo, targetObject);

  // Clear terse fields to intrinsic default values before deserialization.
  for (std::int16_t i = 0; i < structInfo.numFields; i++) {
    const auto& fieldInfo = structInfo.fieldInfos[i];
    clearTerseField(getFieldValuePtr(fieldInfo, fieldValuesBasePtr), fieldInfo);
  }

  // Define out of loop to call advanceToNextField after the loop ends.
  FieldID prevFieldId = 0;

  ProtocolReaderStructReadState<TProtocol> readState;
  readState.readStructBegin(iprot);

  // The index of the expected field in the struct layout.
  std::int16_t index = 0;

  // Every time advanceToNextField reports a field mismatch, either because the
  // field is missing or if the serialized fields are not sorted (protocols
  // don't guarantee a specific field order), we search for the field info
  // matching the read bytes. Then we resume from the one past the found field
  // to reduce the number of scans we have to do if the fields are sorted which
  // is a common case. When we increment index past the number of fields we
  // utilize the same search logic with a field info of type TType::T_STOP.
  for (;; ++index) {
    auto* fieldInfo = index < structInfo.numFields
        ? &structInfo.fieldInfos[index]
        : &kStopMarker;
    // Try to match the next field in order against the current bytes.
    if (UNLIKELY(!readState.advanceToNextField(
            iprot, prevFieldId, fieldInfo->id, fieldInfo->typeInfo->type))) {
      // Loop to skip until we find a match for both field id/name and type.
      for (;;) {
        readState.afterAdvanceFailure(iprot);
        if (readState.atStop()) {
          // Already at stop, return immediately.
          readState.readStructEnd(iprot);
          return;
        }
        fieldInfo = findFieldInfo(iprot, readState, structInfo);
        // Found it.
        if (fieldInfo) {
          // Set the index to the field next in order to the found field.
          index = fieldInfo - structInfo.fieldInfos;
          break;
        }
        skip(iprot, readState);
      }
    } else if (UNLIKELY(index >= structInfo.numFields)) {
      // We are at stop and have tried all of the fields, so return.
      readState.readStructEnd(iprot);
      return;
    }
    // Id and type are what we expect, try read.
    prevFieldId = fieldInfo->id;
    readThriftValue(
        iprot,
        *fieldInfo->typeInfo,
        readState,
        getFieldValuePtr(*fieldInfo, fieldValuesBasePtr));
    markFieldAsSet(targetObject, *fieldInfo, structInfo);
  }
}

template <class TProtocol>
size_t writeUnion(
    TProtocol& iprot, const StructInfo& structInfo, const void* unionObject) {
  size_t written = iprot.writeStructBegin(structInfo.name);

  const int activeFieldId = getActiveId(unionObject, structInfo);
  const FieldInfo* const fieldInfosEnd =
      structInfo.fieldInfos + structInfo.numFields;
  const FieldInfo* foundFieldInfo = std::lower_bound(
      structInfo.fieldInfos,
      fieldInfosEnd,
      activeFieldId,
      [](const FieldInfo& fieldInfo, FieldID fieldId) {
        return fieldInfo.id < fieldId;
      });

  if (foundFieldInfo != fieldInfosEnd && foundFieldInfo->id == activeFieldId) {
    const void* const fieldValuesBasePtr =
        getFieldValuesBasePtr(structInfo, unionObject);
    const void* const fieldValuePtr =
        getFieldValuePtr(*foundFieldInfo, fieldValuesBasePtr);
    const OptionalThriftValue value =
        getValue(*foundFieldInfo->typeInfo, fieldValuePtr);
    if (value.hasValue()) {
      written += writeField(iprot, *foundFieldInfo, value.value());
    } else if (foundFieldInfo->typeInfo->type == protocol::TType::T_STRUCT) {
      // DO_BEFORE(aristidis,20240730): Follow-up to figure out if this branch
      // is required. Document or remove.
      written += iprot.writeFieldBegin(
          foundFieldInfo->name,
          foundFieldInfo->typeInfo->type,
          foundFieldInfo->id);
      written += iprot.writeStructBegin(foundFieldInfo->name);
      written += iprot.writeFieldStop();
      written += iprot.writeStructEnd();
      written += iprot.writeFieldEnd();
    }
  }
  written += iprot.writeFieldStop();
  written += iprot.writeStructEnd();
  return written;
}

template <class TProtocol>
size_t write(
    TProtocol* iprot, const StructInfo& structInfo, const void* targetObject) {
  DCHECK(targetObject);
  if (UNLIKELY(structInfo.unionExt != nullptr)) {
    return writeUnion(*iprot, structInfo, targetObject);
  }

  const void* const fieldValuesBasePtr =
      getFieldValuesBasePtr(structInfo, targetObject);

  size_t written = iprot->writeStructBegin(structInfo.name);
  for (std::int16_t index = 0; index < structInfo.numFields; index++) {
    const FieldInfo& fieldInfo = structInfo.fieldInfos[index];
    if (!structFieldHasValue(targetObject, fieldInfo, structInfo)) {
      continue;
    }

    const void* fieldValuePtr = getFieldValuePtr(fieldInfo, fieldValuesBasePtr);
    OptionalThriftValue optionalValue =
        getValue(*fieldInfo.typeInfo, fieldValuePtr);
    if (!optionalValue) {
      // DO_BEFORE(aristidis,20240920): Check when this is valid. Document.
      continue;
    }

    if (fieldInfo.qualifier == FieldQualifier::Terse &&
        !isTerseFieldSet(optionalValue.value(), fieldInfo)) {
      continue;
    }

    written += writeField(*iprot, fieldInfo, optionalValue.value());
  }
  written += iprot->writeFieldStop();
  written += iprot->writeStructEnd();
  return written;
}

} // namespace apache::thrift::detail
