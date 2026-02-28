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

namespace apache {
namespace thrift {

namespace detail {
namespace compact_v1 {

constexpr uint8_t kCompactV1ProtocolVersion = 0x01;
}
} // namespace detail

inline uint32_t CompactV1ProtocolWriter::writeMessageBegin(
    const std::string& name, MessageType messageType, int32_t seqid) {
  uint32_t wsize = 0;
  wsize += writeByte(apache::thrift::detail::compact::PROTOCOL_ID);
  wsize += writeByte(
      apache::thrift::detail::compact_v1::kCompactV1ProtocolVersion |
      ((static_cast<int32_t>(messageType)
        << apache::thrift::detail::compact::TYPE_SHIFT_AMOUNT) &
       apache::thrift::detail::compact::TYPE_MASK));
  wsize += apache::thrift::util::writeVarint(out_, seqid);
  wsize += writeString(name);
  return wsize;
}

inline uint32_t CompactV1ProtocolWriter::writeDouble(double dub) {
  static_assert(sizeof(double) == sizeof(uint64_t));
  static_assert(std::numeric_limits<double>::is_iec559);

  uint64_t bits = folly::bit_cast<uint64_t>(dub);
  out_.writeLE(bits);
  return sizeof(bits);
}

inline void CompactV1ProtocolWriter::rewriteDouble(double dub, int64_t offset) {
  auto cursor = out_.tail<folly::io::CursorAccess::PRIVATE>(offset);
  cursor.writeLE(folly::bit_cast<uint64_t>(dub));
}

inline void CompactV1ProtocolReader::readMessageBegin(
    std::string& name, MessageType& messageType, int32_t& seqid) {
  int8_t protocolId;
  int8_t versionAndType;

  readByte(protocolId);
  if (protocolId != apache::thrift::detail::compact::PROTOCOL_ID) {
    throw TProtocolException(
        TProtocolException::BAD_VERSION, "Bad protocol identifier");
  }

  readByte(versionAndType);
  if ((int8_t)(versionAndType & VERSION_MASK) !=
      apache::thrift::detail::compact_v1::kCompactV1ProtocolVersion) {
    throw TProtocolException(
        TProtocolException::BAD_VERSION, "Bad protocol version");
  }

  messageType =
      (MessageType)((versionAndType &
                     apache::thrift::detail::compact::TYPE_MASK) >>
                    apache::thrift::detail::compact::TYPE_SHIFT_AMOUNT);
  apache::thrift::util::readVarint(in_, seqid);
  readString(name);
}

inline void CompactV1ProtocolReader::readDouble(double& dub) {
  static_assert(sizeof(double) == sizeof(uint64_t));
  static_assert(std::numeric_limits<double>::is_iec559);

  uint64_t bits = in_.readLE<int64_t>();
  dub = folly::bit_cast<double>(bits);
}

} // namespace thrift
} // namespace apache
