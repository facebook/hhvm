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

#include <thrift/lib/cpp2/fast_thrift/thrift/FastThriftAdapterBase.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/ErrorDecoding.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/RocketFrameDecoder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>

namespace apache::thrift::fast_thrift::thrift {

folly::Expected<std::unique_ptr<folly::IOBuf>, folly::exception_wrapper>
FastThriftAdapterBase::handleRequestResponse(
    ThriftResponseMessage&& response, uint16_t protocolId) {
  auto& inbound = response.payload.get<ThriftClientInboundPayloadVariant>();

  if (FOLLY_LIKELY(inbound.is<ThriftFirstResponsePayload>())) {
    auto& payload = inbound.get<ThriftFirstResponsePayload>();
    DCHECK(payload.metadata != nullptr);
    auto& metadata = *payload.metadata;

    auto action = classifyPayloadAction(metadata);
    if (FOLLY_UNLIKELY(action.hasError())) {
      return folly::makeUnexpected(std::move(action.error()));
    }

    if (FOLLY_UNLIKELY(action.value() == PayloadAction::ExtractAnyException)) {
      return folly::makeUnexpected(
          extractAnyException(std::move(payload.data), protocolId));
    }
    return std::move(payload.data);
  }

  if (inbound.is<ThriftErrorPayload>()) {
    auto& payload = inbound.get<ThriftErrorPayload>();
    auto decoded = decodeErrorAsResponse(payload.errorCode, payload.data.get());
    if (decoded.hasValue()) {
      return folly::makeUnexpected(
          folly::make_exception_wrapper<apache::thrift::TApplicationException>(
              decoded->exType, std::move(decoded->what)));
    }
    return folly::makeUnexpected(std::move(decoded.error()));
  }

  return folly::makeUnexpected(
      folly::make_exception_wrapper<apache::thrift::TApplicationException>(
          apache::thrift::TApplicationException::PROTOCOL_ERROR,
          "Unexpected payload alternative on RR response"));
}

} // namespace apache::thrift::fast_thrift::thrift
