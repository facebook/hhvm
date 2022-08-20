/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/lib/cpp/async/TBinaryAsyncChannel.h>

#include <thrift/lib/cpp/protocol/TBinaryProtocol.h>
#include <thrift/lib/cpp/transport/TBufferTransports.h>

#include <thrift/lib/cpp/async/TStreamAsyncChannel.h>
#include <thrift/lib/cpp/async/TUnframedAsyncChannel.h>

using apache::thrift::protocol::TBinaryProtocolT;
using apache::thrift::transport::TBufferBase;
using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::transport::TTransportException;

namespace apache {
namespace thrift {
namespace async {

bool tryReadUnframed(
    uint8_t* buffer,
    uint32_t bufferLength,
    uint32_t* messageLength,
    bool strictRead) {
  // Try unframed message.
  TMemoryBuffer memBuffer(buffer, bufferLength, TMemoryBuffer::OBSERVE);
  TBinaryProtocolT<TBufferBase> proto(&memBuffer);
  proto.setStrict(strictRead, true);
  try {
    std::string name;
    protocol::TMessageType messageType;
    int32_t seqid;
    proto.readMessageBegin(name, messageType, seqid);
    protocol::skip(proto, protocol::T_STRUCT);
    proto.readMessageEnd();
  } catch (const TTransportException& ex) {
    if (ex.getType() == TTransportException::END_OF_FILE) {
      // We're not at the end of the message yet.
      return false;
    }
    throw;
  }

  *messageLength = memBuffer.readEnd();
  return true;
}

namespace detail {

bool TBinaryACProtocolTraits::getMessageLength(
    uint8_t* buffer, uint32_t bufferLength, uint32_t* messageLength) {
  return tryReadUnframed(buffer, bufferLength, messageLength, strictRead_);
}

} // namespace detail
} // namespace async
} // namespace thrift
} // namespace apache
