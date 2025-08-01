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

#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>

#include <folly/io/IOBufQueue.h>

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift {

namespace {

template <typename ProtocolWriter>
std::unique_ptr<folly::IOBuf> addEnvelope(
    MessageType mtype,
    int32_t seqid,
    folly::StringPiece methodName,
    std::unique_ptr<folly::IOBuf> buf) {
  ProtocolWriter writer;
  auto messageBeginSizeUpperBound = writer.serializedMessageSize(methodName);
  folly::IOBufQueue queue;

  // If possible, serialize header into the headeroom of buf.
  if (!buf->isChained() && buf->headroom() >= messageBeginSizeUpperBound &&
      !buf->isSharedOne()) {
    // Store previous state of the buffer pointers and rewind it.
    auto startBuffer = buf->buffer();
    auto start = buf->data();
    auto origLen = buf->length();
    buf->trimEnd(origLen);
    buf->retreat(start - startBuffer);

    queue.append(std::move(buf), false);
    writer.setOutput(&queue);
    writer.writeMessageBegin(methodName, mtype, seqid);

    // Move the new data to come right before the old data and restore the
    // old tail pointer.
    buf = queue.move();
    buf->advance(start - buf->tail());
    buf->append(origLen);

    return buf;
  } else {
    auto messageBeginBuf = folly::IOBuf::create(messageBeginSizeUpperBound);
    queue.append(std::move(messageBeginBuf));
    writer.setOutput(&queue);
    writer.writeMessageBegin(methodName, mtype, seqid);
    queue.append(std::move(buf));
    return queue.move();
  }

  // We skip writeMessageEnd because for both Binary and Compact it's a noop.
}

std::unique_ptr<folly::IOBuf> addEnvelope(
    uint16_t protocolId,
    MessageType mtype,
    int32_t seqid,
    folly::StringPiece methodName,
    std::unique_ptr<folly::IOBuf> buf) {
  switch (protocolId) {
    case protocol::T_BINARY_PROTOCOL:
      return addEnvelope<BinaryProtocolWriter>(
          mtype, seqid, methodName, std::move(buf));

    case protocol::T_COMPACT_PROTOCOL:
      return addEnvelope<CompactProtocolWriter>(
          mtype, seqid, methodName, std::move(buf));
    default:
      LOG(FATAL) << "Unsupported protocolId: " << protocolId;
  }
}

template <typename ProtocolWriter>
std::unique_ptr<folly::IOBuf> makeEnvelope(
    MessageType mtype, int32_t seqid, folly::StringPiece methodName) {
  ProtocolWriter writer;
  auto messageBeginSizeUpperBound = writer.serializedMessageSize(methodName);
  folly::IOBufQueue queue;
  auto envelopeBuf = folly::IOBuf::create(messageBeginSizeUpperBound);
  queue.append(std::move(envelopeBuf));
  writer.setOutput(&queue);
  writer.writeMessageBegin(methodName, mtype, seqid);
  return queue.move();
}

} // namespace

SerializedRequest SerializedCompressedRequest::uncompress() && {
  if (compression_ == CompressionAlgorithm::NONE) {
    return SerializedRequest(std::move(buffer_));
  }

  if (!payloadSerializer_) {
    return SerializedRequest(
        rocket::PayloadSerializer::getInstance()->uncompressBuffer(
            std::move(buffer_), compression_));
  }

  return SerializedRequest(
      (*payloadSerializer_)
          ->uncompressBuffer(std::move(buffer_), compression_));
}

SerializedCompressedRequest SerializedCompressedRequest::clone() const {
  return SerializedCompressedRequest(buffer_->clone(), compression_);
}

LegacySerializedRequest::LegacySerializedRequest(
    uint16_t protocolId,
    int32_t seqid,
    folly::StringPiece methodName,
    SerializedRequest&& serializedRequest)
    : buffer(addEnvelope(
          protocolId,
          MessageType::T_CALL,
          seqid,
          methodName,
          std::move(serializedRequest.buffer))) {}

LegacySerializedRequest::LegacySerializedRequest(
    uint16_t protocolId,
    folly::StringPiece methodName,
    SerializedRequest&& serializedRequest)
    : LegacySerializedRequest(
          protocolId, 0, methodName, std::move(serializedRequest)) {}

void ResponsePayload::transform(
    std::vector<uint16_t>& writeTrans, size_t minCompressBytes) {
  buffer_ = transport::THeader::transform(
      std::move(buffer_), writeTrans, minCompressBytes);
}

ResponsePayload SerializedResponse::extractPayload(
    bool includeEnvelope,
    int16_t protocolId,
    int32_t seqId,
    MessageType mtype,
    folly::StringPiece methodName) && {
  if (includeEnvelope) {
    auto payload =
        addEnvelope(protocolId, mtype, seqId, methodName, std::move(buffer));
    return ResponsePayload{std::move(payload)};
  } else {
    return ResponsePayload{std::move(buffer)};
  }
}

ResponsePayload SerializedResponse::extractPayload(bool includeEnvelope) && {
  if (includeEnvelope) {
    LOG(FATAL) << "Unsupported conversion to legacy response";
  } else {
    return ResponsePayload{std::move(buffer)};
  }
}

LegacySerializedResponse::LegacySerializedResponse(
    uint16_t protocolId,
    int32_t seqid,
    folly::StringPiece methodName,
    SerializedResponse&& serializedResponse)
    : LegacySerializedResponse(
          protocolId,
          seqid,
          MessageType::T_REPLY,
          methodName,
          std::move(serializedResponse)) {}

LegacySerializedResponse::LegacySerializedResponse(
    uint16_t protocolId,
    folly::StringPiece methodName,
    SerializedResponse&& serializedResponse)
    : LegacySerializedResponse(
          protocolId, 0, methodName, std::move(serializedResponse)) {}

LegacySerializedResponse::LegacySerializedResponse(
    uint16_t protocolId,
    int32_t seqid,
    folly::StringPiece methodName,
    const TApplicationException& ex)
    : buffer(serializeError</*includeEnvelope=*/true>(
          protocolId, ex, methodName.str(), seqid)) {}

LegacySerializedResponse::LegacySerializedResponse(
    uint16_t protocolId,
    folly::StringPiece methodName,
    const TApplicationException& ex)
    : LegacySerializedResponse(protocolId, 0, methodName, ex) {}

LegacySerializedResponse::LegacySerializedResponse(
    uint16_t protocolId,
    int32_t seqid,
    MessageType mtype,
    folly::StringPiece methodName,
    SerializedResponse&& serializedResponse)
    : buffer(addEnvelope(
          protocolId,
          mtype,
          seqid,
          methodName,
          std::move(serializedResponse.buffer))) {}

std::unique_ptr<folly::IOBuf> LegacySerializedResponse::envelope(
    uint16_t protocolId,
    MessageType mtype,
    int32_t seqid,
    folly::StringPiece methodName) {
  switch (protocolId) {
    case protocol::T_BINARY_PROTOCOL:
      return makeEnvelope<BinaryProtocolWriter>(mtype, seqid, methodName);
    case protocol::T_COMPACT_PROTOCOL:
      return makeEnvelope<CompactProtocolWriter>(mtype, seqid, methodName);
    default:
      LOG(FATAL) << "Unsupported protocolId: " << protocolId;
  }
}

std::pair<MessageType, ResponsePayload>
LegacySerializedResponse::extractPayload(
    bool includeEnvelope, int16_t protocolId, int32_t seqId) && {
  int32_t _;
  std::string methodName;
  MessageType mtype;

  size_t headerSize{0};
  try {
    switch (protocolId) {
      case protocol::T_BINARY_PROTOCOL: {
        BinaryProtocolReader iprot;
        iprot.setInput(buffer.get());
        iprot.readMessageBegin(methodName, mtype, _);
        headerSize = iprot.getCursorPosition();
        break;
      }
      case protocol::T_COMPACT_PROTOCOL: {
        CompactProtocolReader iprot;
        iprot.setInput(buffer.get());
        iprot.readMessageBegin(methodName, mtype, _);
        headerSize = iprot.getCursorPosition();
        break;
      }
      default: {
        LOG(ERROR) << "invalid protocolId: " << protocolId;
        break;
      }
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to parse header. Invalid message: "
               << folly::exceptionStr(e);
  }

  if (!includeEnvelope || seqId) {
    IOBufQueue bufQueue;
    bufQueue.append(std::move(buffer));
    bufQueue.trimStart(headerSize);

    if (includeEnvelope) {
      folly::IOBufQueue resultQueue{folly::IOBufQueue::cacheChainLength()};
      try {
        switch (protocolId) {
          case apache::thrift::protocol::T_BINARY_PROTOCOL: {
            BinaryProtocolWriter iprot;
            const auto messageSize = iprot.serializedMessageSize(methodName);
            resultQueue.preallocate(messageSize, messageSize);
            iprot.setOutput(&resultQueue);
            iprot.writeMessageBegin(methodName, mtype, seqId);
            break;
          }
          case apache::thrift::protocol::T_COMPACT_PROTOCOL: {
            CompactProtocolWriter iprot;
            const auto messageSize = iprot.serializedMessageSize(methodName);
            resultQueue.preallocate(messageSize, messageSize);
            iprot.setOutput(&resultQueue);
            iprot.writeMessageBegin(methodName, mtype, seqId);
            break;
          }
          default: {
            LOG(ERROR) << "invalid protocolId: " << protocolId;
            break;
          }
        }
      } catch (...) {
        LOG(ERROR) << "Failed to write header.";
      }
      resultQueue.append(std::move(bufQueue));
      return std::make_pair(mtype, ResponsePayload{resultQueue.move()});
    } else {
      return std::make_pair(mtype, ResponsePayload{bufQueue.move()});
    }
  }
  return std::make_pair(mtype, ResponsePayload{std::move(buffer)});
}

} // namespace apache::thrift
