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

#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/ErrorDecoding.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Process a request-response frame, extracting data or decoding errors.
 *
 * - PAYLOAD frame: extracts the data buffer (zero-copy)
 * - ERROR frame: decodes into an exception via decodeErrorFrame()
 * - Other: returns a PROTOCOL_ERROR TApplicationException
 */
inline folly::Expected<std::unique_ptr<folly::IOBuf>, folly::exception_wrapper>
processRequestResponseFrame(
    apache::thrift::fast_thrift::frame::read::ParsedFrame& frame) {
  if (FOLLY_LIKELY(
          frame.type() ==
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD)) {
    return std::move(frame).extractData();
  } else if (
      frame.type() == apache::thrift::fast_thrift::frame::FrameType::ERROR) {
    return folly::makeUnexpected(decodeErrorFrame(frame));
  } else {
    return folly::makeUnexpected(
        folly::make_exception_wrapper<apache::thrift::TApplicationException>(
            apache::thrift::TApplicationException::PROTOCOL_ERROR,
            fmt::format(
                "Unexpected frame type: {}", static_cast<int>(frame.type()))));
  }
}

} // namespace apache::thrift::fast_thrift::thrift
