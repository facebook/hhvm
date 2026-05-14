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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/RocketFrameDecoder.h>

#include <fmt/core.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

folly::exception_wrapper deserializeRequestMetadata(
    const frame::read::ParsedFrame& frame,
    apache::thrift::RequestRpcMetadata& metadata,
    rocket::server::MetadataProtocol metadataProtocol) noexcept {
  if (!frame.hasMetadata() || frame.metadataSize() == 0) {
    return {};
  }
  try {
    if (metadataProtocol == rocket::server::MetadataProtocol::COMPACT) {
      apache::thrift::CompactProtocolReader reader;
      reader.setInput(frame.metadataCursor());
      metadata.read(&reader);
    } else {
      apache::thrift::BinaryProtocolReader reader;
      reader.setInput(frame.metadataCursor());
      metadata.read(&reader);
    }
  } catch (...) {
    return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
        ("Failed to deserialize request metadata: " +
         folly::exceptionStr(std::current_exception()))
            .toStdString());
  }
  return {};
}

} // namespace

folly::Expected<ThriftServerInboundPayloadVariant, folly::exception_wrapper>
fromRocketFrame(
    frame::read::ParsedFrame&& frame,
    rocket::server::MetadataProtocol metadataProtocol) {
  using frame::FrameType;

  switch (frame.type()) {
    case FrameType::REQUEST_RESPONSE: {
      auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
      if (auto ew =
              deserializeRequestMetadata(frame, *metadata, metadataProtocol)) {
        return folly::makeUnexpected(std::move(ew));
      }
      auto data = std::move(frame).extractData();
      return ThriftServerInboundPayloadVariant{ThriftRequestResponsePayload{
          .data = std::move(data), .metadata = std::move(metadata)}};
    }

    case FrameType::REQUEST_FNF:
    case FrameType::REQUEST_STREAM:
    case FrameType::REQUEST_CHANNEL:
      return folly::makeUnexpected(
          folly::make_exception_wrapper<apache::thrift::TApplicationException>(
              fmt::format(
                  "fromRocketFrame: frame type {} not yet wired",
                  static_cast<int>(frame.type()))));

    default:
      return folly::makeUnexpected(
          folly::make_exception_wrapper<apache::thrift::TApplicationException>(
              fmt::format(
                  "fromRocketFrame: unexpected inbound frame type {}",
                  static_cast<int>(frame.type()))));
  }
}

} // namespace apache::thrift::fast_thrift::thrift
