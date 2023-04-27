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

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace thrift {
namespace py3 {
using apache::thrift::protocol::PROTOCOL_TYPES;
template <typename T>
std::unique_ptr<folly::IOBuf> serialize(const T* obj, PROTOCOL_TYPES protocol) {
  auto queue = folly::IOBufQueue{folly::IOBufQueue::cacheChainLength()};
  switch (protocol) {
    case PROTOCOL_TYPES::T_COMPACT_PROTOCOL:
      apache::thrift::CompactSerializer::serialize(
          *obj, &queue, apache::thrift::SHARE_EXTERNAL_BUFFER);
      break;
    case PROTOCOL_TYPES::T_BINARY_PROTOCOL:
      apache::thrift::BinarySerializer::serialize(
          *obj, &queue, apache::thrift::SHARE_EXTERNAL_BUFFER);
      break;
    case PROTOCOL_TYPES::T_SIMPLE_JSON_PROTOCOL:
      apache::thrift::SimpleJSONSerializer::serialize(
          *obj, &queue, apache::thrift::SHARE_EXTERNAL_BUFFER);
      break;
    case PROTOCOL_TYPES::T_JSON_PROTOCOL:
      apache::thrift::JSONSerializer::serialize(
          *obj, &queue, apache::thrift::SHARE_EXTERNAL_BUFFER);
      break;
    default:
      LOG(FATAL) << "Bad serialization protocol " << uint8_t(protocol);
  }
  return queue.move();
}

template <typename T>
size_t deserialize(const folly::IOBuf* buf, T* obj, PROTOCOL_TYPES protocol) {
  auto queue = folly::IOBufQueue{folly::IOBufQueue::cacheChainLength()};
  switch (protocol) {
    case PROTOCOL_TYPES::T_COMPACT_PROTOCOL:
      return apache::thrift::CompactSerializer::deserialize(
          buf, *obj, apache::thrift::SHARE_EXTERNAL_BUFFER);
      break;
    case PROTOCOL_TYPES::T_BINARY_PROTOCOL:
      return apache::thrift::BinarySerializer::deserialize(
          buf, *obj, apache::thrift::SHARE_EXTERNAL_BUFFER);
    case PROTOCOL_TYPES::T_SIMPLE_JSON_PROTOCOL:
      return apache::thrift::SimpleJSONSerializer::deserialize(
          buf, *obj, apache::thrift::SHARE_EXTERNAL_BUFFER);
      break;
    case PROTOCOL_TYPES::T_JSON_PROTOCOL:
      return apache::thrift::JSONSerializer::deserialize(
          buf, *obj, apache::thrift::SHARE_EXTERNAL_BUFFER);
      break;
    default:
      LOG(FATAL) << "Bad serialization protocol " << uint8_t(protocol);
  }
}
} // namespace py3
} // namespace thrift
