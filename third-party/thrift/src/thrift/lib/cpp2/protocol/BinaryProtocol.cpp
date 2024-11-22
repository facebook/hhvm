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

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

#include <folly/Conv.h>
#include <folly/portability/GFlags.h>

FOLLY_GFLAGS_DEFINE_int32(
    thrift_cpp2_protocol_reader_string_limit,
    0,
    "Limit on string size when deserializing thrift, 0 is no limit");
FOLLY_GFLAGS_DEFINE_int32(
    thrift_cpp2_protocol_reader_container_limit,
    0,
    "Limit on container size when deserializing thrift, 0 is no limit");

namespace apache::thrift {

[[noreturn]] void BinaryProtocolReader::throwBadVersionIdentifier(int32_t sz) {
  throw TProtocolException(
      TProtocolException::BAD_VERSION,
      folly::to<std::string>("Bad version identifier, sz=", sz));
}

[[noreturn]] void BinaryProtocolReader::throwMissingVersionIdentifier(
    int32_t sz) {
  throw TProtocolException(
      TProtocolException::BAD_VERSION,
      folly::to<std::string>(
          "No version identifier... old protocol client in strict mode? sz=",
          sz));
}

void BinaryProtocolReader::skip(TType type, int depth) {
  if (depth >= FLAGS_thrift_protocol_max_depth) {
    protocol::TProtocolException::throwExceededDepthLimit();
  }
  size_t bytesToSkip = 0;
  switch (type) {
    case TType::T_BYTE:
    case TType::T_BOOL:
      bytesToSkip = sizeof(uint8_t);
      break;
    case TType::T_I16:
      bytesToSkip = sizeof(int16_t);
      break;
    case TType::T_FLOAT:
    case TType::T_I32:
      bytesToSkip = sizeof(int32_t);
      break;
    case TType::T_DOUBLE:
    case TType::T_U64:
    case TType::T_I64:
      bytesToSkip = sizeof(int64_t);
      break;
    case TType::T_UTF8:
    case TType::T_UTF16:
    case TType::T_STRING: {
      int32_t size = 0;
      auto in = getCursor();
      readI32(size);
      if (FOLLY_UNLIKELY(!in.canAdvance(static_cast<int32_t>(size)))) {
        protocol::TProtocolException::throwTruncatedData();
      }
      bytesToSkip = size;
      break;
    }
    case TType::T_STRUCT: {
      std::string name;
      TType ftype;
      readStructBegin(name);
      while (true) {
        int8_t rawType;
        readByte(rawType);
        ftype = static_cast<TType>(rawType);
        if (ftype == TType::T_STOP) {
          readStructEnd();
          return;
        }
        skipBytes(sizeof(int16_t));
        skip(ftype, depth + 1);
        readFieldEnd();
      }
    }
    case TType::T_MAP: {
      TType keyType;
      TType valType;
      uint32_t size;
      readMapBegin(keyType, valType, size);
      skip_n(*this, size, {keyType, valType}, depth + 1);
      readMapEnd();
      return;
    }
    case TType::T_SET: {
      TType elemType;
      uint32_t size;
      readSetBegin(elemType, size);
      skip_n(*this, size, {elemType}, depth + 1);
      readSetEnd();
      return;
    }
    case TType::T_LIST: {
      TType elemType;
      uint32_t size;
      readListBegin(elemType, size);
      skip_n(*this, size, {elemType}, depth + 1);
      readListEnd();
      return;
    }
    case TType::T_STOP:
    case TType::T_VOID:
    case TType::T_STREAM:
      // Unimplemented, fallback to default
    default: {
      TProtocolException::throwInvalidSkipType(type);
    }
  }
  skipBytes(bytesToSkip);
}

} // namespace apache::thrift
