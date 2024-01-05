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

std::string anyDebugString(const type::AnyStruct& obj) {
  folly::IOBufQueue queue;
  detail::AnyDebugWriter proto;
  proto.setOutput(&queue);
  proto.write(obj);
  std::unique_ptr<folly::IOBuf> buf = queue.move();
  folly::ByteRange br = buf->coalesce();
  return std::string(reinterpret_cast<const char*>(br.data()), br.size());
}

std::string anyDebugString(const type::AnyData& obj) {
  return anyDebugString(obj.toThrift());
}

namespace detail {

uint32_t AnyDebugWriter::write(const type::AnyStruct& any) {
  uint32_t s = 0;
  s += writeStructBegin("AnyStruct");
  const type::Type& type = *any.type();
  type::BaseType baseType = type.baseType();

  // TODO(rashmim): Add type and protocol fields
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

template <class ProtocolReader>
uint32_t AnyDebugWriter::writeUnregisteredAnyImpl(
    ProtocolReader&, const type::BaseType& type) {
  switch (type) {
    default:
      return writeString("Unrecognized type");
  }
}
} // namespace detail
} // namespace apache::thrift
