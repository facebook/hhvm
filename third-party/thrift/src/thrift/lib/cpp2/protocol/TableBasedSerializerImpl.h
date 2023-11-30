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

namespace apache {
namespace thrift {
namespace detail {

constexpr TypeInfo kStopType = {
    protocol::TType::T_STOP, nullptr, nullptr, nullptr};
constexpr FieldInfo kStopMarker = {
    0, FieldQualifier::Unqualified, nullptr, 0, 0, &kStopType};

template <class Protocol_>
void skip(
    Protocol_* iprot, ProtocolReaderStructReadState<Protocol_>& readState) {
  readState.skip(iprot);
  readState.readFieldEnd(iprot);
  readState.readFieldBeginNoInline(iprot);
}

inline const void* getMember(const FieldInfo& fieldInfo, const void* object) {
  return static_cast<const char*>(object) + fieldInfo.memberOffset;
}

inline void* getMember(const FieldInfo& fieldInfo, void* object) {
  return static_cast<char*>(object) + fieldInfo.memberOffset;
}

inline OptionalThriftValue getValue(
    const TypeInfo& typeInfo, const void* object) {
  if (typeInfo.get) {
    return typeInfo.get(object, typeInfo);
  }
  return object ? folly::make_optional<ThriftValue>(object) : folly::none;
}

FOLLY_ERASE void* invokeSet(VoidFuncPtr set, void* object) {
  return reinterpret_cast<void* (*)(void*)>(set)(object);
}

FOLLY_ERASE void* invokeStructSet(const TypeInfo& info, void* object) {
  return reinterpret_cast<void* (*)(void*, const TypeInfo&)>(info.set)(
      object, info);
}

template <class Protocol_>
const FieldInfo* findFieldInfo(
    Protocol_* iprot,
    ProtocolReaderStructReadState<Protocol_>& readState,
    const StructInfo& structInfo) {
  auto* end = structInfo.fieldInfos + structInfo.numFields;
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
int getActiveId(const void* object, const StructInfo& info);

// Sets the active field id for a Thrift union object.
void setActiveId(void* object, const StructInfo& info, int value);

// Checks whether if a field value is safe to retrieve. For an optional field,
// a field is nullable, so it is safe to get the field value if it is
// explicitly set. An unqualified and terse fields are always safe to retrive
// their values.
bool hasFieldValue(
    const void* object, const FieldInfo& info, const StructInfo& structInfo);

// A helper function to set a field to its intrinsic default value.
void setToIntrinsicDefault(void* value, const FieldInfo& info);

void clearTerseField(void* value, const FieldInfo& info);

// For an unqualified and optional field, the semantic is identical to
// `hasFieldValue`. An unqualified field is not emptiable, and an optional field
// is empty if it is not explicitly set. For a terse field, compare the value
// with the intrinsic default to check whether a field is empty or not.
bool isFieldNotEmpty(
    const void* object,
    const ThriftValue& value,
    const FieldInfo& info,
    const StructInfo& structInfo);

// A terse field skips serialization if it is equal to the intrinsic default.
// Note, for a struct terse field, serialization is skipped if it is empty. If
// it has an unqualified field, it is not eligible to be empty. A struct is
// empty, if optional fields are not explicitly and terse fields are equal to
// the intrinsic default.
bool isTerseFieldSet(const ThriftValue& value, const FieldInfo& info);

void markFieldAsSet(
    void* object, const FieldInfo& info, const StructInfo& structInfo);

template <class Protocol_>
void read(
    Protocol_* iprot,
    const TypeInfo& typeInfo,
    ProtocolReaderStructReadState<Protocol_>& readState,
    void* object) {
  using WireTypeInfo = ProtocolReaderWireTypeInfo<Protocol_>;
  using WireType = typename WireTypeInfo::WireType;
  switch (typeInfo.type) {
    case protocol::TType::T_STRUCT:
      readState.beforeSubobject(iprot);
      read<Protocol_>(
          iprot,
          *static_cast<const StructInfo*>(typeInfo.typeExt),
          typeInfo.set ? invokeStructSet(typeInfo, object) : object);
      readState.afterSubobject(iprot);
      break;
    case protocol::TType::T_I64: {
      std::int64_t temp;
      iprot->readI64(temp);
      reinterpret_cast<void (*)(void*, std::int64_t)>(typeInfo.set)(
          object, temp);
      break;
    }
    case protocol::TType::T_I32: {
      std::int32_t temp;
      iprot->readI32(temp);
      reinterpret_cast<void (*)(void*, std::int32_t)>(typeInfo.set)(
          object, temp);
      break;
    }
    case protocol::TType::T_I16: {
      std::int16_t temp;
      iprot->readI16(temp);
      reinterpret_cast<void (*)(void*, std::int16_t)>(typeInfo.set)(
          object, temp);
      break;
    }
    case protocol::TType::T_BYTE: {
      std::int8_t temp;
      iprot->readByte(temp);
      reinterpret_cast<void (*)(void*, std::int8_t)>(typeInfo.set)(
          object, temp);
      break;
    }
    case protocol::TType::T_BOOL: {
      bool temp;
      iprot->readBool(temp);
      reinterpret_cast<void (*)(void*, bool)>(typeInfo.set)(object, temp);
      break;
    }
    case protocol::TType::T_DOUBLE: {
      double temp;
      iprot->readDouble(temp);
      reinterpret_cast<void (*)(void*, double)>(typeInfo.set)(object, temp);
      break;
    }
    case protocol::TType::T_FLOAT: {
      float temp;
      iprot->readFloat(temp);
      reinterpret_cast<void (*)(void*, float)>(typeInfo.set)(object, temp);
      break;
    }
    case protocol::TType::T_STRING: {
      switch (*static_cast<const StringFieldType*>(typeInfo.typeExt)) {
        case StringFieldType::String:
          iprot->readString(*static_cast<std::string*>(object));
          break;
        case StringFieldType::StringView: {
          std::string temp;
          iprot->readString(temp);
          reinterpret_cast<void (*)(void*, const std::string&)>(typeInfo.set)(
              object, temp);
          break;
        }
        case StringFieldType::Binary:
          iprot->readBinary(*static_cast<std::string*>(object));
          break;
        case StringFieldType::BinaryStringView: {
          std::string temp;
          iprot->readBinary(temp);
          reinterpret_cast<void (*)(void*, const std::string&)>(typeInfo.set)(
              object, temp);
          break;
        }
        case StringFieldType::IOBufObj: {
          const OptionalThriftValue value = getValue(typeInfo, object);
          if (value.hasValue()) {
            iprot->readBinary(*value.value().iobuf);
          } else {
            folly::IOBuf temp;
            iprot->readBinary(temp);
            reinterpret_cast<void (*)(void*, const folly::IOBuf&)>(
                typeInfo.set)(object, temp);
          }
          break;
        }
        case StringFieldType::IOBuf:
          iprot->readBinary(*static_cast<folly::IOBuf*>(object));
          break;
        case StringFieldType::IOBufPtr:
          iprot->readBinary(
              *static_cast<std::unique_ptr<folly::IOBuf>*>(object));
          break;
      }
      break;
    }
    case protocol::TType::T_MAP: {
      readState.beforeSubobject(iprot);
      // Initialize the container to clear out current values.
      auto* actualObject = invokeSet(typeInfo.set, object);
      const MapFieldExt& ext =
          *static_cast<const MapFieldExt*>(typeInfo.typeExt);
      std::uint32_t size = ~0;
      WireType reportedKeyType = WireTypeInfo::defaultValue();
      WireType reportedMappedType = WireTypeInfo::defaultValue();
      iprot->readMapBegin(reportedKeyType, reportedMappedType, size);
      struct Context {
        const TypeInfo* keyInfo;
        const TypeInfo* valInfo;
        Protocol_* iprot;
        ProtocolReaderStructReadState<Protocol_>& readState;
      };
      const Context context = {
          ext.keyInfo,
          ext.valInfo,
          iprot,
          readState,
      };
      const auto keyReader = [](const void* context, void* key) {
        const auto& typedContext = *static_cast<const Context*>(context);
        read(
            typedContext.iprot,
            *typedContext.keyInfo,
            typedContext.readState,
            key);
      };
      const auto valueReader = [](const void* context, void* val) {
        const auto& typedContext = *static_cast<const Context*>(context);
        read(
            typedContext.iprot,
            *typedContext.valInfo,
            typedContext.readState,
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
      auto* actualObject = invokeSet(typeInfo.set, object);
      const SetFieldExt& ext =
          *static_cast<const SetFieldExt*>(typeInfo.typeExt);
      std::uint32_t size = ~0;
      WireType reportedType = WireTypeInfo::defaultValue();
      iprot->readSetBegin(reportedType, size);
      struct Context {
        const TypeInfo* valInfo;
        Protocol_* iprot;
        ProtocolReaderStructReadState<Protocol_>& readState;
      };
      const Context context = {
          ext.valInfo,
          iprot,
          readState,
      };
      const auto reader = [](const void* context, void* value) {
        const auto& typedContext = *static_cast<const Context*>(context);
        read(
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
      auto* actualObject = invokeSet(typeInfo.set, object);
      const ListFieldExt& ext =
          *static_cast<const ListFieldExt*>(typeInfo.typeExt);
      std::uint32_t size = ~0;
      WireType reportedType = WireTypeInfo::defaultValue();

      iprot->readListBegin(reportedType, size);
      struct Context {
        const TypeInfo* valInfo;
        Protocol_* iprot;
        ProtocolReaderStructReadState<Protocol_>& readState;
      };
      const Context context = {
          ext.valInfo,
          iprot,
          readState,
      };
      const auto reader = [](const void* context, void* value) {
        const auto& typedContext = *static_cast<const Context*>(context);
        read(
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

template <class Protocol_>
size_t write(Protocol_* iprot, const TypeInfo& typeInfo, ThriftValue value) {
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
        Protocol_* iprot;
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
            return write(typedContext.iprot, keyInfo, *getValue(keyInfo, key)) +
                write(typedContext.iprot,
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
        Protocol_* iprot;
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
            return write(
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
        Protocol_* iprot;
      };
      const Context context = {
          ext.valInfo,
          iprot,
      };
      written += ext.writeList(
          &context, value.object, [](const void* context, const void* value) {
            const auto& typedContext = *static_cast<const Context*>(context);
            const TypeInfo& valInfo = *typedContext.valInfo;
            return write(
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

template <class Protocol_>
size_t writeField(
    Protocol_* iprot, const FieldInfo& fieldInfo, const ThriftValue& value) {
  size_t written = iprot->writeFieldBegin(
      fieldInfo.name, fieldInfo.typeInfo->type, fieldInfo.id);
  written += write(iprot, *fieldInfo.typeInfo, value);
  written += iprot->writeFieldEnd();
  return written;
}

template <class Protocol_>
void read(Protocol_* iprot, const StructInfo& structInfo, void* object) {
  DCHECK(object);
  ProtocolReaderStructReadState<Protocol_> readState;
  readState.readStructBegin(iprot);

  if (UNLIKELY(structInfo.unionExt != nullptr)) {
    readState.fieldId = 0;
    readState.readFieldBegin(iprot);
    if (readState.atStop()) {
      structInfo.unionExt->clear(object);
      readState.readStructEnd(iprot);
      return;
    }
    if (const auto* fieldInfo = findFieldInfo(iprot, readState, structInfo)) {
      if (getActiveId(object, structInfo) != 0) {
        structInfo.unionExt->clear(object);
      }
      void* value = getMember(*fieldInfo, object);
      if (structInfo.unionExt->initMember[0] != nullptr) {
        structInfo.unionExt->initMember[fieldInfo - structInfo.fieldInfos](
            value);
      }
      read(iprot, *fieldInfo->typeInfo, readState, value);
      setActiveId(object, structInfo, fieldInfo->id);
    } else {
      skip(iprot, readState);
    }
    readState.readFieldEnd(iprot);
    readState.readFieldBegin(iprot);
    if (UNLIKELY(!readState.atStop())) {
      TProtocolException::throwUnionMissingStop();
    }
    readState.readStructEnd(iprot);
    return;
  }

  // Clear terse fields to intrinsic default values before deserialization.
  for (std::int16_t i = 0; i < structInfo.numFields; i++) {
    const auto& fieldInfo = structInfo.fieldInfos[i];
    clearTerseField(getMember(fieldInfo, object), fieldInfo);
  }

  // Define out of loop to call advanceToNextField after the loop ends.
  FieldID prevFieldId = 0;

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
    read(iprot, *fieldInfo->typeInfo, readState, getMember(*fieldInfo, object));
    markFieldAsSet(object, *fieldInfo, structInfo);
  }
}

template <class Protocol_>
size_t write(
    Protocol_* iprot, const StructInfo& structInfo, const void* object) {
  DCHECK(object);
  size_t written = iprot->writeStructBegin(structInfo.name);
  if (UNLIKELY(structInfo.unionExt != nullptr)) {
    const FieldInfo* end = structInfo.fieldInfos + structInfo.numFields;
    int activeId = getActiveId(object, structInfo);
    const FieldInfo* found = std::lower_bound(
        structInfo.fieldInfos,
        end,
        activeId,
        [](const FieldInfo& lhs, FieldID rhs) { return lhs.id < rhs; });
    if (found < end && found->id == activeId) {
      const OptionalThriftValue value =
          getValue(*found->typeInfo, getMember(*found, object));
      if (value.hasValue()) {
        written += writeField(iprot, *found, value.value());
      } else if (found->typeInfo->type == protocol::TType::T_STRUCT) {
        written += iprot->writeFieldBegin(
            found->name, found->typeInfo->type, found->id);
        written += iprot->writeStructBegin(found->name);
        written += iprot->writeStructEnd();
        written += iprot->writeFieldStop();
        written += iprot->writeFieldEnd();
      }
    }
  } else {
    for (std::int16_t index = 0; index < structInfo.numFields; index++) {
      const auto& fieldInfo = structInfo.fieldInfos[index];
      if (hasFieldValue(object, fieldInfo, structInfo)) {
        if (OptionalThriftValue value =
                getValue(*fieldInfo.typeInfo, getMember(fieldInfo, object))) {
          if (fieldInfo.qualifier == FieldQualifier::Terse &&
              !isTerseFieldSet(value.value(), fieldInfo)) {
            continue;
          }
          written += writeField(iprot, fieldInfo, value.value());
        }
      }
    }
  }

  written += iprot->writeFieldStop();
  written += iprot->writeStructEnd();
  return written;
}
} // namespace detail
} // namespace thrift
} // namespace apache
