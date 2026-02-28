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

#include <folly/Range.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>

#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/core/EnvelopeUtil.h>

namespace apache::thrift {

void ClientReceiveState::initFromLegacyFormat(
    std::unique_ptr<folly::IOBuf> buffer) {
  if (!buffer) {
    return;
  }

  auto envelopeAndBody = EnvelopeUtil::stripEnvelope(std::move(buffer));

  if (!envelopeAndBody) {
    auto protocol = protocolId_ == static_cast<uint16_t>(-1)
        ? protocol::T_COMPACT_PROTOCOL
        : protocolId_;
    LOG(ERROR) << "Couldn't parse envelope, would serialize the exception with "
               << protocol;
    protocolId_ = protocol;
    messageType_ = MessageType::T_EXCEPTION;
    response_ = SerializedResponse(serializeErrorStruct(
        static_cast<protocol::PROTOCOL_TYPES>(protocol),
        TApplicationException(
            TApplicationException::PROTOCOL_ERROR,
            "Invalid message envelope!")));
    return;
  }

  protocolId_ = envelopeAndBody->first.protocolId;
  messageType_ = envelopeAndBody->first.messageType;
  response_ = SerializedResponse(std::move(envelopeAndBody->second));
}

} // namespace apache::thrift
