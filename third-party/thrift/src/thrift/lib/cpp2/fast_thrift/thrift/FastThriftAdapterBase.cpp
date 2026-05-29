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
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/ErrorDecoding.h>

#include <fmt/core.h>

namespace apache::thrift::fast_thrift::thrift {

folly::Expected<ThriftResponseMessage, folly::exception_wrapper>
FastThriftAdapterBase::handleRequestResponse(ThriftResponseMessage&& response) {
  if (FOLLY_LIKELY(
          response.frame.type() ==
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD)) {
    apache::thrift::ResponseRpcMetadata metadata;
    if (auto error = deserializeResponseMetadata(response.frame, metadata);
        FOLLY_UNLIKELY(!!error)) {
      return folly::makeUnexpected(std::move(error));
    }

    if (auto error = processPayloadMetadata(metadata);
        FOLLY_UNLIKELY(!!error)) {
      return folly::makeUnexpected(std::move(error));
    }

    return std::move(response);
  } else if (
      response.frame.type() ==
      apache::thrift::fast_thrift::frame::FrameType::ERROR) {
    return folly::makeUnexpected(decodeErrorFrame(response.frame));
  } else {
    return folly::makeUnexpected(
        folly::make_exception_wrapper<apache::thrift::TApplicationException>(
            apache::thrift::TApplicationException::PROTOCOL_ERROR,
            fmt::format(
                "Unexpected frame type: {}",
                static_cast<int>(response.frame.type()))));
  }
}

} // namespace apache::thrift::fast_thrift::thrift
