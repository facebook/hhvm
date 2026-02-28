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

#include <memory>

#include <folly/Try.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/python/streaming/StreamElementEncoder.h>

namespace apache::thrift::python::detail {

class PythonSinkElementDecoder final
    : public apache::thrift::detail::SinkElementDecoder<
          std::unique_ptr<::folly::IOBuf>> {
 public:
  folly::Try<std::unique_ptr<::folly::IOBuf>> operator()(
      folly::Try<apache::thrift::StreamPayload>&& payload) override {
    return decode_stream_element(std::move(payload));
  }
};

} // namespace apache::thrift::python::detail
