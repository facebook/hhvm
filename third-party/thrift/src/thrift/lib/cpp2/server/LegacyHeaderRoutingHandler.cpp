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

#include <thrift/lib/cpp/protocol/TBinaryProtocol.h>
#include <thrift/lib/cpp/protocol/TCompactProtocol.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/server/LegacyHeaderRoutingHandler.h>

namespace apache {
namespace thrift {

LegacyHeaderRoutingHandler::LegacyHeaderRoutingHandler(ThriftServer&) {}

LegacyHeaderRoutingHandler::~LegacyHeaderRoutingHandler() {
  stopListening();
}

void LegacyHeaderRoutingHandler::stopListening() {
  listening_ = false;
}

// copied from THeader::compactFramed
bool compactFramed(uint32_t magic) {
  int8_t protocolId = (magic >> 24);
  int8_t protocolVersion =
      (magic >> 16) & (uint32_t)protocol::TCompactProtocol::VERSION_MASK;
  return (
      (protocolId == protocol::TCompactProtocol::PROTOCOL_ID) &&
      (protocolVersion <= protocol::TCompactProtocol::VERSION_N) &&
      (protocolVersion >= protocol::TCompactProtocol::VERSION_LOW));
}

bool LegacyHeaderRoutingHandler::canAcceptConnection(
    const std::vector<uint8_t>& bytes, const wangle::TransportInfo&) {
  if (!listening_) {
    return false;
  }

  uint32_t firstWord =
      bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
  // logic from THeader::analyzeFirst32bit
  if ((firstWord & protocol::TBinaryProtocol::VERSION_MASK) ==
      static_cast<uint32_t>(protocol::TBinaryProtocol::VERSION_1)) {
    // unframed
    return true;
  } else if (compactFramed(firstWord)) {
    // unframed compact
    return true;
  } else if (
      firstWord == transport::THeader::HTTP_SERVER_MAGIC ||
      firstWord == transport::THeader::HTTP_GET_CLIENT_MAGIC ||
      firstWord == transport::THeader::HTTP_HEAD_CLIENT_MAGIC ||
      firstWord == transport::THeader::HTTP_CLIENT_MAGIC) {
    // HTTP
    return true;
  }

  uint32_t secondWord =
      bytes[4] << 24 | bytes[5] << 16 | bytes[6] << 8 | bytes[7];
  // logic from THeader::analyzeSecond32bit
  if ((secondWord & protocol::TBinaryProtocol::VERSION_MASK) ==
      static_cast<uint32_t>(protocol::TBinaryProtocol::VERSION_1)) {
    // framed
    return true;
  } else if (compactFramed(secondWord)) {
    // framed compact
    return true;
  } else if (
      (secondWord & transport::THeader::HEADER_MASK) ==
      transport::THeader::HEADER_MAGIC) {
    // header
    return true;
  }
  return false;
}

bool LegacyHeaderRoutingHandler::canAcceptEncryptedConnection(
    const std::string& protocolName) {
  // for HTTP requests, the body is expected to be Thrift binary content
  return listening_ && (protocolName == "thrift" || protocolName == "http/1.1");
}

void LegacyHeaderRoutingHandler::handleConnection(
    wangle::ConnectionManager* /* connectionManager */,
    folly::AsyncTransport::UniquePtr sock,
    const folly::SocketAddress* address,
    const wangle::TransportInfo& tinfo,
    std::shared_ptr<Cpp2Worker> worker) {
  if (!listening_) {
    return;
  }
  // send it back to the Cpp2Worker for handling
  worker->handleHeader(std::move(sock), address, tinfo);
}

} // namespace thrift
} // namespace apache
