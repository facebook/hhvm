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

#include <folly/Expected.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/RequestMetadata.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

inline constexpr size_t kDataHeadroomBytes = 128;

namespace detail {

template <typename ProtocolWriter>
constexpr apache::thrift::ProtocolId protocolIdFor();

template <>
constexpr apache::thrift::ProtocolId
protocolIdFor<apache::thrift::CompactProtocolWriter>() {
  return apache::thrift::ProtocolId::COMPACT;
}

template <>
constexpr apache::thrift::ProtocolId
protocolIdFor<apache::thrift::BinaryProtocolWriter>() {
  return apache::thrift::ProtocolId::BINARY;
}

template <typename ProtocolWriter, typename SerializeFn, typename SizeFn>
folly::Expected<ThriftRequestMessage, folly::exception_wrapper>
serializeRequestMessage(
    std::unique_ptr<folly::IOBuf> metadata,
    apache::thrift::RpcKind rpcKind,
    SerializeFn&& serializeFn,
    SizeFn&& sizeFn) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());

  try {
    ProtocolWriter writer;
    uint32_t serSize = sizeFn(writer);

    auto dataBuf = folly::IOBuf::create(kDataHeadroomBytes + serSize);
    dataBuf->advance(kDataHeadroomBytes);
    queue.append(std::move(dataBuf));
    writer.setOutput(&queue);
    serializeFn(writer);

  } catch (...) {
    return folly::makeUnexpected(
        folly::make_exception_wrapper<apache::thrift::TApplicationException>(
            "Failed to serialize request data"));
  }

  return ThriftRequestMessage{
      .payload = ThriftRequestPayload{
          .metadata = std::move(metadata),
          .data = queue.move(),
          .rpcKind = rpcKind,
          .complete = true}};
}

} // namespace detail

/**
 * Serialize a Thrift request.
 *
 * Caller provides fully-formed serialize/size callbacks that handle the
 * entire struct payload (including writeStructBegin/End if needed).
 *
 * @param methodName Must point to static storage (string literal).
 * @param serializeFn (ProtocolWriter&) -> void — serializes the payload
 * @param sizeFn  (ProtocolWriter&) -> uint32_t — returns serialized size
 */
template <typename ProtocolWriter, typename SerializeFn, typename SizeFn>
folly::Expected<ThriftRequestMessage, folly::exception_wrapper>
serializeRequest(
    const apache::thrift::RpcOptions& rpcOptions,
    std::string_view methodName,
    apache::thrift::RpcKind rpcKind,
    SerializeFn&& serializeFn,
    SizeFn&& sizeFn) {
  std::unique_ptr<folly::IOBuf> metadata;
  try {
    metadata = makeSerializedRequestMetadata(
        rpcOptions,
        apache::thrift::ManagedStringView::from_static(methodName),
        rpcKind,
        detail::protocolIdFor<ProtocolWriter>());
  } catch (...) {
    return folly::makeUnexpected(
        folly::make_exception_wrapper<apache::thrift::TApplicationException>(
            "Failed to serialize request metadata"));
  }

  return detail::serializeRequestMessage<ProtocolWriter>(
      std::move(metadata),
      rpcKind,
      std::forward<SerializeFn>(serializeFn),
      std::forward<SizeFn>(sizeFn));
}

} // namespace apache::thrift::fast_thrift::thrift
