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

/*
 * Deprecated. For backcompat only with stored data serialized with this version
 * of compact protocol.
 *
 * Deprecated because it serializes doubles incorrectly, in little-endian byte
 * order, and is therefore incompatible with all other language implementations.
 */

#pragma once

#include <folly/Portability.h>
#include <folly/lang/Bits.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift {

class CompactV1ProtocolReader;

class CompactV1ProtocolWriter : protected CompactProtocolWriter {
 public:
  using ProtocolReader = CompactV1ProtocolReader;

  using CompactProtocolWriter::CompactProtocolWriter;
  using CompactProtocolWriter::kHasIndexSupport;
  using CompactProtocolWriter::kSortKeys;
  using CompactProtocolWriter::protocolType;
  using CompactProtocolWriter::setOutput;

  inline uint32_t writeMessageBegin(
      const std::string& name, MessageType messageType, int32_t seqid);
  using CompactProtocolWriter::writeBool;
  using CompactProtocolWriter::writeByte;
  using CompactProtocolWriter::writeFieldBegin;
  using CompactProtocolWriter::writeFieldEnd;
  using CompactProtocolWriter::writeFieldStop;
  using CompactProtocolWriter::writeI16;
  using CompactProtocolWriter::writeI32;
  using CompactProtocolWriter::writeI64;
  using CompactProtocolWriter::writeListBegin;
  using CompactProtocolWriter::writeListEnd;
  using CompactProtocolWriter::writeMapBegin;
  using CompactProtocolWriter::writeMapEnd;
  using CompactProtocolWriter::writeMessageEnd;
  using CompactProtocolWriter::writeSetBegin;
  using CompactProtocolWriter::writeSetEnd;
  using CompactProtocolWriter::writeStructBegin;
  using CompactProtocolWriter::writeStructEnd;
  inline uint32_t writeDouble(double dub);
  using CompactProtocolWriter::writeBinary;
  using CompactProtocolWriter::writeFloat;
  using CompactProtocolWriter::writeRaw;
  using CompactProtocolWriter::writeString;

  using CompactProtocolWriter::serializedFieldSize;
  using CompactProtocolWriter::serializedMessageSize;
  using CompactProtocolWriter::serializedSizeBinary;
  using CompactProtocolWriter::serializedSizeBool;
  using CompactProtocolWriter::serializedSizeByte;
  using CompactProtocolWriter::serializedSizeDouble;
  using CompactProtocolWriter::serializedSizeFloat;
  using CompactProtocolWriter::serializedSizeI16;
  using CompactProtocolWriter::serializedSizeI32;
  using CompactProtocolWriter::serializedSizeI64;
  using CompactProtocolWriter::serializedSizeListBegin;
  using CompactProtocolWriter::serializedSizeListEnd;
  using CompactProtocolWriter::serializedSizeMapBegin;
  using CompactProtocolWriter::serializedSizeMapEnd;
  using CompactProtocolWriter::serializedSizeSetBegin;
  using CompactProtocolWriter::serializedSizeSetEnd;
  using CompactProtocolWriter::serializedSizeStop;
  using CompactProtocolWriter::serializedSizeString;
  using CompactProtocolWriter::serializedSizeZCBinary;
  using CompactProtocolWriter::serializedStructSize;

  using CompactProtocolWriter::tail;

  inline void rewriteDouble(double dub, int64_t offset);

  static constexpr bool kSupportsArithmeticVectors() { return false; }
};

class CompactV1ProtocolReader : protected CompactProtocolReader {
 public:
  using ProtocolWriter = CompactV1ProtocolWriter;

  using CompactProtocolReader::CompactProtocolReader;
  using CompactProtocolReader::kOmitsContainerSizes;
  using CompactProtocolReader::kUsesFieldNames;
  using CompactProtocolReader::protocolType;

  using CompactProtocolReader::setContainerSizeLimit;
  using CompactProtocolReader::setInput;
  using CompactProtocolReader::setStringSizeLimit;

  inline void readMessageBegin(
      std::string& name, MessageType& messageType, int32_t& seqid);
  using CompactProtocolReader::readBool;
  using CompactProtocolReader::readByte;
  using CompactProtocolReader::readFieldBegin;
  using CompactProtocolReader::readFieldEnd;
  using CompactProtocolReader::readI16;
  using CompactProtocolReader::readI32;
  using CompactProtocolReader::readI64;
  using CompactProtocolReader::readListBegin;
  using CompactProtocolReader::readListEnd;
  using CompactProtocolReader::readMapBegin;
  using CompactProtocolReader::readMapEnd;
  using CompactProtocolReader::readMessageEnd;
  using CompactProtocolReader::readSetBegin;
  using CompactProtocolReader::readSetEnd;
  using CompactProtocolReader::readStructBegin;
  using CompactProtocolReader::readStructEnd;
  inline void readDouble(double& dub);
  using CompactProtocolReader::fixedSizeInContainer;
  using CompactProtocolReader::peekList;
  using CompactProtocolReader::peekMap;
  using CompactProtocolReader::peekSet;
  using CompactProtocolReader::readBinary;
  using CompactProtocolReader::readFloat;
  using CompactProtocolReader::readString;
  using CompactProtocolReader::skip;
  using CompactProtocolReader::skipBytes;

  using CompactProtocolReader::getCursor;
  using CompactProtocolReader::getCursorPosition;
};

} // namespace apache::thrift

#include <thrift/lib/cpp2/protocol/CompactV1Protocol-inl.h>
