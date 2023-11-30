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
#include <thrift/lib/cpp2/protocol/TableBasedSerializerImpl.h>

namespace apache {
namespace thrift {
namespace detail {

// Returns active field id for a Thrift union object.
int getActiveId(const void* object, const StructInfo& info) {
  auto getActiveIdFunc = info.unionExt->getActiveId;
  if (getActiveIdFunc != nullptr) {
    return getActiveIdFunc(object);
  }
  return *reinterpret_cast<const int*>(
      static_cast<const char*>(object) + info.unionExt->unionTypeOffset);
}

// Sets the active field id for a Thrift union object.
void setActiveId(void* object, const StructInfo& info, int value) {
  auto setActiveIdFunc = info.unionExt->setActiveId;
  if (setActiveIdFunc != nullptr) {
    setActiveIdFunc(object, value);
  } else {
    *reinterpret_cast<int*>(
        static_cast<char*>(object) + info.unionExt->unionTypeOffset) = value;
  }
}

// Checks whether if a field value is safe to retrieve. For an optional field,
// a field is nullable, so it is safe to get the field value if it is
// explicitly set. An unqualified and terse fields are always safe to retrive
// their values.
bool hasFieldValue(
    const void* object, const FieldInfo& info, const StructInfo& structInfo) {
  switch (info.qualifier) {
    case FieldQualifier::Unqualified:
    case FieldQualifier::Terse:
      return true;
    case FieldQualifier::Optional: {
      if (structInfo.getIsset != nullptr) {
        return structInfo.getIsset(object, info.issetOffset);
      }
      if (info.issetOffset == 0) {
        return true; // return true for union fields
      }
      return *reinterpret_cast<const bool*>(
          static_cast<const char*>(object) + info.issetOffset);
    }
  }
  return false;
}

// A helper function to set a field to its intrinsic default value.
void setToIntrinsicDefault(void* value, const FieldInfo& info) {
  const TypeInfo& typeInfo = *info.typeInfo;
  const void* typeInfoExt = typeInfo.typeExt;
  switch (typeInfo.type) {
    case protocol::TType::T_STRUCT: {
      void* structField =
          typeInfo.set ? invokeStructSet(typeInfo, value) : value;
      const auto& structInfo = *static_cast<const StructInfo*>(typeInfoExt);
      for (std::int16_t index = 0; index < structInfo.numFields; index++) {
        const auto& fieldInfo = structInfo.fieldInfos[index];
        setToIntrinsicDefault(getMember(fieldInfo, structField), fieldInfo);
      }
      break;
    }
    case protocol::TType::T_I64:
      reinterpret_cast<void (*)(void*, std::int64_t)>(typeInfo.set)(value, 0);
      break;
    case protocol::TType::T_I32:
      reinterpret_cast<void (*)(void*, std::int32_t)>(typeInfo.set)(value, 0);
      break;
    case protocol::TType::T_I16:
      reinterpret_cast<void (*)(void*, std::int16_t)>(typeInfo.set)(value, 0);
      break;
    case protocol::TType::T_BYTE:
      reinterpret_cast<void (*)(void*, std::int8_t)>(typeInfo.set)(value, 0);
      break;
    case protocol::TType::T_BOOL:
      reinterpret_cast<void (*)(void*, bool)>(typeInfo.set)(value, false);
      break;
    case protocol::TType::T_DOUBLE:
      reinterpret_cast<void (*)(void*, double)>(typeInfo.set)(value, 0.0);
      break;
    case protocol::TType::T_FLOAT:
      reinterpret_cast<void (*)(void*, float)>(typeInfo.set)(value, 0.0);
      break;
    case protocol::TType::T_STRING: {
      switch (*static_cast<const StringFieldType*>(typeInfoExt)) {
        case StringFieldType::String:
          static_cast<std::string*>(value)->clear();
          break;
        case StringFieldType::StringView:
        case StringFieldType::BinaryStringView:
          reinterpret_cast<void (*)(void*, const std::string&)>(typeInfo.set)(
              value, "");
          break;
        case StringFieldType::Binary:
          static_cast<std::string*>(value)->clear();
          break;
        case StringFieldType::IOBufObj: {
          static_cast<folly::IOBuf*>(value)->clear();
          break;
        }
        case StringFieldType::IOBuf:
          static_cast<folly::IOBuf*>(value)->clear();
          break;
        case StringFieldType::IOBufPtr: {
          // Default constructed IOBufPtr does not own IOBuf. CLear only if
          // IOBufPtr owns IOBuf.
          auto&& iobuf_ptr =
              *static_cast<std::unique_ptr<folly::IOBuf>*>(value);
          if (iobuf_ptr) {
            iobuf_ptr->clear();
          }
          break;
        }
      }
      break;
    }
    case protocol::TType::T_MAP: {
      static_cast<const MapFieldExt*>(typeInfoExt)->clear(value);
      break;
    }
    case protocol::TType::T_SET: {
      static_cast<const SetFieldExt*>(typeInfoExt)->clear(value);
      break;
    }
    case protocol::TType::T_LIST: {
      static_cast<const ListFieldExt*>(typeInfoExt)->clear(value);
      break;
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
}

void clearTerseField(void* value, const FieldInfo& info) {
  if (info.qualifier != FieldQualifier::Terse) {
    return;
  }
  const TypeInfo& typeInfo = *info.typeInfo;
  const void* typeInfoExt = typeInfo.typeExt;
  switch (typeInfo.type) {
    case protocol::TType::T_STRUCT: {
      // We only clear terse fields in a terse struct field.
      void* structField =
          typeInfo.set ? invokeStructSet(typeInfo, value) : value;
      const auto& structInfo = *static_cast<const StructInfo*>(typeInfoExt);
      for (std::int16_t index = 0; index < structInfo.numFields; index++) {
        const auto& fieldInfo = structInfo.fieldInfos[index];
        clearTerseField(getMember(fieldInfo, structField), fieldInfo);
      }
      break;
    }
    case protocol::TType::T_I64:
    case protocol::TType::T_I32:
    case protocol::TType::T_I16:
    case protocol::TType::T_BYTE:
    case protocol::TType::T_BOOL:
    case protocol::TType::T_DOUBLE:
    case protocol::TType::T_FLOAT:
    case protocol::TType::T_STRING:
    case protocol::TType::T_MAP:
    case protocol::TType::T_SET:
    case protocol::TType::T_LIST:
    case protocol::TType::T_STOP:
    case protocol::TType::T_VOID:
    case protocol::TType::T_STREAM:
    case protocol::TType::T_UTF8:
    case protocol::TType::T_U64:
    case protocol::TType::T_UTF16:
      setToIntrinsicDefault(value, info);
      break;
  }
}

// A terse field skips serialization if it is equal to the intrinsic default.
// Note, for a struct terse field, serialization is skipped if it is empty. If
// it has an unqualified field, it is not eligible to be empty. A struct is
// empty, if optional fields are not explicitly and terse fields are equal to
// the intrinsic default.
bool isTerseFieldSet(const ThriftValue& value, const FieldInfo& info) {
  const void* typeInfoExt = info.typeInfo->typeExt;
  switch (info.typeInfo->type) {
    case protocol::TType::T_STRUCT: {
      const auto& structInfo = *static_cast<const StructInfo*>(typeInfoExt);
      // union.
      if (structInfo.unionExt != nullptr) {
        return getActiveId(value.object, structInfo) != 0;
      }
      // struct and exception.
      for (std::int16_t index = 0; index < structInfo.numFields; index++) {
        const auto& fieldInfo = structInfo.fieldInfos[index];
        if (hasFieldValue(value.object, fieldInfo, structInfo)) {
          if (OptionalThriftValue fieldValue = getValue(
                  *fieldInfo.typeInfo, getMember(fieldInfo, value.object))) {
            if (isFieldNotEmpty(
                    value.object, fieldValue.value(), fieldInfo, structInfo)) {
              return true;
            }
          }
        }
      }
      return false;
    }
    case protocol::TType::T_I64:
      return value.int64Value != 0;
    case protocol::TType::T_I32:
      return value.int32Value != 0;
    case protocol::TType::T_I16:
      return value.int16Value != 0;
    case protocol::TType::T_BYTE:
      return value.int8Value != 0;
    case protocol::TType::T_BOOL:
      return value.boolValue != false;
    case protocol::TType::T_DOUBLE:
      return value.doubleValue != 0.0;
    case protocol::TType::T_FLOAT:
      return value.floatValue != 0.0;
    case protocol::TType::T_STRING: {
      switch (*static_cast<const StringFieldType*>(typeInfoExt)) {
        case StringFieldType::String:
          return !static_cast<const std::string*>(value.object)->empty();
        case StringFieldType::StringView:
        case StringFieldType::BinaryStringView:
          return !value.stringViewValue.empty();
        case StringFieldType::Binary:
          return !static_cast<const std::string*>(value.object)->empty();
        case StringFieldType::IOBufObj:
          return !static_cast<const folly::IOBuf*>(value.iobuf)->empty();
        case StringFieldType::IOBuf:
          return !static_cast<const folly::IOBuf*>(value.object)->empty();
        case StringFieldType::IOBufPtr:
          return !(*static_cast<const std::unique_ptr<folly::IOBuf>*>(
                       value.object))
                      ->empty();
      }
    }
    case protocol::TType::T_MAP: {
      const auto& ext = *static_cast<const MapFieldExt*>(typeInfoExt);
      return ext.size(value.object) != 0;
    }
    case protocol::TType::T_SET: {
      const auto& ext = *static_cast<const SetFieldExt*>(typeInfoExt);
      return ext.size(value.object) != 0;
    }
    case protocol::TType::T_LIST: {
      const auto& ext = *static_cast<const ListFieldExt*>(typeInfoExt);
      return ext.size(value.object) != 0;
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
  return false;
}

bool isFieldNotEmpty(
    const void* object,
    const ThriftValue& value,
    const FieldInfo& info,
    const StructInfo& structInfo) {
  switch (info.qualifier) {
    case FieldQualifier::Unqualified:
    case FieldQualifier::Optional:
      return hasFieldValue(object, info, structInfo);
    case FieldQualifier::Terse:
      return isTerseFieldSet(value, info);
  }
  return false;
}

void markFieldAsSet(
    void* object, const FieldInfo& info, const StructInfo& structInfo) {
  if (structInfo.setIsset != nullptr) {
    structInfo.setIsset(object, info.issetOffset, true);
    return;
  }
  if (info.issetOffset == 0) {
    return;
  }
  *reinterpret_cast<bool*>(static_cast<char*>(object) + info.issetOffset) =
      true;
}

} // namespace detail
} // namespace thrift
} // namespace apache
