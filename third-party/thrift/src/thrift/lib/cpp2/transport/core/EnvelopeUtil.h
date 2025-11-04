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

#include <folly/Optional.h>
#include <folly/Utility.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>

namespace apache::thrift {

class EnvelopeUtil {
 public:
  struct Envelope {
    protocol::PROTOCOL_TYPES protocolId;
    MessageType messageType;
    std::string methodName;
  };

  // Strips the envelope out of the payload and extracts method name from it.
  // This function is required to maintain legacy compatibility.  Eventually,
  // the envelope should only be used with the header transport (i.e., not even
  // created in the first place), at which point this function will be
  // deprecated.
  static folly::Optional<std::pair<Envelope, std::unique_ptr<folly::IOBuf>>>
  stripRequestEnvelope(std::unique_ptr<folly::IOBuf>&& payload) noexcept {
    auto ret = stripEnvelope(std::move(payload));
    if (ret) {
      DCHECK(MessageType::T_CALL == ret->first.messageType);
    }
    return ret;
  }

  static folly::Optional<std::pair<Envelope, std::unique_ptr<folly::IOBuf>>>
  stripEnvelope(std::unique_ptr<folly::IOBuf>&& payload) noexcept {
    uint64_t sz;
    while (payload->length() == 0) {
      if (payload->next() != payload.get()) {
        payload = payload->pop();
      } else {
        LOG(ERROR) << "Payload is empty";
        return folly::none;
      }
    }
    Envelope envelope;
    try {
      // Sequence id is always 0 in the envelope.  So we ignore it.
      int32_t seqId;
      auto protByte = payload->data()[0];
      switch (protByte) {
        case 0x80: {
          BinaryProtocolReader reader;
          envelope.protocolId = protocol::T_BINARY_PROTOCOL;
          reader.setInput(payload.get());
          reader.readMessageBegin(
              envelope.methodName, envelope.messageType, seqId);
          sz = reader.getCursorPosition();
          break;
        }
        case 0x82: {
          envelope.protocolId = protocol::T_COMPACT_PROTOCOL;
          CompactProtocolReader reader;
          reader.setInput(payload.get());
          reader.readMessageBegin(
              envelope.methodName, envelope.messageType, seqId);
          sz = reader.getCursorPosition();
          break;
        }
        // TODO: Add Frozen2 case.
        default:
          LOG(ERROR) << "Unknown protocol: " << protByte;
          return folly::none;
      }
    } catch (const std::exception& ex) {
      LOG(ERROR) << "Invalid envelope: " << ex.what();
      return folly::none;
    }
    // Remove the envelope.
    removePrefix(payload, folly::to_narrow(sz));
    return std::make_pair(std::move(envelope), std::move(payload));
  }

  static void removePrefix(std::unique_ptr<folly::IOBuf>& buf, uint32_t size) {
    while (buf->length() < size) {
      uint32_t len = folly::to_narrow(buf->length());
      size -= len;
      buf = buf->pop();
    }
    buf->trimStart(size);
  }
};

} // namespace apache::thrift
