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

#include <thrift/lib/cpp2/type/AnyDebugWriter.h>

namespace apache::thrift {

std::string anyDebugString(
    const type::AnyStruct& obj, bool try_unregistered_structs_as_any) {
  folly::IOBufQueue queue;
  detail::AnyDebugWriter proto(try_unregistered_structs_as_any);
  proto.setOutput(&queue);
  proto.write(obj);
  std::unique_ptr<folly::IOBuf> buf = queue.move();
  folly::ByteRange br = buf->coalesce();
  return std::string(reinterpret_cast<const char*>(br.data()), br.size());
}

std::string anyDebugString(
    const type::AnyData& obj, bool try_unregistered_structs_as_any) {
  return anyDebugString(obj.toThrift(), try_unregistered_structs_as_any);
}

namespace detail {
namespace {

std::string_view appendTypeUri(const type::TypeUri& uri) {
  switch (uri.getType()) {
    case apache::thrift::type::TypeUri::uri:
      return *uri.uri_ref();
    case apache::thrift::type::TypeUri::typeHashPrefixSha2_256:
      return uri.typeHashPrefixSha2_256_ref()->c_str();
    case apache::thrift::type::TypeUri::scopedName:
    case apache::thrift::type::TypeUri::__EMPTY__:
      return "(unspecified)";
  }
}

void appendType(const type::TypeStruct& type, fmt::memory_buffer& buf);

void appendTypeParams(
    const std::vector<type::TypeStruct>& types, fmt::memory_buffer& buf) {
  bool first = true;
  for (const auto& t : types) {
    if (!std::exchange(first, false)) {
      buf.push_back(',');
    }
    appendType(t, buf);
  }
}

std::string getTypeName(const type::TypeStruct& type) {
  switch (type.name()->getType()) {
    case type::TypeName::boolType:
      return "bool";
    case type::TypeName::byteType:
      return "byte";
    case type::TypeName::i16Type:
      return "i16";
    case type::TypeName::i32Type:
      return "i32";
    case type::TypeName::i64Type:
      return "i64";
    case type::TypeName::floatType:
      return "float";
    case type::TypeName::doubleType:
      return "double";
    case type::TypeName::stringType:
      return "string";
    case type::TypeName::binaryType:
      return "binary";
    case type::TypeName::enumType:
      return fmt::format(
          "enum<{}>", appendTypeUri(*type.name()->enumType_ref()));
    case type::TypeName::typedefType:
      return fmt::format(
          "typedef<{}>", appendTypeUri(*type.name()->typedefType_ref()));
    case type::TypeName::structType:
      return fmt::format(
          "struct<{}>", appendTypeUri(*type.name()->structType_ref()));
    case type::TypeName::unionType:
      return fmt::format(
          "union<{}>", appendTypeUri(*type.name()->unionType_ref()));
    case type::TypeName::exceptionType:
      return fmt::format(
          "exception<{}>", appendTypeUri(*type.name()->exceptionType_ref()));
    case type::TypeName::listType:
      return "list";
    case type::TypeName::setType:
      return "set";
    case type::TypeName::mapType:
      return "map";
    case type::TypeName::__EMPTY__:
      return "void";
  }
}

void appendType(const type::TypeStruct& type, fmt::memory_buffer& buf) {
  fmt::format_to(std::back_inserter(buf), "{}", getTypeName(type));
  if (!type.get_params().empty()) {
    buf.push_back('<');
    appendTypeParams(type.get_params(), buf);
    buf.push_back('>');
  }
}
} // namespace

uint32_t AnyDebugWriter::write(const type::AnyStruct& any) {
  uint32_t s = 0;
  s += writeStructBegin("AnyStruct");
  const type::Type& type = *any.type();
  type::BaseType baseType = type.baseType();

  s += writeFieldBegin(
      "type",
      TType::T_STRUCT,
      folly::to_underlying(
          op::get_field_id<type::AnyStruct, apache::thrift::ident::type>::
              value));
  s += write(type);
  s += writeFieldEnd();

  s += writeFieldBegin(
      "protocol",
      TType::T_STRUCT,
      folly::to_underlying(
          op::get_field_id<type::AnyStruct, apache::thrift::ident::protocol>::
              value));
  s += writeString(any.protocol()->name());
  s += writeFieldEnd();

  s += writeFieldBegin(
      "data",
      type::toTType(baseType),
      folly::to_underlying(
          op::get_field_id<type::AnyStruct, apache::thrift::ident::data>::
              value));
  s += writeUnregisteredAny(any);
  s += writeFieldEnd();

  s += writeFieldStop();
  s += writeStructEnd();

  return s;
}

uint32_t AnyDebugWriter::write(const type::AnyData& any) {
  return write(any.toThrift());
}

uint32_t AnyDebugWriter::writeUnregisteredAny(const type::AnyStruct& any) {
  const type::Protocol& prot = *any.protocol();
  const type::Type& type = *any.type();
  type::BaseType baseType = type.baseType();

  // Nested AnyStructs
  if (type == type::Type::get<type::struct_t<type::AnyStruct>>()) {
    type::AnyData data{any};
    type::AnyStruct nested_any;
    data.get(nested_any);
    return write(nested_any);
  } else {
    const folly::IOBuf& data = *any.data();
    if (prot == type::Protocol::get<type::StandardProtocol::Binary>()) {
      BinaryProtocolReader reader;
      reader.setInput(&data);
      return writeUnregisteredAnyImpl(reader, baseType);
    } else if (prot == type::Protocol::get<type::StandardProtocol::Compact>()) {
      CompactProtocolReader reader;
      reader.setInput(&data);
      return writeUnregisteredAnyImpl(reader, baseType);
    } else {
      return writeBinary(data);
    }
  }
}

template <class ProtocolReader, typename Tag>
type::native_type<Tag> AnyDebugWriter::decode(ProtocolReader& reader) {
  type::native_type<Tag> value;
  op::decode<Tag>(reader, value);
  return value;
}

template <class ProtocolReader>
bool AnyDebugWriter::tryAsAny(ProtocolReader& reader) {
  auto cursor = reader.getCursor();
  auto cur_pos = cursor.getCurrentPosition();
  try {
    type::AnyStruct any;
    op::decode<type::struct_t<type::AnyStruct>>(reader, any);
    if (!any.data()->empty() && type::AnyData::isValid(any)) {
      write(any);
      return true;
    }
  } catch (...) {
  }

  auto new_pos = cursor.getCurrentPosition();
  cursor.retreat(new_pos - cur_pos);
  reader.setInput(cursor);
  return false;
}

template <class ProtocolReader>
uint32_t AnyDebugWriter::writeUnregisteredAnyImpl(
    ProtocolReader& reader, const type::BaseType& type) {
  switch (type) {
    case type::BaseType::Void:
      return 0;
    case type::BaseType::Bool:
      return writeBool(decode<ProtocolReader, type::bool_t>(reader));
    case type::BaseType::Byte:
      return writeByte(decode<ProtocolReader, type::byte_t>(reader));
    case type::BaseType::I16:
      return writeI16(decode<ProtocolReader, type::i16_t>(reader));
    case type::BaseType::I32:
      return writeI32(decode<ProtocolReader, type::i32_t>(reader));
    case type::BaseType::I64:
      return writeI64(decode<ProtocolReader, type::i64_t>(reader));
    case type::BaseType::Float:
      return writeI64(decode<ProtocolReader, type::float_t>(reader));
    case type::BaseType::Double:
      return writeDouble(decode<ProtocolReader, type::double_t>(reader));
    case type::BaseType::String:
      return writeString(decode<ProtocolReader, type::string_t>(reader));
    case type::BaseType::Binary:
      return writeBinary(decode<ProtocolReader, type::binary_t>(reader));
    case type::BaseType::List: {
      uint32_t s = 0;
      TType elemType;
      uint32_t size;
      reader.readListBegin(elemType, size);
      s += writeListBegin(elemType, size);
      for (uint32_t i = 0; i < size; ++i) {
        s += writeUnregisteredAnyImpl(reader, type::toBaseType(elemType));
      }
      reader.readListEnd();
      s += writeListEnd();
      return s;
    }
    case type::BaseType::Set: {
      uint32_t s = 0;
      TType elemType;
      uint32_t size;
      reader.readSetBegin(elemType, size);
      s += writeSetBegin(elemType, size);
      for (uint32_t i = 0; i < size; ++i) {
        s += writeUnregisteredAnyImpl(reader, type::toBaseType(elemType));
      }
      reader.readSetEnd();
      s += writeSetEnd();
      return s;
    }
    case type::BaseType::Map: {
      uint32_t s = 0;
      TType keyType;
      TType valType;
      uint32_t size;
      reader.readMapBegin(keyType, valType, size);
      s += writeMapBegin(keyType, valType, size);
      for (uint32_t i = 0; i < size; ++i) {
        s += writeUnregisteredAnyImpl(reader, type::toBaseType(keyType));
        s += writeUnregisteredAnyImpl(reader, type::toBaseType(valType));
      }
      reader.readMapEnd();
      s += writeMapEnd();
      return s;
    }
    case type::BaseType::Enum:
      return writeI32(decode<ProtocolReader, type::i32_t>(reader));
    case type::BaseType::Struct:
      if (try_unregistered_structs_as_any_ && tryAsAny(reader)) {
        return 0;
      }
      [[fallthrough]];
    case type::BaseType::Union:
    case type::BaseType::Exception: {
      uint32_t s = 0;
      std::string name;
      int16_t fid;
      TType ftype;
      reader.readStructBegin(name);
      s += writeStructBegin(name.c_str());
      while (true) {
        reader.readFieldBegin(name, ftype, fid);
        if (ftype == protocol::T_STOP) {
          break;
        }
        s += writeFieldBegin(name.c_str(), ftype, fid);
        s += writeUnregisteredAnyImpl(reader, type::toBaseType(ftype));
        s += writeFieldEnd();
      }
      reader.readStructEnd();
      s += writeStructEnd();
      return s;
    }
  }
}

uint32_t AnyDebugWriter::write(const type::Type& type) {
  fmt::memory_buffer buf;
  appendType(type.toThrift(), buf);
  return writeString(fmt::to_string(buf));
}
} // namespace detail
} // namespace apache::thrift
